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

#include "../../PreCompiled.h"

#ifndef _PreComp_
# include <BRep_Tool.hxx>
# include <BRepMesh_IncrementalMesh.hxx>
# include <GeomAPI_ProjectPointOnSurf.hxx>
# include <GeomLProp_SLProps.hxx>
# include <Poly_Triangulation.hxx>
# include <TopExp_Explorer.hxx>
# include <TopoDS_Face.hxx>
# include <sstream>
#endif

#include <App/Application.h>
#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Sequencer.h>

#include "LuxRender.h"
#include "LuxRenderProcess.h"

using Base::Console;

using namespace Raytracing;
using namespace std;


// #SAMPLE FOR LUX RENDERER!
// #Global Information
// LookAt 0 10 100 0 -1 0 0 1 0
// Camera "perspective" "float fov" [30]
// 
// Film "fleximage"
// "integer xresolution" [200] "integer yresolution" [200]
// 
// PixelFilter "mitchell" "float xwidth" [2] "float ywidth" [2]
// 
// Sampler "lowdiscrepancy" "string pixelsampler" ["lowdiscrepancy"]
// 
// #Scene Specific Information
// WorldBegin
// 
// AttributeBegin
//         CoordSysTransform "camera"
//         LightSource "distant"
//                 "point from" [0 0 0] "point to" [0 0 1]
//                 "color L" [3 3 3]
// AttributeEnd
// 
// AttributeBegin
//         Rotate 135 1 0 0
// 
//         Texture "checks" "color" "checkerboard"
//                 "float uscale" [4] "float vscale" [4]
//                 "color tex1" [1 0 0] "color tex2" [0 0 1]
// 
//         Material "matte"
//                 "texture Kd" "checks"
//         Shape "disk" "float radius" [20] "float height" [-1]
// AttributeEnd
// 
// WorldEnd

LuxRender::LuxRender()
{
    //Each renderer must declare a name used to distinguish itself from other render backends
    providerName = "lux";

    renderPresetsPath   = App::Application::getResourceDir() + "Mod/Raytracing/Presets/Lux";
    materialsPath       = App::Application::getResourceDir() + "Mod/Raytracing/Materials/Lux";
    renderTemplatesPath = App::Application::getResourceDir() + "Mod/Raytracing/Templates/Lux";

    scanPresets();
    scanTemplates();
}

LuxRender::~LuxRender(void){}

void LuxRender::generateScene()
{
    QTextStream out;
    if(!getOutputStream(out))
      return;

    out << "#Global Information:" << endl
        << genRenderProps()
        << genCamera(camera)
        << "#Scene Specific Information:" << endl
        << "WorldBegin" << endl;

    // Add the template
    out << genRenderTemplate();

    for (std::vector<RenderLight *>::iterator it = lights.begin(); it != lights.end(); ++it) {
        out << genLight(*it);
    }
    for (std::vector<RenderPart *>::iterator it = parts.begin(); it != parts.end(); ++it) {
        out << genObject(*it);
    }

    out << "\nWorldEnd";
}

