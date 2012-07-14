/***************************************************************************
 *   Copyright (c) 2012 Luke Parry         <l.parry@warwick.ac.uk>         *
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
# include <QMessageBox>
#endif

#include "ui_TaskSketchPlaneParameters.h"
#include "TaskSketchPlaneParameters.h"
#include <App/Application.h>
#include <App/Document.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/BitmapFactory.h>
#include <Gui/ViewProvider.h>
#include <Gui/WaitCursor.h>
#include <Base/Console.h>
#include <Gui/Selection.h>
#include <Gui/Command.h>
#include <Mod/PartDesign/App/FeatureSketchPlane.h>

#include "SelectionLineEdit.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskSketchPlaneParameters */

TaskSketchPlaneParameters::TaskSketchPlaneParameters(ViewProviderSketchPlane *PlaneView,QWidget *parent)
    : TaskBox(Gui::BitmapFactory().pixmap("PartDesign_SketchPlane"),tr("Sketch Plane Parameters"),true, parent),PlaneView(PlaneView)
{
    // we need a separate container widget to add all controls to
    proxy = new QWidget(this);
    ui = new Ui_TaskSketchPlaneParameters();
    ui->setupUi(proxy);
    QMetaObject::connectSlotsByName(this);

    QObject::connect(ui->doubleSpinBoxOffsetX, SIGNAL(valueChanged(double)),
            this, SLOT(onOffsetXChanged(double)));
    QObject::connect(ui->doubleSpinBoxOffsetY, SIGNAL(valueChanged(double)),
            this, SLOT(onOffsetYChanged(double)));
    QObject::connect(ui->doubleSpinBoxOffsetZ, SIGNAL(valueChanged(double)),
            this, SLOT(onOffsetZChanged(double)));

    QObject::connect(ui->doubleSpinBoxRotation, SIGNAL(valueChanged(double)),
        this, SLOT(onRotationChanged(double)));
    
    QObject::connect(ui->checkBoxReversed, SIGNAL(toggled(bool)),
            this, SLOT(onReversed(bool)));
    QObject::connect(ui->changeMode, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onModeChanged(int)));

    QObject::connect(ui->entity1, SIGNAL(textChanged(QString)),
            this, SLOT(onEntityChanged(QString)));
    QObject::connect(ui->entity2, SIGNAL(textChanged(QString)),
        this, SLOT(onEntityChanged(QString)));
    QObject::connect(ui->entity3, SIGNAL(textChanged(QString)),
    this, SLOT(onEntityChanged(QString)));

    this->groupLayout()->addWidget(proxy);

    ui->groupBox->setLayout(ui->horizontalLayout_3);

    // Get the feature data
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());

    double offsetX  = pcPlane->OffsetX.getValue();
    double offsetY  = pcPlane->OffsetY.getValue();
    double offsetZ  = pcPlane->OffsetZ.getValue();

    double rotation = pcPlane->Rotation.getValue();

    bool reversed = pcPlane->Reversed.getValue();

    int index = pcPlane->Type.getValue(); // must extract value here, clear() kills it!

    const std::vector<std::string> &sub1 = pcPlane->Entity1.getSubValues();
    const std::vector<std::string> &sub2 = pcPlane->Entity2.getSubValues();
    const std::vector<std::string> &sub3 = pcPlane->Entity3.getSubValues();

    // Fill data into dialog elements
    ui->doubleSpinBoxOffsetX->setMinimum(-INT_MAX);
    ui->doubleSpinBoxOffsetX->setMaximum(INT_MAX);
    ui->doubleSpinBoxOffsetX->setValue(offsetX);

    ui->doubleSpinBoxOffsetX->setMinimum(-INT_MAX);
    ui->doubleSpinBoxOffsetX->setMaximum(INT_MAX);
    ui->doubleSpinBoxOffsetX->setValue(offsetY);

    ui->doubleSpinBoxOffsetZ->setMinimum(-INT_MAX);
    ui->doubleSpinBoxOffsetZ->setMaximum(INT_MAX);
    ui->doubleSpinBoxOffsetZ->setValue(offsetZ);

    ui->doubleSpinBoxRotation->setMinimum(-179.99);
    ui->doubleSpinBoxRotation->setMaximum(179.99);
    ui->doubleSpinBoxRotation->setValue(rotation);

    ui->checkBoxReversed->setChecked(reversed);

    Part::Feature *feat1 = static_cast<Part::Feature*>(pcPlane->Entity1.getValue());
    Part::Feature *feat2 = static_cast<Part::Feature*>(pcPlane->Entity2.getValue());
    Part::Feature *feat3 = static_cast<Part::Feature*>(pcPlane->Entity3.getValue());

    if(feat1 && sub1.size() > 0) {
        QString str;
        QTextStream buf(&str);
        buf << feat1->getNameInDocument() << " "
            << sub1[0].c_str();
        ui->entity1->setText(str);
    }

    if(feat2 && sub2.size() > 0) {
        QString str;
        QTextStream buf(&str);
        buf << feat2->getNameInDocument() << " "
            << sub2[0].c_str();
        ui->entity2->setText(str);
    }

    if(feat3 && sub3.size() > 0) {
        QString str;
        QTextStream buf(&str);
        buf << feat3->getNameInDocument() << " "
            << sub3[0].c_str();
        ui->entity3->setText(str);
    }

    // Set as a Null Pointer
    entSelected = 0;

    ui->entity1->installEventFilter(this);
    ui->entity2->installEventFilter(this);
    ui->entity3->installEventFilter(this);

    ui->changeMode->clear();
    ui->changeMode->insertItem(0, tr("Points"));
    ui->changeMode->insertItem(1, tr("Face"));
    ui->changeMode->setCurrentIndex(index);

    // activate and de-activate dialog elements as appropriate
    updateUI(index);
}

