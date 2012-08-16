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

#include <Inventor/events/SoLocation2Event.h>
#include <Base/Console.h>

#include <App/Application.h>
#include <Base/Exception.h>

#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Control.h>
#include <Gui/Document.h>
#include <Gui/MainWindow.h>
#include <Gui/MDIView.h>
#include <Gui/Selection.h>
#include <Gui/View3DInventor.h>
#include <Gui/View3DInventorViewer.h>

#include <QDeclarativeContext>
#include <QDeclarativeView>
#include <QDeclarativeItem>
#include <QGraphicsObject>
#include <QGLWidget>
#include <QDragMoveEvent>

#include <Mod/Part/App/PartFeature.h>
#include <Mod/Raytracing/App/Appearances.h>
#include <Mod/Raytracing/App/RenderMaterial.h>
#include <Mod/Raytracing/App/RenderFeature.h>

#include "MaterialParametersModel.h"
#include "TaskDlgAppearances.h"
#include "TaskDlgRender.h"
#include "ViewProviderRender.h"

using namespace Raytracing;
using namespace RaytracingGui;


//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskDlgAppearances::TaskDlgAppearances()
    : TaskDialog()
{
    // A Render Feature MUST be active to open the appearances dialog
    RenderFeature *feat;
    App::DocumentObject *activeObj =  App::GetApplication().getActiveDocument()->getActiveObject();
    if(activeObj && activeObj->getTypeId() == RenderFeature::getClassTypeId())
      feat = static_cast<RenderFeature *>(activeObj);


    if(!feat || !feat->hasRenderer())
        throw Base::Exception("The currently active object must be a render feature and have a render backend");

    model = new AppearancesModel();

    Base::Console().Log(feat->getRenderer()->getProviderName());
    // Load the materials for the render feature's renderer
    std::vector<LibraryMaterial *> materials = Appearances().getMaterialsByProvider(feat->getRenderer()->getProviderName());

    for (std::vector<LibraryMaterial *>::const_iterator it= materials.begin(); it!= materials.end(); ++it) {
            model->addLibraryMaterial(*it);
    }

    view = new QDeclarativeView(qobject_cast<QWidget *>(this));
    QDeclarativeContext *ctxt = view->rootContext();
    ctxt->setContextProperty(QString::fromAscii("appearancesModel"), model);

    view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    view->setSource(QUrl(QString::fromAscii("qrc:/qml/appearancesUi.qml"))); // Load the Main QML File

     // Connect an Update Signal when an image is available
    QObject *rootObject = view->rootObject();
    QObject::connect(rootObject, SIGNAL(materialDrag(QString)), this , SLOT(dragInit(QString)));
    QObject::connect(rootObject, SIGNAL( materialPropsAccepted()), this , SLOT(materialParamSave())); // Initiated when a Material Properties is Saved

    this->Content.push_back(view);
}

TaskDlgAppearances::~TaskDlgAppearances()
{
    delete model;
    model = 0;
}

void TaskDlgAppearances::dragInit(QString str)
{
    const LibraryMaterial *dragMaterial = Appearances().getMaterialById(str.toAscii());

    QDrag *drag = new QDrag(this->Content.back());
    QMimeData *mimeData = new QMimeData;

    mimeData->setText(str);
    drag->setMimeData(mimeData);
    QPixmap pixmap(dragMaterial->previewFilename);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(drag->pixmap().width()/2,
                            drag->pixmap().height()));

    Gui::MDIView *mdi = Gui::Application::Instance->activeDocument()->getActiveView();

    // Catch all the drag events occuring
    mdi->installEventFilter(this);
    Qt::DropAction dropAction = drag->exec();
}

bool TaskDlgAppearances::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::DragMove) {
        materialDragEvent(static_cast<QDragMoveEvent *> (event));
    } else if(event->type() == QEvent::Drop) {
        materialDropEvent(static_cast<QDropEvent *> (event));
    }
}

