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

#include "RenderLights.h"
#include "RenderPreset.h"
#include "RenderProcess.h"
#include "Appearances.h"

#include <3rdParty/salomesmesh/inc/Rn.h>

class TopoDS_Shape;
class TopoDS_Face;

namespace Raytracing
{ 

// These are for storing actual material properties within a hash and are designed to be lightweight
class MaterialProperty
{
public:
  MaterialProperty(MaterialParameter::Type propType): type(propType) {};
  ~MaterialProperty() {};
  MaterialParameter::Type getType() { return type;}
private:
  MaterialParameter::Type type;
};

// Template class used for storing material types
// Property Type must be either FLOAT, COLOR, BOOL, TEXTURE, STRING
class MaterialBoolProperty : public MaterialProperty
{
public:
  MaterialBoolProperty (bool val)
  : MaterialProperty(MaterialParameter::BOOL), value(val) {}
  ~MaterialBoolProperty (){}

  void setValue(bool val) { val = value;}
  bool getValue(void) { return value; }
private:
  bool value;
};

class MaterialFloatProperty : public MaterialProperty
{
public:
  MaterialFloatProperty(float val)
  : MaterialProperty(MaterialParameter::FLOAT), value(val) {}
  ~MaterialFloatProperty(){}

  void setValue(float val) { val = value;}
  float getValue(void) { return value; }
private:
  float value;
};

class MaterialColorProperty : public MaterialProperty
{
public:
  MaterialColorProperty(int r, int b, int g)
  : MaterialProperty(MaterialParameter::COLOR) { setValue(r,g,b); }
  ~MaterialColorProperty(){}

  void setValue(int r, int b, int g)
  {
    color[0] = (float) r / 255;
    color[1] = (float) g / 255;
    color[2] = (float) b / 255;
  }
  const float * getValue(void) { return color; }
private:
  float color[3];
};

class RenderMaterial
{
public:
  RenderMaterial(const LibraryMaterial *mat) : material(mat) {}
  ~RenderMaterial()
  {
    
        // Delete all the parameters stored under the material
    QMap<QString, MaterialProperty *>::iterator it = properties.begin();
    while (it != properties.end()) {
        switch(it.value()->getType()) {
          case MaterialParameter::COLOR: {
            MaterialColorProperty *prop = static_cast<MaterialColorProperty*>(it.value());
            delete prop;
          } break;
          case MaterialParameter::BOOL: {
            MaterialBoolProperty *prop = static_cast<MaterialBoolProperty*>(it.value());
            delete prop;
          } break;
          case MaterialParameter::FLOAT: {
            MaterialFloatProperty *prop = static_cast<MaterialFloatProperty*>(it.value());
            delete prop;
          } break;
        }

        it.value() = 0;
        ++it;
    }
    properties.clear();
  }

  QMap<QString, MaterialProperty *> properties;
  const LibraryMaterial * getMaterial() const { return material; }
private:
  const LibraryMaterial *material;
};

class RenderCamera
{
public:
    enum CamType {
    PERSPECTIVE,
    ORTHOGRAPHIC,
    ENVIRONMENT,
    REALISTIC };

    RenderCamera(const Base::Vector3d& cCamPos,const Base::Vector3d& cCamDir,const Base::Vector3d& cLookAt, const Base::Vector3d& cUp, const CamType& cType = PERSPECTIVE)
                 : CamPos(cCamPos),
                   CamDir(cCamDir),
                   LookAt(cLookAt),
                   Up(cUp),
                   Type(cType)
    {autofocus = false;}

    CamType Type;
    Base::Vector3d  CamPos;
    Base::Vector3d  CamDir;
    Base::Vector3d  LookAt;
    Base::Vector3d  Up;
    float fov;
    float focaldistance;
    bool autofocus;
};

class RenderPart
{
public:
    RenderPart(const char *partName, const TopoDS_Shape &shape, float meshDev)
               : PartName(partName),Shape(shape), meshDeviation(meshDev) {}
    ~RenderPart()
    {
      delete material;
      material = 0;
    }
    const TopoDS_Shape getShape() {return Shape;}
    const char * getName() { return PartName;}
    const float getMeshDeviation() { return meshDeviation;}
    void setMaterial(RenderMaterial *mat) { material = mat; }
    RenderMaterial * getMaterial(void) { return material;}
private:
    const char *PartName;
    TopoDS_Shape Shape;
    float meshDeviation;
    RenderMaterial *material;
};

class AppRaytracingExport Renderer : public Base::BaseClass
{
    TYPESYSTEM_HEADER();
public:
    Renderer(void);
    ~Renderer(void);
    void addCamera(RenderCamera *cam);
    void addLight(RenderLight *light);
    void addObject(const char *PartName, const TopoDS_Shape &Shape, float meshDeviation);
    void addObject(RenderPart *part);
    void setRenderPreset(const char *presetId);
    void attachRenderProcess(RenderProcess *process);

    /// Functions for find Render Presets
    void scanPresets(void);
    std::vector<RenderPreset *> parsePresetXML(QString filename);
    void clearPresets(void);
    RenderPreset * getRenderPreset(const char *id) const;
    std::vector<RenderPreset *> getRenderPresets(void) const;

    ///Render Actions
    virtual void preview();
    virtual void render();
    virtual void finish();

    ///Setter methods
    void setCamera(const Base::Vector3d &camPos, const Base::Vector3d &CamDir, const Base::Vector3d &lookAt, const Base::Vector3d &Up);
    void setOutputPath(const char *loc);
    void setRenderSize(int x, int y) { xRes = x; yRes = y;};

    //virtual void loadSceneDefinition(const char *file);
    //virtual void Restore(Base::XMLReader &/*reader*/);

protected:

    /// All these methods must be defined by the Render plugin subclass because they generate the scene file used for rendering
    virtual std::string genCamera(RenderCamera *light) const = 0;
    virtual std::string genFace(const TopoDS_Face& aFace, int index ) = 0;
    virtual std::string genLight(RenderLight *light) const = 0;
    virtual std::string genObject(RenderPart *part) = 0;
    virtual std::string genMaterial(RenderMaterial *mat) = 0;
    virtual std::string genRenderProps() = 0;
    virtual void generateScene() = 0;

    bool getOutputStream(QTextStream &ts);

    void clear();

    /// Useful helpful function for calculating the mesh for individual face
    void transferToArray(const TopoDS_Face& aFace,gp_Vec** vertices,gp_Vec** vertexnormals, long** cons,int &nbNodesInFace,int &nbTriInFace );

    /// Common Properties
    RenderCamera *camera;
    RenderProcess *process;
    RenderPreset *preset;
    std::vector<RenderLight *> lights;
    std::vector<RenderPart *> parts;
    std::vector<RenderPreset *> libraryPresets;

    std::string outputPath;
    std::string renderPresetsPath;

    QTemporaryFile inputFile;

    int xRes;
    int yRes;
};

}
#endif //_Renderer_h_ 