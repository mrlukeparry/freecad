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
# include <BRep_Tool.hxx>
# include <GeomAPI_ProjectPointOnSurf.hxx>
# include <GeomLProp_SLProps.hxx>
# include <Poly_Triangulation.hxx>
# include <TopoDS_Face.hxx>
# include <Inventor/SoInput.h>
# include <Inventor/nodes/SoNode.h>
# include <Inventor/nodes/SoOrthographicCamera.h>
# include <vector>
# include <Inventor/nodes/SoPerspectiveCamera.h>
# include <Inventor/SbViewVolume.h>
# include <QApplication>
# include <QMessageBox>
#endif

#include <Base/Console.h>
#include <Base/Exception.h>
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <App/Material.h>

#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/Command.h>
#include <Gui/Control.h>
#include <Gui/FileDialog.h>
#include <Gui/View.h>
#include <Gui/ViewProvider.h>
#include <Gui/Selection.h>
#include <Gui/FileDialog.h>
#include <Gui/MainWindow.h>
#include <Gui/View3DInventor.h>
#include <Gui/View3DInventorViewer.h>

#include <Mod/Part/App/PartFeature.h>

#include <Mod/Raytracing/App/renderer/lux/LuxRender.h>
#include <Mod/Raytracing/App/Appearances.h>
#include <Mod/Raytracing/Gui/ViewProviderRender.h>

#include "RenderView.h"
#include "TaskDlgAppearances.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Helpers
using namespace Raytracing;
using namespace RaytracingGui;

bool isToolActive(Gui::Document *doc)
{
    if (doc) {
      if (doc->getInEdit())
           // checks if a Sketch Viewprovider is in Edit and is in no special mode
        if (doc->getInEdit() && doc->getInEdit()->isDerivedFrom(RaytracingGui::ViewProviderRender::getClassTypeId())) {
            return true;
        }
    }
    return false;
}