TaskSketchPlaneParameters::~TaskSketchPlaneParameters()
{
  delete ui;
}

bool TaskSketchPlaneParameters::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
    {
        if (object == ui->entity1 || object == ui->entity2 || object == ui->entity3)
        {
            entSelected = qobject_cast<QWidget *>(object);
            SelectionLineEdit *lineEdit = static_cast<SelectionLineEdit *>(entSelected);
 
            // Update the line Edit status
            updatePlaneStatus();
            lineEdit->updateStatus(SelectionLineEdit::STATUS_SELECT);
        }
    }
    return false;
}

void TaskSketchPlaneParameters::updateUI(int index)
{
    if (index == 0) {  // three points
        ui->checkBoxReversed->setEnabled(true);

        ui->doubleSpinBoxOffsetX->setEnabled(true);
        ui->doubleSpinBoxOffsetY->setEnabled(true);
        ui->doubleSpinBoxOffsetZ->setEnabled(true);
        ui->doubleSpinBoxRotation->setEnabled(true);

        ui->entity1->setEnabled(true);
        ui->entity2->setEnabled(true);
        ui->entity3->show();
        ui->entityLabel3->show();
        ui->entity3->setEnabled(true);
    } else if (index == 1) { //face
        ui->checkBoxReversed->setEnabled(true);

        ui->doubleSpinBoxOffsetX->setEnabled(true);
        ui->doubleSpinBoxOffsetY->setEnabled(true);
        ui->doubleSpinBoxOffsetZ->setEnabled(true);
        ui->doubleSpinBoxRotation->setEnabled(true);
        
        ui->entity1->setEnabled(true);
        ui->entity2->setEnabled(true);

        ui->entity3->hide();
        ui->entity3->clear(); // Clear the Reference in this mode
        ui->entityLabel3->hide();
    } 
}

void TaskSketchPlaneParameters::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());

    if (!msg.pSubName || msg.pSubName[0] == '\0')
        return;

    std::vector<std::string> ref;
    ref.push_back(msg.pSubName);
    App::DocumentObject *docObj = pcPlane->getDocument()->getObject(msg.pObjectName);

    if(!docObj)
      return;

    if (msg.Type == Gui::SelectionChanges::AddSelection) {
        QString str;
        QTextStream buf(&str);
        buf << msg.pObjectName << " "
            << ref[0].c_str();

        if(!entSelected)
          return;

        QLineEdit * lineEdit = dynamic_cast<QLineEdit *>(entSelected);

        if(!lineEdit)
          return;

        if(lineEdit == ui->entity1) {
            pcPlane->Entity1.setValue(docObj,ref);
            pcPlane->getDocument()->recomputeFeature(pcPlane);
            lineEdit->setText(str);
        } else if(lineEdit == ui->entity2) {
            pcPlane->Entity2.setValue(docObj,ref);
            pcPlane->getDocument()->recomputeFeature( pcPlane);
            lineEdit->setText(str);
        } else if(lineEdit == ui->entity3) {
            pcPlane->Entity3.setValue(docObj,ref);
            pcPlane->getDocument()->recomputeFeature(pcPlane);
            lineEdit->setText(str);
        } else {
            return;
        }
        updatePlaneStatus();

        //Reset Entity Selection
        entSelected = 0;
        lineEdit->clearFocus();

        //Clear Selection
        Gui::Selection().clearSelection();
    }
}