void TaskDlgAppearances::materialDropEvent(QDropEvent *ev)
{
    // First remove the event filter
    Gui::MDIView *mdi = Gui::Application::Instance->activeDocument()->getActiveView();
    mdi->installEventFilter(this);

    // Grab the associated material
    QString materialId = ev->mimeData()->text();
    const LibraryMaterial *libMat = Appearances().getMaterialById(materialId.toAscii());

    if(!libMat) {
        Base::Console().Error("Material with ID could not be found");
        return;
    }

    // Detect the Preselection
    const Gui::SelectionChanges selection = Gui::Selection().getPreselection();

    // Selection must be derived from a part feature

    RenderMaterial myMaterial(libMat);
    QString objectName = QString::fromAscii(selection.pObjectName);

    App::DocumentObject *docObj = App::GetApplication().getActiveDocument()->getObject(selection.pObjectName);

    if(!docObj->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId())) {
        Base::Console().Error("A correct part must be chosen derived from Part::Feature");
        return;
    }

    std::vector<std::string> sub;
    //sub.push_back(selection.pSubName); //TODO

    myMaterial.Link.setValue(docObj, sub);

    // Find the RenderFeature object. It must be currently active to add the RenderMaterial
    App::DocumentObject *activeObj =  App::GetApplication().getActiveDocument()->getActiveObject();
    if(activeObj && activeObj->getTypeId() == RenderFeature::getClassTypeId()) {
        RenderFeature *feat = static_cast<RenderFeature *>(activeObj);

        // Remove the previous RenderMaterial if one exists
        const RenderMaterial *mat = feat->getRenderMaterial(selection.pObjectName);

        if(mat)
        {
            Gui::Command::openCommand("Removing Material");
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.ActiveObject.removeRenderMaterialFromPart('%s')", selection.pObjectName);
            Gui::Command::commitCommand();
        }
        feat->addRenderMaterial(&myMaterial);

        // TODO check if we should reload the QML or create a new widget
        // TODO should params model be put on the heap - likely
        MaterialParametersModel paramsModel;

        QMap<QString, MaterialParameter*> params =  myMaterial.getMaterial()->parameters;
        QMap<QString, MaterialParameter*>::const_iterator i;
        for (i = params.constBegin(); i != params.constEnd(); ++i) {
            MaterialParameter *param = i.value();
            paramsModel.addParameter(param); //Add the parameter to model
        }

//         QDeclarativeView *paramView = new QDeclarativeView (qobject_cast<QWidget *>(this));
//         QDeclarativeContext *ctxt = paramView->rootContext();
//         ctxt->setContextProperty(QString::fromAscii("appearancesModel"), &paramsModel);
// 
//         paramView->setResizeMode(QDeclarativeView::SizeRootObjectToView);

        RenderMaterial *matClone = new RenderMaterial(myMaterial); // Make a copy of the material on the heap
        materialData = new RenderMaterialData(matClone); // Assuming this gets deleted with destruction of QDeclartiveContext
        // TODO delete above

        QDeclarativeContext *ctxt = view->rootContext();
        ctxt->setContextProperty(QString::fromAscii("materialData"), materialData);
        ctxt->setContextProperty(QString::fromAscii("materialParametersModel"), &paramsModel);

        QObject *rootObject = qobject_cast<QObject *>(view->rootObject());
        QMetaObject::invokeMethod(rootObject, "openMaterialParametersWidget");

       //paramView->setSource(QUrl(QString::fromAscii("qrc:/qml/materialParametersUi.qml"))); // Load the Main QML File

        // Connect an Update Signal when an image is available
//         QString str = QString::fromAscii("materialParametersWidget");
//         QObject *rootObject = paramView->rootObject();

        //QObject::connect(rootObject, SIGNAL(cancel()) , paramView , SLOT(close()));

    }
}

void TaskDlgAppearances::materialParamSave()
{
        // Find the RenderFeature object. It must be currently active to add the RenderMaterial
    App::DocumentObject *activeObj =  App::GetApplication().getActiveDocument()->getActiveObject();
    if(activeObj->getTypeId() == RenderFeature::getClassTypeId()) {
        RenderFeature *feat = static_cast<RenderFeature *>(activeObj);
        if(!materialData)
            return; //No Material Data Set

        // TODO implement App::Command for undo states
        feat->setRenderMaterial(materialData->getRenderMaterial()); //setRenderMaterial will clone the material
        QObject *rootObject = qobject_cast<QObject *>(view->rootObject());
        QMetaObject::invokeMethod(rootObject, "openMaterialLibraryWidget");
        // delete temporary material
    }
}

void TaskDlgAppearances::materialParamCancel()
{

}
void TaskDlgAppearances::materialDragEvent(QDragMoveEvent *ev)
{

    if(!ev)
      return;

    Gui::MDIView *mdi = Gui::Application::Instance->activeDocument()->getActiveView();

    int height = static_cast<Gui::View3DInventor *>(mdi)->height();

    // Simulate a mouse position event
    SoLocation2Event *locEv = new SoLocation2Event();
    SbVec2s pos(ev->pos().x(), height - ev->pos().y()); // We must reverse the y coordinates
    locEv->setPosition(pos);

    //Send the Mouse Position to the viewer
    char buf[255];
    static_cast<Gui::View3DInventor *>(mdi)->getViewer()->sendSoEvent(locEv);
}


//==== calls from the TaskView ===============================================================


void TaskDlgAppearances::open()
{

}

void TaskDlgAppearances::clicked(int)
{

}

bool TaskDlgAppearances::accept()
{
        // We load the TaskDlgRender, since this is the active
    Gui::Document * doc = Gui::Application::Instance->activeDocument();
    if (doc) {
        if (doc->getInEdit() && doc->getInEdit()->isDerivedFrom(ViewProviderRender::getClassTypeId())) {
            ViewProviderRender *vp = dynamic_cast<ViewProviderRender*>(doc->getInEdit());

            Gui::Control().closeDialog();
            Gui::Selection().clearSelection();
            Gui::Control().showDialog(new TaskDlgRender(vp));
            return false;
        }
    }
    return true;
}

bool TaskDlgAppearances::reject()
{

    // We load the TaskDlgRender, since this is the active 
    Gui::Document * doc = Gui::Application::Instance->activeDocument();
    if (doc) {
        if (doc->getInEdit() && doc->getInEdit()->isDerivedFrom(ViewProviderRender::getClassTypeId())) {
            ViewProviderRender *vp = dynamic_cast<ViewProviderRender*>(doc->getInEdit());

            Gui::Control().closeDialog();
            Gui::Selection().clearSelection();
            Gui::Control().showDialog(new TaskDlgRender(vp));
            return false;
        }
    }
}

void TaskDlgAppearances::helpRequested()
{

}


#include "moc_TaskDlgAppearances.cpp"
