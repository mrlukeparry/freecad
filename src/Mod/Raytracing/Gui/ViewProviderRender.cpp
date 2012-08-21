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
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
# endif

/// Here the FreeCAD includes sorted by Base,App,Gui......
#include <Base/Console.h>
#include <Base/Parameter.h>
#include <Base/Exception.h>
#include <Base/Sequencer.h>
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Control.h>
#include <Gui/Document.h>
#include <Gui/SoFCSelection.h>
#include <Gui/Selection.h>
#include <Gui/SoTextLabel.h>
#include <Gui/MainWindow.h>
#include <Gui/SoFCUnifiedSelection.h>
#include <Gui/SoFCBoundingBox.h>
#include <Gui/View3DInventor.h>

#include <QMenu>
#include <QDropEvent>
#include <QMimeData>
#include <Gui/BitmapFactory.h>
#include <Gui/ViewProviderDocumentObjectGroup.h>

#include <Mod/Part/App/PartFeature.h>
#include <Mod/Raytracing/App/RenderCamera.h>
#include "ViewProviderRender.h"
#include "TaskDlgRender.h"

using namespace Raytracing;
using namespace RaytracingGui;

PROPERTY_SOURCE(RaytracingGui::ViewProviderRender, Gui::ViewProviderDocumentObjectGroup)


//**************************************************************************
// Construction/Destruction

ViewProviderRender::ViewProviderRender() : edit(false)
{
    sPixmap = "Page";
    // ensure that we are in sketch only selection mode
}

ViewProviderRender::~ViewProviderRender()
{

}

void ViewProviderRender::attach(App::DocumentObject *pcFeat)
{
    // call parent attach method
    ViewProviderDocumentObject::attach(pcFeat);
}

Raytracing::RenderFeature * ViewProviderRender::getRenderFeature(void) const
{
    return dynamic_cast<Raytracing::RenderFeature *>(pcObject);
}

void ViewProviderRender::setDisplayMode(const char* ModeName)
{
    ViewProviderDocumentObject::setDisplayMode(ModeName);
}

std::vector<std::string> ViewProviderRender::getDisplayModes(void) const
{
    // get the modes of the father
    //TODO unsure about this
    std::vector<std::string> StrList;
    StrList.push_back("Flat Lines");
    return StrList;
}

void ViewProviderRender::updateData(const App::Property* prop)
{
    ViewProviderDocumentObjectGroup::updateData(prop);

    if (edit && (prop == &(getRenderFeature()->MaterialsList))) {
        draw();
        signalMaterialsChanged();
    }
}

void ViewProviderRender::createInventorNodes()
{
    editRoot = new SoSeparator();
    pcRoot->addChild(editRoot);
}

void ViewProviderRender::draw()
{
    RenderFeature *feat = getRenderFeature();

    const std::vector<RenderMaterial *> mats = feat->MaterialsList.getValues();

    editRoot->renderCaching = SoSeparator::OFF ;
    if(editRoot->getNumChildren() > 0)
        editRoot->removeAllChildren();

    SoGroup *labelGroup = new SoGroup();

    for(std::vector<RenderMaterial *>::const_iterator it = mats.begin(); it != mats.end(); ++it){
        App::DocumentObject *obj = feat->getRenderMaterialLink((*it));
        if(!obj || !obj->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId()))
            continue;

        Part::Feature *part = dynamic_cast<Part::Feature *>(obj);

        if(!part)
            continue;

        Base::Placement placement = part->Placement.getValue();
        Base::Vector3d pos = placement.getPosition();

        SoSeparator *sep = new SoSeparator();
        SoTranslation *tran = new SoTranslation();
        tran->translation.setValue(SbVec3f(pos.x, pos.y, pos.z));

        Gui::SoFrameLabel *label = new Gui::SoFrameLabel();
        label->string.setValue((*it)->getMaterial()->label.toAscii());

        sep->addChild(tran);
        sep->addChild(label);
        labelGroup->addChild(sep);

    }
    editRoot->addChild(labelGroup);
}

bool ViewProviderRender::mouseMove(const SbVec3f &pos, const SbVec3f &norm, const SoPickedPoint* pp) {

    return true;
}

bool ViewProviderRender::keyPressed(bool pressed, int key)
{
    switch (key)
    {
    case SoKeyboardEvent::ESCAPE:
        {

        }
    default:
        {
        }
    }

    return true; // handle all other key events
}