void TaskSketchPlaneParameters::togglePlane()
{
    PlaneView->ShowGrid.setValue(PlaneView->getObject()->isValid());
}

void TaskSketchPlaneParameters::updatePlaneStatus()
{
    togglePlane();
    SelectionLineEdit::EditMode mode;
    if(ui->entity1->text().isEmpty() && ui->entity2->text().isEmpty() && ui->entity3->text().isEmpty())
        mode = SelectionLineEdit::STATUS_EMPTY;
    else if(PlaneView->getObject()->isValid())
        mode = SelectionLineEdit::STATUS_VALID;
    else
        mode = SelectionLineEdit::STATUS_INVALID;

    ui->entity1->updateStatus(mode);
    ui->entity2->updateStatus(mode);
    ui->entity3->updateStatus(mode);
}

void TaskSketchPlaneParameters::onRotationChanged(double rotation)
{
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());
    pcPlane->Rotation.setValue((float)rotation);
    pcPlane->getDocument()->recomputeFeature(pcPlane);
    updatePlaneStatus();
}

void TaskSketchPlaneParameters::onOffsetXChanged(double offset)
{
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());
    pcPlane->OffsetX.setValue((float)offset);
    pcPlane->getDocument()->recomputeFeature(pcPlane);
    updatePlaneStatus();
}

void TaskSketchPlaneParameters::onOffsetYChanged(double offset)
{
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());
    pcPlane->OffsetY.setValue((float)offset);
    pcPlane->getDocument()->recomputeFeature(pcPlane);
    updatePlaneStatus();
}

void TaskSketchPlaneParameters::onOffsetZChanged(double offset)
{
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());
    pcPlane->OffsetZ.setValue((float)offset);
    pcPlane->getDocument()->recomputeFeature(pcPlane);
    updatePlaneStatus();
}

void TaskSketchPlaneParameters::onEntityChanged(QString s)
{
    // Find and cast the sender - we can assume it is from a SelectionLineEdit widget
    QWidget *selected = qobject_cast<QWidget *>(QObject::sender());
    QLineEdit * lineEdit = dynamic_cast<QLineEdit *>(selected);
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());

    // If the text is now empty, the entity reference should be deleted
    if(s.isEmpty())
    {
        // Declare an empty string
        std::vector<std::string> str;
        // Set to a null poitner
        if(lineEdit == ui->entity1) {
            pcPlane->Entity1.setValue(0, str);
            pcPlane->getDocument()->recomputeFeature(pcPlane);
        } else if(lineEdit == ui->entity2) {
            pcPlane->Entity2.setValue(0, str);
            pcPlane->getDocument()->recomputeFeature( pcPlane);
        } else if(lineEdit == ui->entity3) {
            pcPlane->Entity3.setValue(0, str);
            pcPlane->getDocument()->recomputeFeature(pcPlane);
        }
        updatePlaneStatus();
    }
}

void TaskSketchPlaneParameters::onReversed(bool on)
{
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());
    pcPlane->Reversed.setValue(on);
    pcPlane->getDocument()->recomputeFeature(pcPlane);
    updatePlaneStatus();
}


void TaskSketchPlaneParameters::onModeChanged(int index)
{
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());

    switch (index) {
      default:
        case 0: pcPlane->Type.setValue("Points"); break;
        case 1: pcPlane->Type.setValue("Face"); break;
    }

    updateUI(index);

    pcPlane->getDocument()->recomputeFeature(pcPlane);
    updatePlaneStatus();
}

double TaskSketchPlaneParameters::getOffsetX(void) const
{
    return ui->doubleSpinBoxOffsetX->value();
}

