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

#include "ViewProviderSketchPlane.h"
#include "TaskSketchPlaneParameters.h"
#include <Mod/PartDesign/App/FeatureSketchPlane.h>
#include <Gui/Control.h>
#include <Gui/Command.h>
#include <Gui/Application.h>

using namespace PartDesignGui;

PROPERTY_SOURCE(PartDesignGui::ViewProviderSketchPlane,PartGui::ViewProvider2DObject)

ViewProviderSketchPlane::ViewProviderSketchPlane()
{

}

ViewProviderSketchPlane::~ViewProviderSketchPlane()
{
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

void ViewProviderSketchPlane::unsetEdit(int ModNum)
{
    this->ShowGrid.setValue(false);
    if (ModNum == ViewProvider::Default) {
        // and update the pad
        //getSketchObject()->getDocument()->recompute();

        // when pressing ESC make sure to close the dialog
        Gui::Control().closeDialog();
    }
    else {
        PartGui::ViewProviderPart::unsetEdit(ModNum);
    }
}

bool ViewProviderSketchPlane::onDelete(const std::vector<std::string> &)
{
    // get the support and Sketch
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(getObject());

    return true;
}

