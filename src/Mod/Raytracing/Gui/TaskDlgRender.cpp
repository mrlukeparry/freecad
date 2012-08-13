/***************************************************************************
 *   Copyright (c) Luke Parry          (l.parry@warwick.ac.uk)    2012     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include "PreCompiled.h"
#ifndef _PreComp_
#endif

#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/nodes/SoCamera.h>

#include <Base/Console.h>

#include <App/Application.h>

#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/MainWindow.h>
#include <Gui/MDIView.h>
#include <Gui/Selection.h>
#include <Gui/SoFCBoundingBox.h>
#include <Gui/View3DInventor.h>
#include <Gui/View3DInventorViewer.h>

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QGLWidget>
#include <QDragMoveEvent>

#include <Mod/Part/App/PartFeature.h>

#include "TaskDlgRender.h"
#include "RenderView.h"

using namespace Raytracing;
using namespace RaytracingGui;

PresetsModel::PresetsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[LabelRole] = "label";
    roles[DescriptionRole] = "description";
    setRoleNames(roles);
}

void PresetsModel::addRenderPreset(RenderPreset *preset)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_libPresets << preset;
    endInsertRows();
}

int PresetsModel::rowCount(const QModelIndex & parent) const {
    return m_libPresets.count();
}

QVariant PresetsModel::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() > m_libPresets.count())
        return QVariant();

    const RenderPreset *preset = m_libPresets[index.row()];

    if (role == IdRole)
        return preset->getId();
    else if (role == LabelRole)
        return preset->getLabel();
    else if (role == DescriptionRole)
        return preset->getDescription();
    return QVariant();
}

// ====== Templates Model for QML ============ //
TemplatesModel::TemplatesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[LabelRole] = "label";
    roles[DescriptionRole] = "description";
    setRoleNames(roles);
}

void TemplatesModel::addRenderTemplate(RenderTemplate *renderTemplate)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_rendTemplates << renderTemplate;
    endInsertRows();
}

int TemplatesModel::rowCount(const QModelIndex & parent) const {
    return m_rendTemplates.count();
}

QVariant TemplatesModel::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() > m_rendTemplates.count())
        return QVariant();

    const RenderTemplate *templ = m_rendTemplates[index.row()];

    if (role == IdRole)
        return templ->getId();
    else if (role == LabelRole)
        return templ->getLabel();
    else if (role == DescriptionRole)
        return templ->getDescription();
    return QVariant();
}

//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskDlgRender::TaskDlgRender(ViewProviderRender *vp)
    : TaskDialog(),
      renderView(vp)
{
    assert(renderView);
    documentName = renderView->getObject()->getDocument()->getName();

    RenderMaterial *mat;
    LibraryMaterial *libMat;
    libMat->parameters;
    view = new QDeclarativeView (qobject_cast<QWidget *>(this));

    RenderFeature *feat = this->getRenderView()->getRenderFeature();
    RenderFeatureData *data = new RenderFeatureData(feat); // Assuming this gets deleted with destruction of QDeclartiveContext

    // Create the Presets Model using all the Render Presets found in Renderer
    PresetsModel presetsModel;

    std::vector<RenderPreset *> presets = feat->getRenderer()->getRenderPresets();
    for (std::vector<RenderPreset *>::const_iterator it= presets.begin(); it!= presets.end(); ++it) {
            presetsModel.addRenderPreset(*it);
    }

    TemplatesModel templatesModel;

    std::vector<RenderTemplate *> renderTemplates = feat->getRenderer()->getRenderTemplates();
    for (std::vector<RenderTemplate *>::const_iterator it= renderTemplates.begin(); it!= renderTemplates.end(); ++it) {
            templatesModel.addRenderTemplate(*it);
    }

    QDeclarativeView *view = new QDeclarativeView ();
    QDeclarativeContext *ctxt = view->rootContext();

    ctxt->setContextProperty(QString::fromAscii("presetsModel"), &presetsModel); // Render Presets
    ctxt->setContextProperty(QString::fromAscii("templatesModel"), &templatesModel); // Render Templates
    ctxt->setContextProperty(QString::fromAscii("renderFeature"), data);

    view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    view->setSource(QUrl(QString::fromAscii("qrc:/qml/renderUi.qml"))); // Load the Main QML File

     // Connect an Update Signal when an image is available
    QObject *rootObject = qobject_cast<QObject *>(view->rootObject());

    // Connect the slots
    QObject::connect(rootObject, SIGNAL(preview()), this , SLOT(preview()));
    QObject::connect(rootObject, SIGNAL(previewWindow()), this , SLOT(previewWindow()));

    QObject::connect(this, SIGNAL(renderStop()), rootObject , SLOT(renderStopped()));
    QObject::connect(this, SIGNAL(renderStart()), rootObject , SLOT(renderRunning()));

    // Check if a Render is currently active for this RenderFeature
    if(isRenderActive()) {
        QMetaObject::invokeMethod(rootObject, "renderRunning");

        // Reconnect slot when finished to enable buttons
        RenderFeature *feat = this->getRenderView()->getRenderFeature();
        RenderProcess *process = feat->getActiveRenderProcess();
        QObject *renderProcQObj = qobject_cast<QObject *>(process);
        QObject::connect(renderProcQObj , SIGNAL(finished()), rootObject , SLOT(renderStopped())); // Connect Render Process Signal when stopped by user
    }

    this->Content.push_back(view);
}

TaskDlgRender::~TaskDlgRender()
{

    // Disable Drag and Drops on the 3D Inventor Window
//     Gui::MDIView *mdi = Gui::Application::Instance->activeDocument()->getActiveView();
//     if(mdi) {
//         mdi->setAcceptDrops(false);
//     }
}

bool TaskDlgRender::isRenderActive()
{
    RenderFeature *feat = this->getRenderView()->getRenderFeature();
    RenderProcess *process = feat->getActiveRenderProcess();
    if(!process)
        return false;
    if(process->isActive())
        return true;
}

//==== SLOTS ====/
void TaskDlgRender::preview()
{

    RenderFeature *feat = this->getRenderView()->getRenderFeature();
    feat->reset(); 
    Gui::Document * doc = Gui::Application::Instance->activeDocument();

        // get all objects of the active document
    std::vector<Part::Feature*> DocObjects = App::GetApplication().getActiveDocument()->getObjectsOfType<Part::Feature>();

    for (std::vector<Part::Feature*>::const_iterator it=DocObjects.begin();it!=DocObjects.end();++it) {
        Gui::ViewProvider* vp = doc->getViewProvider(*it);
        if (vp && vp->isVisible()) {
          float meshDev = 0.1;
//             App::PropertyColor *pcColor = dynamic_cast<App::PropertyColor *>(vp->getPropertyByName("ShapeColor"));
//             App::Color col = pcColor->getValue();

            RenderPart *part = new RenderPart((*it)->getNameInDocument(), (*it)->Shape.getValue(), meshDev);
            feat->getRenderer()->addObject(part); //TODO we need to provide an interface in the RenderFeature for adding / storing objects
        }
    }

    RenderView *renderView    = new RenderView(doc, Gui::getMainWindow());

    feat->preview();

    // Get Active Render Process
    RenderProcess *process = feat->getActiveRenderProcess();
    if(!process) {
      Base::Console().Error("Render Process is not available \n");
      return;
    }

    QObject *renderProcQObj = qobject_cast<QObject *>(process);
    QObject::connect(renderProcQObj , SIGNAL(finished()), this , SLOT(renderStopped())); // Connect Render Process Signal when stopped by user
    QObject::connect(renderProcQObj , SIGNAL(started()) , this , SLOT(renderStarted())); // Connect Render Process (QProcess) start signal when active

    // Invoke renderActive slot if active, because we have most likely missed RenderProcess Start Method
    if(process->isActive())
        Q_EMIT renderStart();

    renderView->attachRender(feat->getRenderer());
    renderView->setWindowTitle(QObject::tr("Render viewer") + QString::fromAscii("[*]"));
    Gui::getMainWindow()->addWindow(renderView);

}

void TaskDlgRender::getRenderBBox(SbBox3f &box)
{
    // ensure that we are in sketch only selection mode
    Gui::MDIView *mdi = Gui::Application::Instance->activeDocument()->getActiveView();
    Gui::View3DInventorViewer *viewer;
    viewer = static_cast<Gui::View3DInventor *>(mdi)->getViewer();

    SoNode* root = viewer->getSceneGraph();

    SoSearchAction sa;
    sa.setType(Gui::SoSkipBoundingGroup::getClassTypeId());
    sa.setInterest(SoSearchAction::ALL);
    sa.apply(viewer->getSceneGraph());

    const SoPathList & pathlist = sa.getPaths();
    for (int i = 0; i < pathlist.getLength(); i++ ) {
        SoPath * path = pathlist[i];
        Gui::SoSkipBoundingGroup * group = static_cast<Gui::SoSkipBoundingGroup*>(path->getTail());
        group->mode = Gui::SoSkipBoundingGroup::EXCLUDE_BBOX;
    }

    SoGetBoundingBoxAction action(viewer->getViewportRegion());
    action.apply(viewer->getSceneGraph());
}

//---------- SLOTS ---------//
void TaskDlgRender::renderStopped()
{
    // Basically need to emit signals so that these are guaranteed to be picked up by the QML UI
    Q_EMIT renderStop();
}

void TaskDlgRender::renderStarted()
{
    Q_EMIT renderStart();
}


void TaskDlgRender::previewWindow()
{

   // Check if a renderer camera exists
    RenderFeature *feat = this->getRenderView()->getRenderFeature();
    feat->reset(); 
    RenderCamera *renderCam = feat->getCamera();
    if(!renderCam)
        return;

    SoCamera *Cam;

    Gui::MDIView *mdi = Gui::Application::Instance->activeDocument()->getActiveView();
    if (mdi && mdi->isDerivedFrom(Gui::View3DInventor::getClassTypeId())) {
        Gui::View3DInventorViewer *viewer = static_cast<Gui::View3DInventor *>(mdi)->getViewer();
        Cam =  viewer->getCamera();
    } else {
        return; //throw Base::Exception("Could Not Read Camera");
    }

    Gui::Document * doc = Gui::Application::Instance->activeDocument();

        // get all objects of the active document
    std::vector<Part::Feature*> DocObjects = App::GetApplication().getActiveDocument()->getObjectsOfType<Part::Feature>();

    for (std::vector<Part::Feature*>::const_iterator it=DocObjects.begin();it!=DocObjects.end();++it) {
        Gui::ViewProvider* vp = doc->getViewProvider(*it);
        if (vp && vp->isVisible()) {
          float meshDev = 0.1;
//             App::PropertyColor *pcColor = dynamic_cast<App::PropertyColor *>(vp->getPropertyByName("ShapeColor"));
//             App::Color col = pcColor->getValue();

            RenderPart *part = new RenderPart((*it)->getNameInDocument(), (*it)->Shape.getValue(), meshDev);
            feat->getRenderer()->addObject(part); //TODO we need to provide an interface in the RenderFeature for adding / storing objects
        }
    }


    // Get the current camera positions
    SbRotation camrot = Cam->orientation.getValue();
    SbViewVolume::ProjectionType CamType = Cam->getViewVolume().getProjectionType();

    SbVec3f upvec(0, 1, 0); // init to default up vector
    camrot.multVec(upvec, upvec);

    SbVec3f lookat(0, 0, -1); // init to default view direction vector
    camrot.multVec(lookat, lookat);

    SbVec3f pos = Cam->position.getValue();
    float Dist = Cam->focalDistance.getValue();

    RenderCamera::CamType camType;

    // Get viewport camera type
    char *camTypeStr;
    switch(CamType) {
      case SbViewVolume::ORTHOGRAPHIC:
        camTypeStr = "Orthographic";
        camType = RenderCamera::ORTHOGRAPHIC; break;
      case SbViewVolume::PERSPECTIVE:
        camType = RenderCamera::PERSPECTIVE;
        camTypeStr = "Perspective";
        break;
    }

        // Calculate Camera Properties
    Base::Vector3d camPos(pos[0], pos[1], pos[2]);
    Base::Vector3d camDir(lookat[0],lookat[1], lookat[2]);
    Base::Vector3d camUp(upvec[0],upvec[1], upvec[2]);
    Base::Vector3d camLookAt = camDir * Dist + camPos;


    // Not happy with this, most likely will figure a better solution out
    Base::Vector3d tmp1, tmp2, tmp3, tmp4;
    tmp1 = renderCam->CamPos;
    tmp2 = renderCam->CamDir;
    tmp3 = renderCam->Up;
    tmp4 = renderCam->LookAt;
    RenderCamera::CamType tmp5 = renderCam->Type;

    // Set the rendera camera to viewport
    feat->setCamera(camPos, camDir, camUp, camLookAt, camTypeStr);

     // Get the View Provider Height from MDI View
    int height = static_cast<Gui::View3DInventor *>(mdi)->height();
    int width  = static_cast<Gui::View3DInventor *>(mdi)->width();

    // Unsure if we should set this temporarily
    //Store current output size to restore later
    int tempWidth  = feat->OutputX.getValue();
    int tempHeight = feat->OutputY.getValue();

    feat->OutputX.setValue(width);
    feat->OutputY.setValue(height);

    // Run the preview
    feat->preview();

    // Restore preview sizes
    feat->OutputX.setValue(tempWidth);
    feat->OutputY.setValue(tempHeight);

    // Restore previous camera settings
    switch(tmp5) {
      case SbViewVolume::ORTHOGRAPHIC:
        camTypeStr = "Orthographic";
      case SbViewVolume::PERSPECTIVE:
        camTypeStr = "Perspective";
        break;
    }

    // Set the rendera camera back to previous
    feat->setCamera(tmp1,tmp2, tmp3, tmp4, camTypeStr);

    // Get Active Render Process
    RenderProcess *process = feat->getActiveRenderProcess();
    if(!process) {
      Base::Console().Error("Render Process is not available \n");
      return;
    }

    QList<QDeclarativeError> errors = this->view->errors();
    int count = errors.count();

    RenderView *renderView    = new RenderView(doc, Gui::getMainWindow());


    QObject *renderProcQObj = qobject_cast<QObject *>(process);
    // Unfortunatly we have to make this class acts as a middleman with events
    QObject::connect(renderProcQObj , SIGNAL(finished()), this , SLOT(renderStopped())); // Connect Render Process Signal when stopped by user
    QObject::connect(renderProcQObj , SIGNAL(started()) , this , SLOT(renderStarted())); // Connect Render Process (QProcess) start signal when active

    // Invoke renderActive slot if active, because we have most likely missed RenderProcess Start Method


    renderView->attachRender(feat->getRenderer());
    renderView->setWindowTitle(QObject::tr("Render viewer") + QString::fromAscii("[*]"));
    Gui::getMainWindow()->addWindow(renderView);

}

void TaskDlgRender::render()
{

}

//==== calls from the TaskView ===============================================================


void TaskDlgRender::open()
{

}

void TaskDlgRender::clicked(int)
{

}

bool TaskDlgRender::accept()
{
    return true;
}

bool TaskDlgRender::reject()
{
//     std::string document = documentName; // needed because resetEdit() deletes this instance
//     Gui::Command::doCommand(Gui::Command::Gui,"Gui.getDocument('%s').resetEdit()", document.c_str());
//     Gui::Command::doCommand(Gui::Command::Doc,"App.getDocument('%s').recompute()", document.c_str());

    return true;
}

void TaskDlgRender::helpRequested()
{

}


#include "moc_TaskDlgRender.cpp"