double TaskSketchPlaneParameters::getOffsetY(void) const
{
    return ui->doubleSpinBoxOffsetY->value();
}

double TaskSketchPlaneParameters::getOffsetZ(void) const
{
    return ui->doubleSpinBoxOffsetZ->value();
}

double TaskSketchPlaneParameters::getRotation(void) const
{
    return ui->doubleSpinBoxRotation->value();
}

bool   TaskSketchPlaneParameters::getReversed(void) const
{
    return ui->checkBoxReversed->isChecked();
}

int TaskSketchPlaneParameters::getMode(void) const
{
    return ui->changeMode->currentIndex();
}

void TaskSketchPlaneParameters::changeEvent(QEvent *e)
{
    TaskBox::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(proxy);
    }
}

//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskDlgSketchPlaneParameters::TaskDlgSketchPlaneParameters(ViewProviderSketchPlane *PlaneView)
    : TaskDialog(),PlaneView(PlaneView)
{
    assert(PlaneView);
    parameter  = new TaskSketchPlaneParameters(PlaneView);

    Content.push_back(parameter);
}

TaskDlgSketchPlaneParameters::~TaskDlgSketchPlaneParameters()
{

}

//==== calls from the TaskView ===============================================================


void TaskDlgSketchPlaneParameters::open()
{
    
}

void TaskDlgSketchPlaneParameters::clicked(int)
{
    
}

bool TaskDlgSketchPlaneParameters::accept()
{
    std::string name = PlaneView->getObject()->getNameInDocument();
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());

    const std::vector<std::string> &sub1 = pcPlane->Entity1.getSubValues();
    const std::vector<std::string> &sub2 = pcPlane->Entity2.getSubValues();
    const std::vector<std::string> &sub3 = pcPlane->Entity3.getSubValues();

    Part::Feature *feat1 = static_cast<Part::Feature*>(pcPlane->Entity1.getValue());
    Part::Feature *feat2 = static_cast<Part::Feature*>(pcPlane->Entity2.getValue());
    Part::Feature *feat3 = static_cast<Part::Feature*>(pcPlane->Entity3.getValue());

    try {
        //Gui::Command::openCommand("Pad changed");
        Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Type = %u",name.c_str(),parameter->getMode());
        Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.OffsetZ = %f",name.c_str(),parameter->getOffsetZ());
        Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Reversed = %i",name.c_str(),parameter->getReversed()? 1:0);

        if(feat1 && sub1.size() == 1)
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Entity1 = (App.ActiveDocument.%s, [\"%s\"])"
                                                    ,name.c_str(), feat1->getNameInDocument(), sub1[0].c_str());

        if(feat2 && sub2.size() == 1)
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Entity2 = (App.ActiveDocument.%s, [\"%s\"])"
                                                    ,name.c_str(), feat2->getNameInDocument(), sub2[0].c_str());

        if(feat3 && sub3.size() == 1)
        Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Entity3 = (App.ActiveDocument.%s, [\"%s\"])"
                                                 ,name.c_str(), feat3->getNameInDocument(), sub3[0].c_str());

        Gui::Command::doCommand(Gui::Command::Doc,"App.activeDocument().recompute()");
        if (!PlaneView->getObject()->isValid())
            throw Base::Exception(PlaneView->getObject()->getStatusString());
        Gui::Command::doCommand(Gui::Command::Gui,"Gui.activeDocument().resetEdit()");
        Gui::Command::commitCommand();
    }
    catch (const Base::Exception& e) {
        QMessageBox::warning(parameter, tr("Input error"), QString::fromAscii(e.what()));
        return false;
    }

    return true;
}

bool TaskDlgSketchPlaneParameters::reject()
{
    // get the support and Sketch
    PartDesign::SketchPlane* pcPlane = static_cast<PartDesign::SketchPlane*>(PlaneView->getObject());

    // role back the done things
    Gui::Command::abortCommand();
    Gui::Command::doCommand(Gui::Command::Gui,"Gui.activeDocument().resetEdit()");

    //Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.recompute()");
    //Gui::Command::commitCommand();

    return true;
}

#include "moc_TaskSketchPlaneParameters.cpp"
#include "moc_SelectionLineEdit.cpp"
