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

#include <Base/Console.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/MDIView.h>
#include <QDeclarativeContext>
#include <QDeclarativeView>
#include <QGraphicsObject>
#include <QGLWidget>
#include <Gui/Command.h>
#include <Mod/Raytracing/App/Appearances.h>

#include "TaskDlgAppearances.h"
#include "AppearancesModel.h"
#include <App/Application.h>

using namespace Raytracing;
using namespace RaytracingGui;


//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskDlgAppearances::TaskDlgAppearances()
    : TaskDialog()
{
    std::string matPath = App::Application::getResourceDir() + "Mod/Raytracing/Materials/Lux";
    Appearances().setUserMaterialsPath(matPath.c_str());
    Appearances().scanMaterials();

    AppearancesModel model;
    
    std::vector<LibraryMaterial *> materials = Appearances().getMaterialsByProvider("lux");
    for (std::vector<LibraryMaterial *>::const_iterator it= materials.begin(); it!= materials.end(); ++it) {
            model.addLibraryMaterial(*it);
    }

    QDeclarativeView *view = new QDeclarativeView ();
    QDeclarativeContext *ctxt = view->rootContext();
    ctxt->setContextProperty(QString::fromAscii("appearancesModel"), &model);

    view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    view->setSource(QUrl(QString::fromAscii("qrc:/qml/appearancesUi.qml"))); // Load the Main QML File

     // Connect an Update Signal when an image is available
    QObject *rootObject = view->rootObject();
    QObject::connect(rootObject, SIGNAL(materialDrag(QString)), this , SLOT(dragInit(QString)));
    
    this->Content.push_back(view);

    // Enable Drag and Drops on the 3D Inventor Window
//     Gui::MDIView *mdi = Gui::Application::Instance->activeDocument()->getActiveView();
//     if(mdi) {
//         mdi->setAcceptDrops(true);
//     }

}
void TaskDlgAppearances::dragInit(QString str)
{
    Base::Console().Log(str.toAscii());
    const LibraryMaterial *dragMaterial = Appearances().getMaterialById(str.toAscii());

    // ASSERT


    QDrag *drag = new QDrag(this->Content.back());
    QMimeData *mimeData = new QMimeData;

    mimeData->setText(str);
    drag->setMimeData(mimeData);
    QPixmap pixmap(dragMaterial->previewFilename);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(drag->pixmap().width()/2,
                            drag->pixmap().height()));
    Qt::DropAction dropAction = drag->exec();
}

TaskDlgAppearances::~TaskDlgAppearances()
{

    // Disable Drag and Drops on the 3D Inventor Window
//     Gui::MDIView *mdi = Gui::Application::Instance->activeDocument()->getActiveView();
//     if(mdi) {
//         mdi->setAcceptDrops(false);
//     }
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
    return true;
}

bool TaskDlgAppearances::reject()
{
//     std::string document = documentName; // needed because resetEdit() deletes this instance
//     Gui::Command::doCommand(Gui::Command::Gui,"Gui.getDocument('%s').resetEdit()", document.c_str());
//     Gui::Command::doCommand(Gui::Command::Doc,"App.getDocument('%s').recompute()", document.c_str());

    return true;
}

void TaskDlgAppearances::helpRequested()
{

}


#include "moc_TaskDlgAppearances.cpp"