QString LuxRender::genLight(RenderLight *light) const
{
    QString outStr;
    QTextStream out(&outStr);

    out <<  "\nAttributeBegin " << endl
        <<  "\tLightGroup \"default\"" << endl;

    // Switch the camera type
    switch(light->LightType) {
      case RenderLight::AREA: {

        const float * color = light->Color;
        out << "\n\tMaterial \"matte\" \"color Kd\" [" << color[0] << " " << color[1] << " " << color[2] << "]" << endl;

        // Actual area light
        out << "\n\tAreaLightSource \"area\"" << endl
            << "\t\t\"float power\" [" << light->Power << "]" << endl
            << "\t\t\"color L\" [" << color[0] << " " << color[1] << " " << color[2] << "]" << endl;

        // Create the area light
        // Generate Points for plane centered around origin
        RenderAreaLight *areaLight = static_cast<RenderAreaLight *>(light);
        Base::Vector3d pnts[4];
        areaLight->generateGeometry(pnts);

        // More Efficient to create geometry straight away
        out << "Shape \"trianglemesh\"" << endl
            << "\t\"integer indices\" [0 2 3 0 3 1]" << endl
            << "\t\"point P\" [" << pnts[0][0] << " " << pnts[0][1] << " " << pnts[0][2] << " "
                                 << pnts[1][0] << " " << pnts[1][1] << " " << pnts[1][2] << " "
                                 << pnts[2][0] << " " << pnts[2][1] << " " << pnts[2][2] << " "
                                 << pnts[3][0] << " " << pnts[3][1] << " " << pnts[3][2] << "]" << endl
            << "\t\"string name\" [\"Lamp\"]" << endl;
      } break;
      case RenderLight::DISTANT: {
        out <<  "\tCoordSysTransform \"camera\"" << endl
            <<  "\n\tLightSource \"distant\"" << endl
            <<  "\t\t\"point from\" [0 0 0] \"point to\" [0 0 1]" << endl
            <<  "\t\t\"color L\" [3 3 3]" << endl;
      }
    }
    out <<  "AttributeEnd" << endl;

    return outStr;
}
QString LuxRender::genCamera(RenderCamera *camera) const
{
    QString outStr;
    QTextStream out(&outStr);

    if(!camera)
        return  outStr;

    QString camType;
    // Switch the camera type
    switch(camera->Type) {
      case RenderCamera::ORTHOGRAPHIC:
        camType = QString::fromAscii("orthographic"); break;
      default:
      case RenderCamera::PERSPECTIVE:
        camType = QString::fromAscii("perspective"); break;
    }

    out << "\nLookAt " << camera->CamPos.x << " " << camera->CamPos.y << " " << camera->CamPos.z << " "
                      << camera->LookAt.x  << " " << camera->LookAt.y << " " << camera->LookAt.z << " "
                      << camera->Up.x      << " " << camera->Up.y     << " " << camera->Up.z     << "\n " << endl;

    out << "# Camera Declaration and View Direction" << endl
        << "Camera \"" << camType << "\" ";
    if(camera->Type == RenderCamera::PERSPECTIVE) {
        out << "\t\"float fov\" [" << camera->Fov << "]" << endl;
        if(camera->Autofocus)
            out << "\t\"bool autofocus\" [\"true\"]" << endl;
    }

    out << "\t\"float focaldistance\" [" << camera->Focaldistance << "]" << endl;

    // Process Preview Areas
    float x1, y1, x2, y2;
    if(mode == PREVIEW_AREA) {
        x1 = (float) previewCoords[0] / xRes * 2 - 1;
        y1 = (float) previewCoords[1] / yRes * 2 - 1;
        x2 = (float) previewCoords[2] / xRes * 2 - 1;
        y2 = (float) previewCoords[3] / yRes * 2 - 1;
        out << "\t\"float screenwindow\" [" << x1 << " " << y1 << " " << x2 << " " << y2 << "]";
    }
    return outStr;
}

