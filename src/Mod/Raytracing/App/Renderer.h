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

class TopoDS_Shape;
class TopoDS_Face;

namespace Raytracing
{ 

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
    const char * getName() { return PartName;};
    const float getMeshDeviation() { return meshDeviation;}
    setMaterial(Material *mat) { material = mat; }
    Material * getMaterial(void) { return material;}
private:
    const char *PartName;
    TopoDS_Shape Shape;
    float meshDeviation;
    Material *material;
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
    void addPart(RenderPart *part);
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
    virtual std::string genLight(RenderLight *light) const = 0;
    virtual std::string genObject(const char *PartName, const TopoDS_Shape& Shape, float meshDeviation) = 0;
    virtual std::string genFace(const TopoDS_Face& aFace, int index ) = 0;
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