RaytracingGui::ViewProviderRender* getRenderViewprovider(Gui::Document *doc)
{
    if (doc) {
        if (doc->getInEdit() && doc->getInEdit()->isDerivedFrom
            (RaytracingGui::ViewProviderRender::getClassTypeId()) )
            return dynamic_cast<RaytracingGui::ViewProviderRender*>(doc->getInEdit());
    }
    return 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===========================================================================
// Raytracing_AddAppearances
//===========================================================================

DEF_STD_CMD_A(CmdRaytracingAddAppearances);

CmdRaytracingAddAppearances::CmdRaytracingAddAppearances()
  : Command("Raytracing_AddAppearances")
{
    sAppModule      = "Raytracing";
    sGroup          = QT_TR_NOOP("Render");
    sMenuText       = QT_TR_NOOP("Add Appearances");
    sToolTipText    = QT_TR_NOOP("Add appearances");
    sWhatsThis      = "Raytracing_Add Appearancest";
    sStatusTip      = sToolTipText;
    sPixmap         = "Raytrace_New";
    eType           = ForEdit;
}

void CmdRaytracingAddAppearances::activated(int iMsg)
{
    // For now just close the current dialog
    Gui::Control().closeDialog();

    // clear the selection (convenience)
    Gui::Selection().clearSelection();
    
    Gui::Control().showDialog(new TaskDlgAppearances());
}

bool CmdRaytracingAddAppearances::isActive(void)
{
    return isToolActive(getActiveGuiDocument());
}


//===========================================================================
// CmdRaytracingCmdRaytracingCreateRenderFeature
//===========================================================================
DEF_STD_CMD_A(CmdRaytracingCreateRenderFeature);

CmdRaytracingCreateRenderFeature::CmdRaytracingCreateRenderFeature()
  :Command("Raytracing_CreateRenderFeature")
{
    sAppModule    = "Raytracing";
    sGroup        = QT_TR_NOOP("Render");
    sMenuText     = QT_TR_NOOP("Create a new Render Feature");
    sToolTipText  = QT_TR_NOOP("Create a new Render Feature");
    sWhatsThis    = sToolTipText;
    sStatusTip    = sToolTipText;
    sPixmap       = "Raytrace_Export";
}

void CmdRaytracingCreateRenderFeature::activated(int iMsg)
{
    // get all objects of the active document
    std::vector<Part::Feature*> DocObjects = getActiveGuiDocument()->getDocument()->getObjectsOfType<Part::Feature>();

    SoCamera *Cam;
     Gui::MDIView *mdi = Gui::Application::Instance->activeDocument()->getActiveView();
    if (mdi && mdi->isDerivedFrom(Gui::View3DInventor::getClassTypeId())) {
        Gui::View3DInventorViewer *viewer = static_cast<Gui::View3DInventor *>(mdi)->getViewer();
        Cam =  viewer->getCamera();
    } else {
        throw Base::Exception("Could Not Read Camera");
    }

    SbRotation camrot = Cam->orientation.getValue();
    SbViewVolume::ProjectionType CamType = Cam->getViewVolume().getProjectionType();
    
    SbVec3f upvec(0, 1, 0); // init to default up vector
    camrot.multVec(upvec, upvec);

    SbVec3f lookat(0, 0, -1); // init to default view direction vector
    camrot.multVec(lookat, lookat);

    SbVec3f pos = Cam->position.getValue();
    float Dist = Cam->focalDistance.getValue();

    RenderCamera::CamType camType;

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

    std::string FeatName = getUniqueObjectName("RenderFeature");

    openCommand("Raytracing create render feature");
    doCommand(Doc,"import Raytracing,RaytracingGui");
    doCommand(Doc,"App.activeDocument().addObject('Raytracing::RenderFeature','%s')",FeatName.c_str());

    doCommand(Doc,"App.activeDocument().%s.setRenderer('Lux')",FeatName.c_str());

    int x = 1024, y = 768;
    doCommand(Doc,"App.ActiveDocument.%s.setRenderSize(%i, %i)",FeatName.c_str(), x, y);
    doCommand(Doc,"App.ActiveDocument.%s.attachRenderCamera(Raytracing.RenderCamera())", FeatName.c_str());

    // Set the camera to current view's camera
    doCommand(Doc,"App.activeDocument().%s.setCamera(App.Vector(%f,%f,%f), App.Vector(%f,%f,%f), App.Vector(%f,%f,%f), App.Vector(%f,%f,%f), '%s')",
                   FeatName.c_str(), camPos.x, camPos.y, camPos.z,
                                     camDir.x, camDir.y, camDir.z,
                                     camUp.x, camUp.y, camUp.z,
                                     camLookAt.x, camLookAt.y, camLookAt.z, camTypeStr);

    doCommand(Doc,"App.ActiveDocument.%s.setRenderPreset('metropolisUnbiased')", FeatName.c_str());
    doCommand(Doc,"App.ActiveDocument.%s.setRenderTemplate('lux_default')", FeatName.c_str());

    doCommand(Gui,"Gui.activeDocument().setEdit('%s')",FeatName.c_str());
    commitCommand();
/*
    LuxRender *renderer = new LuxRender();
//     RenderCamera *camera = new RenderCamera(camPos, camDir, camLookAt, camUp, camType);
//     camera->Autofocus = true;
//     camera->Focaldistance = Dist;

    // Add Camera

    RenderAreaLight *light = new RenderAreaLight();
    light->setColor(255, 255, 255);
    light->Height = 100;
    light->Width = 100;

    Base::Rotation lightRot = Base::Rotation(Base::Vector3d(0, 1, 0), 0.);
    Base::Vector3d lightPos = Base::Vector3d(-50., -50., 200);
    light->setPlacement(lightPos, lightRot);

//     renderer->addCamera(camera);
    renderer->addLight(light);

    // Get a list of Materials
    std::string matPath = App::Application::getResourceDir() + "Mod/Raytracing/Materials/Lux";
    Appearances().setUserMaterialsPath(matPath.c_str());
    Appearances().scanMaterials();
  // go through all document objects
    for (std::vector<Part::Feature*>::const_iterator it=DocObjects.begin();it!=DocObjects.end();++it) {
        Gui::ViewProvider* vp = getActiveGuiDocument()->getViewProvider(*it);
        if (vp && vp->isVisible()) {
          float meshDev = 0.1;
          // See if we can obtain the user set mesh deviation // otherwise resort to a default
//           if(vp->getTypeId() == PartGui::ViewProviderPartExt::getClassTypeId()) {
//               meshDev = static_cast<PartGui::ViewProviderPartExt *>(vp)->Deviation.getValue();
//           }
            App::PropertyColor *pcColor = dynamic_cast<App::PropertyColor *>(vp->getPropertyByName("ShapeColor"));
            App::Color col = pcColor->getValue();

            RenderPart *part = new RenderPart((*it)->getNameInDocument(), (*it)->Shape.getValue(), meshDev);
//             const LibraryMaterial *gold = Appearances().getMaterialById("lux_extra_gold");
//             RenderMaterial *defaultMat = new RenderMaterial(gold);
//             part->setMaterial(defaultMat);

            const LibraryMaterial *matte = Appearances().getMaterialById("lux_default_matte");

            if(!matte) {
              Base::Console().Log("Material Not Found @");
              Base::Console().Log(matPath.c_str());
            } else {
              RenderMaterial *defaultMatte = new RenderMaterial(matte);

              MaterialFloatProperty *sigmaValue = new MaterialFloatProperty(0.5);
              MaterialColorProperty *colorValue = new MaterialColorProperty(col.r * 255, col.g * 255, col.b * 255);

              defaultMatte->Properties.insert(QString::fromAscii("Kd"), colorValue);
              defaultMatte->Properties.insert(QString::fromAscii("sigma"), sigmaValue);
//               part->setMaterial(defaultMatte);
            }
            renderer->addObject(part);
        }
    }*/
//      RenderPreset *preset = renderer->getRenderPreset("metropolisUnbiased");
// 
//      if(!preset) {
//         Base::Console().Log("Couldn't find Render Preset\n");
//      } else {
//         renderer->setRenderPreset(preset);
//      }
//     renderer->setRenderSize(800, 600);
//     renderer->preview();

//     if(renderer->getRenderProcess() && renderer->getRenderProcess())
//     {
//         Gui::Document * doc = getActiveGuiDocument();
//         RenderView *view = new RenderView(doc, Gui::getMainWindow());
// 
//         view->attachRender(renderer);
// 
//         view->setWindowTitle(QObject::tr("Render viewer") + QString::fromAscii("[*]"));
//         Gui::getMainWindow()->addWindow(view);
//     }

}

bool CmdRaytracingCreateRenderFeature::isActive(void)
{
    return getGuiApplication()->sendHasMsgToActiveView("GetCamera");
}


void CreateRenderCommands(void)
{
    Gui::CommandManager &rcCmdMgr = Gui::Application::Instance->commandManager();

    rcCmdMgr.addCommand(new CmdRaytracingAddAppearances());
    rcCmdMgr.addCommand(new CmdRaytracingCreateRenderFeature());

}
