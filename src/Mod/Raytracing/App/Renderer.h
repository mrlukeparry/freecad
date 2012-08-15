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


#ifndef _Renderer_h_
#define _Renderer_h_

#include <QTemporaryFile>
#include <QTextStream>

#include <Base/Vector3D.h>
#include <Base/BaseClass.h>

# include <gp_Vec.hxx>
#include <vector>

#include "Appearances.h"
#include "RenderMaterial.h"
#include "RenderCamera.h"
#include "RenderLights.h"
#include "RenderPreset.h"
#include "RenderProcess.h"
#include "RenderTemplate.h"

#include <3rdParty/salomesmesh/inc/Rn.h>

class TopoDS_Shape;
class TopoDS_Face;

namespace Raytracing
{ 

 /// Just a helper class
class RenderPart
{
public:
    RenderPart(const char *partName, const TopoDS_Shape &shape, float meshDev)
               : PartName(partName),Shape(shape), meshDeviation(meshDev) {}
    ~RenderPart(){}
    const TopoDS_Shape getShape() {return Shape;}
    const char * getName() { return PartName;}
    const float getMeshDeviation() { return meshDeviation;}

private:
    const char *PartName;
    TopoDS_Shape Shape;
    float meshDeviation;
};

class RaytracingExport Renderer : public Base::BaseClass
{
    TYPESYSTEM_HEADER();
public:
    enum RenderMode {
      PREVIEW_AREA,
      PREVIEW,
      RENDER};

    Renderer(void);
    ~Renderer(void);
    void addCamera(RenderCamera *cam);
    void addLight(RenderLight *light);
    void addObject(const char *PartName, const TopoDS_Shape &Shape, float meshDeviation);
    void addObject(RenderPart *part);

    void attachRenderProcess(RenderProcess *process);
    RenderProcess * getRenderProcess() { return process; }

    /// Functions for find Render Presets
    std::vector<RenderPreset *> parsePresetXML(QString filename);
    RenderPreset * getRenderPreset(const char *id) const;
    std::vector<RenderPreset *> getRenderPresets(void) const;
    void clearPresets(void);
    void scanPresets(void);
    void setRenderPreset(const char *presetId);
    void setRenderPreset(RenderPreset *preset);

    /// Functions for finding Render Templates;
    std::vector<RenderTemplate *> parseTemplateXML(QString filename);
    RenderTemplate * getRenderTemplate(const char *id) const;
    std::vector<RenderTemplate *> getRenderTemplates(void) const;
    void clearTemplates(void);
    void scanTemplates(void);
    void setRenderTemplate(const char *presetId);
    void setRenderTemplate(RenderTemplate *preset);

    /// Functions related to Render Materials
    void attachRenderMaterials(const std::vector<RenderMaterial *> &mats);
    std::vector<RenderMaterial *>getRenderPartMaterials(RenderPart *part) const;
    bool hasCamera(void) { return (camera == 0) ? false: true; }
    const char * getOutputPath() const { return outputPath.c_str(); }

    ///Render Actions
    virtual void finish();
    virtual void preview();
    virtual void preview(int x1, int y1, int x2, int y2);
    virtual void render();
    virtual void reset();

    /// Get methods
    RenderCamera * getCamera() { return camera; }
    ///Setter methods
    void setCamera(const Base::Vector3d &camPos, const Base::Vector3d &CamDir, const Base::Vector3d &Up, const Base::Vector3d &lookAt);
    void setOutputPath(const char *loc) { outputPath = loc; }
    void setRenderSize(int x, int y) { xRes = x; yRes = y;}
    void setUpdateInteval(int msecs) { updateInterval = msecs; }
    void setBBox(const Base::Vector3d &min, const Base::Vector3d &max) { bbMin = min; bbMax = max; } 

    //virtual void loadSceneDefinition(const char *file);
    //virtual void Restore(Base::XMLReader &/*reader*/);

protected:
    virtual void initRender(RenderMode mode);
    /// All these methods must be defined by the Render plugin subclass because they generate the scene file used for rendering
    virtual QString genCamera(RenderCamera *light) const = 0;
    virtual QString genFace(const TopoDS_Face& aFace, int index ) = 0;
    virtual QString genLight(RenderLight *light) const = 0;
    virtual QString genObject(RenderPart *part) = 0;
    virtual QString genMaterial(RenderMaterial *mat) = 0;
    virtual QString genRenderProps() = 0;
    virtual QString genRenderTemplate() = 0;
    virtual void generateScene() = 0;

    bool getOutputStream(QTextStream &ts);

    void clear();

    /// Useful helpful function for calculating the mesh for individual face
    void transferToArray(const TopoDS_Face& aFace,gp_Vec** vertices,gp_Vec** vertexnormals, long** cons,int &nbNodesInFace,int &nbTriInFace );

    /// Common Properties
    RenderCamera *camera;
    RenderProcess *process;
    RenderPreset *preset;
    RenderTemplate *renderTemplate;

    std::vector<RenderLight *> lights;
    std::vector<RenderMaterial *> materials;
    std::vector<RenderPart *> parts;

    /// Library Collections
    std::vector<RenderPreset *> libraryPresets;
    std::vector<RenderTemplate *> libraryTemplates;

    /// Path storage
    std::string outputPath;
    std::string renderPresetsPath;
    std::string renderTemplatesPath;

    QTemporaryFile inputFile;

    Base::Vector3d bbMin, bbMax; // Bounding Box

    int xRes;
    int yRes;
    int updateInterval;
    RenderMode mode;
    int previewCoords[4];
};

}
#endif //_Renderer_h_ 