void ViewProviderRender::setupContextMenu(QMenu *menu, QObject *receiver, const char *member)
{
    menu->addAction(QObject::tr("Edit Render"), receiver, member);
}

bool ViewProviderRender::setEdit(int ModNum)
{
   // When double-clicking on the item for this sketch the
    // object unsets and sets its edit mode without closing
    // the task panel
    Gui::TaskView::TaskDialog *dlg = Gui::Control().activeDialog();
    TaskDlgRender *taskDlgRender = qobject_cast<TaskDlgRender *>(dlg);
//     if (sketchDlg && sketchDlg->getSketchView() != this)
//         sketchDlg = 0; // another sketch left open its task panel
    if (dlg && !taskDlgRender) {
        QMessageBox msgBox;
        msgBox.setText(QObject::tr("A dialog is already open in the task panel"));
        msgBox.setInformativeText(QObject::tr("Do you want to close this dialog?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes)
            Gui::Control().closeDialog();
        else
            return false;
    }
    Gui::Selection().clearSelection();


    //start the edit dialog
    if (taskDlgRender)
        Gui::Control().showDialog(taskDlgRender);
    else
        Gui::Control().showDialog(new TaskDlgRender(this));

    edit = true;
    createInventorNodes();
    draw();
    return true;
}

void ViewProviderRender::getRenderBBox(SbBox3f &box)
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
    box = action.getBoundingBox();
}

bool ViewProviderRender::doubleClicked(void)
{
    Gui::Application::Instance->activeDocument()->setEdit(this);
    return true;
}

void ViewProviderRender::setEditViewer(Gui::View3DInventorViewer* viewer, int ModNum)
{
    RenderCamera *cam     = getRenderFeature()->getCamera();

    Base::Vector3d lookAt = cam->LookAt;
    Base::Vector3d pos    = cam->CamPos;
    Base::Vector3d camUp  = cam->Up;

    SoCamera *myCam = viewer->getCamera();

    // Set to render feature's camera
    myCam->position.setValue(SbVec3f(pos.x, pos.y, pos.z));
    myCam->pointAt(SbVec3f(lookAt.x, lookAt.y, lookAt.z), SbVec3f(camUp.x, camUp.y, camUp.z));
    myCam->focalDistance.setValue((lookAt - pos).Length());

    viewer->setEditing(TRUE);
    SoNode* root = viewer->getSceneGraph();
    //static_cast<Gui::SoFCUnifiedSelection*>(root)->selectionRole.setValue(FALSE);
}

void ViewProviderRender::unsetEditViewer(Gui::View3DInventorViewer* viewer)
{
    viewer->setEditing(FALSE);
}

void ViewProviderRender::unsetEdit(int ModNum)
{
    edit = false;
    editRoot->removeAllChildren();
    pcRoot->removeChild(editRoot);

    // clear the selection and set the new/edited sketch(convenience)
    Gui::Selection().clearSelection();
    std::string ObjName = getRenderFeature()->getNameInDocument();
    std::string DocName = getRenderFeature()->getDocument()->getName();
    Gui::Selection().addSelection(DocName.c_str(),ObjName.c_str());

    // when pressing ESC make sure to close the dialog
    Gui::Control().closeDialog();
}

bool ViewProviderRender::onDelete(const std::vector<std::string> &subList)
{
    //FIXME use the selection subelements instead of the Sel Sets...
    if (edit) {
//         this->blockConnection(true);
        for(std::vector<std::string>::const_reverse_iterator it=subList.rbegin(); it != subList.rend(); ++it)
        {
            QRegExp rx(QString::fromAscii("^RenderMaterial(\\d+)$"));
            QString expr = QString::fromStdString((*it));
            int pos = expr.indexOf(rx);
            if (pos > -1) {
                bool ok;
                int index = rx.cap(1).toInt(&ok);
                if (!ok)
                  continue;

                // Remove the Given Render Material
                try {
                    Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.removeRenderMaterial(%i)",getObject()->getNameInDocument(), index);
                }
                catch (const Base::Exception& e) {
                    Base::Console().Error("%s\n", e.what());
                }
            }
        }
//         this->blockConnection(false);

        Gui::Selection().clearSelection();
        draw();
        // if in edit not delete the object
        return false;
    }
    // if not in edit delete the whole object
    return true;
}
