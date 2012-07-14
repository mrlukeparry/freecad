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


#ifndef GUI_TASKVIEW_TaskSketchPlaneParameters_H
#define GUI_TASKVIEW_TaskSketchPlaneParameters_H

#include <Gui/TaskView/TaskView.h>
#include <Gui/Selection.h>
#include <Gui/TaskView/TaskDialog.h>

#include "ViewProviderSketchPlane.h"

class Ui_TaskSketchPlaneParameters;

namespace App {
class Property;
}

namespace Gui {
class ViewProvider;
}

namespace PartDesignGui {

class TaskSketchPlaneParameters : public Gui::TaskView::TaskBox, public Gui::SelectionObserver
{
    Q_OBJECT

public:
    TaskSketchPlaneParameters(ViewProviderSketchPlane *PlaneView,QWidget *parent = 0);
    ~TaskSketchPlaneParameters();

    int    getMode(void) const;
    double getOffset(void) const;
    bool   getReversed(void) const;
    double getOffsetX() const;
    double getOffsetY() const;
    double getOffsetZ() const;
    double getRotation() const;

private Q_SLOTS:
    void onOffsetXChanged(double);
    void onOffsetYChanged(double);
    void onOffsetZChanged(double);
    void onRotationChanged(double);
    void onReversed(bool);
    void onModeChanged(int);
    void onEntityChanged(QString);

protected:
    void changeEvent(QEvent *e);

private:
    void onSelectionChanged(const Gui::SelectionChanges& msg);
    void updateUI(int index);
    void togglePlane();
    void updatePlaneStatus();
    bool eventFilter(QObject *object, QEvent *event);

private:
    QWidget* proxy;
    QWidget* entSelected;
    Ui_TaskSketchPlaneParameters* ui;
    ViewProviderSketchPlane *PlaneView;
};

/// simulation dialog for the TaskView
class TaskDlgSketchPlaneParameters : public Gui::TaskView::TaskDialog
{
    Q_OBJECT

public:
    TaskDlgSketchPlaneParameters(ViewProviderSketchPlane *PlaneView);
    ~TaskDlgSketchPlaneParameters();

    ViewProviderSketchPlane* getPlaneView() const
    { return PlaneView; }


public:
    /// is called the TaskView when the dialog is opened
    virtual void open();
    /// is called by the framework if an button is clicked which has no accept or reject role
    virtual void clicked(int);
    /// is called by the framework if the dialog is accepted (Ok)
    virtual bool accept();
    /// is called by the framework if the dialog is rejected (Cancel)
    virtual bool reject();
    /// is called by the framework if the user presses the help button 
    virtual bool isAllowedAlterDocument(void) const
    { return false; }

    /// returns for Close and Help button 
    virtual QDialogButtonBox::StandardButtons getStandardButtons(void) const
    { return QDialogButtonBox::Ok|QDialogButtonBox::Cancel; }

protected:
    ViewProviderSketchPlane   *PlaneView;
    TaskSketchPlaneParameters *parameter;
};

} //namespace PartDesignGui

#endif // GUI_TASKVIEW_TASKAPPERANCE_H