QString LuxRender::genFace(const TopoDS_Face& aFace, int index )
{
    QString outStr;
    QTextStream out(&outStr);

    // this block mesh the face and transfers it in a C array of vertices and face indexes
    Standard_Integer nbNodesInFace,nbTriInFace;
    gp_Vec* verts=0;
    gp_Vec* vertNorms=0;
    long* cons=0;

    transferToArray(aFace,&verts,&vertNorms,&cons,nbNodesInFace,nbTriInFace);

    if (!verts)
      return outStr;

    out << "#Face Number: " << index << endl
        << "AttributeBegin" << endl
        << "\tShape \"trianglemesh\" \"string name\" \"Face " << index << "\"" << endl;

    // Write the Vertex Points in order
    out << "\n\t\"point P\"" << endl
        << "\t[" << endl;
    for (int i = 0; i < nbNodesInFace; i++) {
        out << "\t\t" <<  verts[i].X() << " " << verts[i].Y() << " " << verts[i].Z() << " " << endl;
    }
    out << "\t]" << endl; // End Property

    // Write the Normals in order
    out << "\n\t\"normal N\"" << endl
        << "\t[" << endl;
    for (int j = 0; j < nbNodesInFace; j++) {
        out << "\t\t" <<  vertNorms[j].X() << " " << vertNorms[j].Y() << " " << vertNorms[j].Z() << " " << endl;
    }
    out << "\t]" << endl; // End Property

    // Write the Face Indices in order
    out << "\n\t\"integer indices\"" << endl
        << "\t[" << endl;
   for (int k = 0; k < nbTriInFace; k++) {
        out << "\t\t" <<  cons[3 * k] << " " << cons[3 * k + 2] << " " << cons[3 * k + 1] << endl;
    }
    out << "\t]" << endl; // End Property

    // End of Face
    out << "AttributeEnd\n" << endl;

    delete [] vertNorms;
    delete [] verts;
    delete [] cons;

    return outStr;
}

QString LuxRender::genMaterial(RenderMaterial *mat)
{
    // Texture "checks" "color" "checkerboard"
    //         "float uscale" [4] "float vscale" [4]
    //         "color tex1" [1 0 0] "color tex2" [0 0 1]
    //
    // Material "matte"
    //         "texture Kd" "checks"
    // Texture "SolidColor" "color" "constant" "color value" [1.000 0.910 0.518]

    QString outStr;
    QTextStream out(&outStr);

    if(mat->getMaterial()->source == LibraryMaterial::BUILTIN) {
        out << "Material \"" << mat->getMaterial()->compat << "\"" << endl;
        QMap<QString, MaterialProperty *>::const_iterator it = mat->Properties.constBegin();
        while (it != mat->Properties.constEnd()) {
          if(!it.value())
            continue; //No key perhaps throw error

          switch(it.value()->getType()) {
            case MaterialParameter::BOOL: break;
            case MaterialParameter::FLOAT: {
              MaterialFloatProperty *prop = static_cast<MaterialFloatProperty *>(it.value());
              out << "\"float " << it.key() << "\" [" << prop->getValue() <<  "]"; } break;
            case MaterialParameter::COLOR: {
              MaterialColorProperty *prop = static_cast<MaterialColorProperty *>(it.value());
              const float *color = prop->getValue();
              if(!color)
                continue;
              out << "\"color " << it.key() << "\" [" << color[0] << " " << color[1] << " " << color[2] <<  "]" << endl;
            } break;
            default: break;
          }
          ++it;
        }
        out << endl;
    } else {
        //Open the filename and append
        QFile file(mat->getMaterial()->filename);
        if(!file.open(QFile::ReadOnly))
            return QString();

        QTextStream textStr(&file);
        while(!textStr.atEnd())
        {
          out << textStr.readLine() << endl;
        }

        out << "\nNamedMaterial \"" << mat->getMaterial()->compat << "\"" << endl;
    }

    return outStr;
}

QString LuxRender::genObject(RenderPart *obj)
{
    //fMeshDeviation is a class variable
    Base::Console().Log("Meshing with Deviation: %f\n", obj->getMeshDeviation());

    const TopoDS_Shape &Shape = obj->getShape();
    TopExp_Explorer ex;
    BRepMesh_IncrementalMesh MESH(Shape,obj->getMeshDeviation());

    const char * name = obj->getName();
    // counting faces and start sequencer
    int l = 1;

    QString outStr;
    QTextStream out(&outStr);

    out << "\nObjectBegin \"" << name << "\"" << endl;

    // Generate the material if there is one.
    std::vector<RenderMaterial *> partMaterials = this->getRenderPartMaterials(obj);

    // Look for any whole feature materials

    if(partMaterials.size() > 0) {
        for(std::vector<RenderMaterial *>::const_iterator it = partMaterials.begin(); it != partMaterials.end(); ++it) {
            getRenderMaterialLink(*it);
            out << genMaterial(*it);
//             std::vector<std::string> subs = (*it)->Link.getSubValues();
//             if(subs.size() == 0) {
//                 out << genMaterial(*it);
//             }
        }
    }

    //Generate each face
    for (ex.Init(Shape, TopAbs_FACE); ex.More(); ex.Next(),l++) { 
        const TopoDS_Face& aFace = TopoDS::Face(ex.Current());
        out << genFace(aFace, l);
    }
    out << "ObjectEnd" << endl
        << "ObjectInstance \"" << name << "\"" << endl;
    return outStr;
}

