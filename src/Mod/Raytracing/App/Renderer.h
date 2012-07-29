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
  MaterialBoolProperty (MaterialParameter::Type propType, bool val)
  : MaterialProperty(propType), value(val) {}
  ~MaterialBoolProperty (){}

  void setValue(bool val) { val = value;}
  bool getValue(void) { return value; }
private:
  bool value;
};

class MaterialFloatProperty : public MaterialProperty
{
public:
  MaterialFloatProperty(MaterialParameter::Type propType, float val)
  : MaterialProperty(propType), value(val) {}
  ~MaterialFloatProperty(){}

  void setValue(float val) { val = value;}
  float getValue(void) { return value; }
private:
  float value;
};

// class MaterialColorProperty : public MaterialProperty
// {
//   MaterialPropertyValue(MaterialParameter::Type propType, T val)
//   : MaterialProperty(propType), value(val) {}
//   ~MaterialPropertyValue(){}
// 
//   void setVal(T val) { val = value;}
//    getVal(void) { return value; }
// private:
//   T value;
// };

class RenderMaterial
{
public:
  RenderMaterial(const LibraryMaterial *mat) : material(mat) {}
  ~RenderMaterial()
  {
        // Delete all the parameters stored under the material
    QMap<QString, MaterialProperty *>::iterator it = properties.begin();
    while (it != properties.end()) {
        delete (it.value());
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

class RenderLight
{
public:
    enum LightType {
    AREA,
    INFINITE,
    POINT,
    SPOT };

    RenderLight(){}
    ~RenderLight(){}
    LightType Type;
    Base::Vector3d  Pos;
    float Color[3];
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
    void attachRenderProcess(RenderProcess *process);
    virtual void preview();
    virtual void render();
    virtual void finish();

    virtual void generateScene() = 0;
    void setCamera(const Base::Vector3d &camPos, const Base::Vector3d &CamDir, const Base::Vector3d &lookAt, const Base::Vector3d &Up);
    void setOutputPath(const char *loc);
    void setRenderSize(int x, int y) { xRes = x; yRes = y;};

    //virtual void loadSceneDefinition(const char *file);
    //virtual void Restore(Base::XMLReader &/*reader*/);

protected:

    //All these methods must be defined by the subclass as they determine the actual output
    virtual std::string genCamera(RenderCamera *light) const = 0;
    virtual std::string genFace(const TopoDS_Face& aFace, int index ) = 0;
    virtual std::string genLight(RenderLight *light) const = 0;
    virtual std::string genObject(RenderPart *part) = 0;
    virtual std::string genMaterial(RenderMaterial *mat) = 0;
    virtual std::string genRenderProps() = 0;

    bool getOutputStream(QTextStream &ts);

    void clear();
    void transferToArray(const TopoDS_Face& aFace,gp_Vec** vertices,gp_Vec** vertexnormals, long** cons,int &nbNodesInFace,int &nbTriInFace );

    // Common Properties
    RenderCamera *camera;
    RenderProcess *process;
    std::vector<RenderLight *> lights;
    std::vector<RenderPart *> parts;
    std::string outputPath;

    QTemporaryFile inputFile;

    int xRes;
    int yRes;

};

}
#endif //_Renderer_h_ 