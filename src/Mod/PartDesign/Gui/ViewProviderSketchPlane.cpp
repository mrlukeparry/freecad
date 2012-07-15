/***************************************************************************
 *   Copyright (c) 2012 Luke Parry           <l.parry@warwick.ac.uk>       *
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

#include <Gui/Control.h>
#include <Gui/Command.h>
#include <Gui/Application.h>

# include <Inventor/SbVec3f.h>
# include <Inventor/nodes/SoSeparator.h>
# include <Inventor/nodes/SoTransform.h>
# include <Inventor/nodes/SoRotation.h>

#include "ViewProviderSketchPlane.h"
#include "TaskSketchPlaneParameters.h"

#include <Mod/PartDesign/App/FeatureSketchPlane.h>

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderSketchPlane,PartGui::ViewProvider2DObject)

ViewProviderSketchPlane::ViewProviderSketchPlane() :edit(false)
{
}

ViewProviderSketchPlane::~ViewProviderSketchPlane()
{
}

void ViewProviderSketchPlane::draggerMotionCallback(void *data, SoDragger *dragger)
{
    static_cast<ViewProviderSketchPlane*>(data)->draggerMotionCallback(dragger);
}

void ViewProviderSketchPlane::draggerMotionCallback(SoDragger *dragger)
{
    TranslationDragger *transDrag = static_cast<TranslationDragger *>(dragger);

    SbVec3f distance = transDrag->translation.getValue();
    PartDesign::SketchPlane* planeObj = static_cast<PartDesign::SketchPlane*>(pcObject);
    planeObj->OffsetZ.setValue(distance[0]);
    offsetZChanged(distance[0]);
    planeObj->recompute();
}

void ViewProviderSketchPlane::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    QAction* act;
    act = menu->addAction(QObject::tr("Edit sketch plane"), receiver, member);
    act->setData(QVariant((int)ViewProvider::Default));
    PartGui::ViewProviderPart::setupContextMenu(menu, receiver, member);
}

bool ViewProviderSketchPlane::setEdit(int ModNum)
{
    if (ModNum == ViewProvider::Default ) {
        // When double-clicking on the item for this pad the
        // object unsets and sets its edit mode without closing
        // the task panel
        Gui::TaskView::TaskDialog *dlg = Gui::Control().activeDialog();
        TaskDlgSketchPlaneParameters *planeDlg = qobject_cast<TaskDlgSketchPlaneParameters *>(dlg);
        if (planeDlg && planeDlg->getPlaneView() != this)
            planeDlg = 0; 
        if (dlg && !planeDlg) {
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

        // clear the selection (convenience)
        Gui::Selection().clearSelection();
        //if (ModNum == 1)
        //    Gui::Command::openCommand("Change pad parameters");

        createInventorNodes();
        edit = true;

        // start the edit dialog
        if (planeDlg)
            Gui::Control().showDialog(planeDlg);
        else
            Gui::Control().showDialog(new TaskDlgSketchPlaneParameters(this));


        return true;
    }
    else {
        return ViewProviderPart::setEdit(ModNum);
    }
}

void ViewProviderSketchPlane::createInventorNodes()
{
    vpRoot = new SoSeparator();
    vpRoot->renderCaching = SoSeparator::OFF; // Disable Rendercaching since Dragger scales with zoomTransform

    transDragger = new TranslationDragger();
    transform = new SoTransform();
    switchNode = new SoSwitch();

    transDragger->addMotionCallback(draggerMotionCallback,this);
    switchNode->addChild(transform);
    switchNode->addChild(transDragger);

    // Initialise the Transdragger with the correct distance
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(pcObject);
    if(pcPlane->isValid()) {
        SbVec3f dist(pcPlane->OffsetZ.getValue(), 0.f, 0.f);
        transDragger->translation.setValue(dist);
    }
    vpRoot->addChild(switchNode);

    vpRoot->ref();
    pcRoot->addChild(vpRoot);
}

void ViewProviderSketchPlane::unsetEdit(int ModNum)
{
    this->ShowGrid.setValue(false);
    vpRoot->removeAllChildren();
    pcRoot->removeChild(vpRoot);

    edit = false;
    // and update the sketch
    //pcObject->getDocument()->recompute();

    // when pressing ESC make sure to close the dialog
    Gui::Control().closeDialog();
}

bool ViewProviderSketchPlane::onDelete(const std::vector<std::string> &)
{
    // get the support and Sketch

    return true;
}

void ViewProviderSketchPlane::onChanged(const App::Property* prop)
{
    // call father
    PartGui::ViewProvider2DObject::onChanged(prop);
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(pcObject);

    if(!edit)
      return;

    if(pcPlane->isValid())
    {
        switchNode->whichChild.setValue(-3); //All are enabled
        Base::Vector3d axis(0,1,0);
        bool reverse = pcPlane->Reversed.getValue();
        double angle = -M_PI_2;
        if(reverse)
            angle *= -1;
        Base::Rotation rot(axis, angle);
        SbVec3f pos = transDragger->translation.getValue();
        double q1,q2,q3,q4;
        rot.getValue(q1,q2,q3,q4);
        transform->rotation.setValue((float) q1,(float) q2, (float) q3, (float)q4);
        float distance = -pos[0];
        if(reverse)
          distance *= -1.f;
        transform->translation.setValue(SbVec3f ( 0.f, 0.f, distance));
    } else {
        switchNode->whichChild.setValue(-2); // Hides the dragger
    }
}

void ViewProviderSketchPlane::updateData(const App::Property *prop)
{
    PartGui::ViewProvider2DObject::updateData(prop);
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(pcObject);

    if(!edit)
      return;
    if(pcPlane->isValid())
    {
        switchNode->whichChild.setValue(-3); //All are enabled
        Base::Vector3d axis(0,1,0);
        bool reverse = pcPlane->Reversed.getValue();
        double angle = -M_PI_2;
        if(reverse)
            angle *= -1;
        Base::Rotation rot(axis, angle);
        SbVec3f pos = transDragger->translation.getValue();
        double q1,q2,q3,q4;
        rot.getValue(q1,q2,q3,q4);
        transform->rotation.setValue((float) q1,(float) q2, (float) q3, (float)q4);

        float distance = -pos[0];
        if(reverse)
          distance *= -1.f;
        transform->translation.setValue(SbVec3f ( 0.f, 0.f, distance));
    } else {
        switchNode->whichChild.setValue(-2); // Hides the dragger
    }
}