// Renderer "sampler"
//
// Sampler "metropolis"
//         "float largemutationprob" [0.400000005960464]
//         "bool usevariance" ["false"]
//
// Accelerator "qbvh"
//
// SurfaceIntegrator "bidirectional"
//         "integer eyedepth" [48]
//         "integer lightdepth" [48]
//
// VolumeIntegrator "multi"
//         "float stepsize" [1.000000000000000]
//
// PixelFilter "mitchell"
//         "bool supersample" ["true"]

QString LuxRender::genRenderProps()
{

    QString outStr;
    QTextStream out(&outStr);

    if(!preset)
      return outStr; // Throw exception?

    out << "\n# Scene render Properties:" << endl;

    //Open the filename and append
    QFile file(preset->getFilename());
    if(!file.open(QFile::ReadOnly))
        return outStr;

    // Read the external preset file
    QTextStream textStr(&file);
    while(!textStr.atEnd())
    {
        out << textStr.readAll();
    }

    out << "\n\nFilm \"fleximage\" \"integer xresolution\" [" << this->xRes << "] \"integer yresolution\" [" << yRes << "]" << endl;
    out << "\n\t\"integer writeinterval\" 3" << endl;
    return outStr;
}

QString LuxRender::genRenderTemplate()
{
    QString outStr;
    QTextStream out(&outStr);

    if(!renderTemplate)
      return outStr; // Throw exception?

    Base::Vector3d camPos = getCamera()->CamPos;
    float camLength = getCamera()->CamPos.Length();
    camLength *= 2;

    Base::Vector3d delta = (bbMax - bbMin) ; 
    // Check if the bounding box was set for the scene
    if (delta.Length() < FLT_EPSILON) {
        return outStr; // Throw exception MUST BE SET?
    }

    // Get Bounding box centered
    Base::Vector3d center = (bbMax + bbMin) / 2;  // Translate the middle of the scene to center of bounding box
    float planePos = 0; // Set the z-value for the plane
    // Find largest delta length
    float bbBoxsize = (delta.y > delta.x) ? ((delta.z > delta.y) ? delta.z : delta.y ): (delta.z > delta.x) ? delta.z : delta.x;
    
    float scale = (camLength > bbBoxsize) ? camLength : bbBoxsize;
    scale *= exp(ceil(log(std::abs(scale)))) * 1.2;
    out << "\n# Scene Template" << endl
        << "TransformBegin" << endl
        << "Translate " << center.x << " " << center.y << " " << planePos << endl
        << "Scale " << scale << " " << scale << " " << scale << endl;

    if(renderTemplate->getSource() == RenderTemplate::EXTERNAL)
    {
        //Open the filename and append
        QFile file(renderTemplate->getFilename());
        if(!file.open(QFile::ReadOnly))
            return outStr;

        // Read the external template file
        QTextStream textStr(&file);
        while(!textStr.atEnd())
        {
            out << textStr.readAll();
        }
    }

    out << "\n\nTransformEnd" << endl;
    return outStr;
}

void LuxRender::initRender(RenderMode renderMode)
{
    // Check if there are any processes running;
    if(process && process->isActive())
        return;

    // Create a new Render Process
    LuxRenderProcess * process = new LuxRenderProcess();
    this->attachRenderProcess(process);
    Renderer::initRender(renderMode);
}