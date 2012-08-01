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
# include <BRepMesh_IncrementalMesh.hxx>
# include <GeomAPI_ProjectPointOnSurf.hxx>
# include <GeomLProp_SLProps.hxx>
# include <Poly_Triangulation.hxx>
# include <TopExp_Explorer.hxx>
# include <sstream>
# include <QString>
# include <QFile>
# include <QDir>
# include <QXmlStreamReader>
# include <TopoDS.hxx>
# include <TopoDS_Face.hxx>
#endif

#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Sequencer.h>
#include <App/ComplexGeoData.h>

#include <Base/Writer.h>
#include <Base/Reader.h>
#include "Renderer.h"

using namespace Raytracing;

TYPESYSTEM_SOURCE_ABSTRACT(Raytracing::Renderer, Base::BaseClass)

Renderer::Renderer(void)
{
    this->process = 0;
    this->camera  = 0;
    this->xRes = 0;
    this->yRes = 0;

}
Renderer::~Renderer(void)
{
    this->clear();
}

void Renderer::clear()
{
    delete this->camera;
    this->camera = 0;

    delete this->process;
    this->process = 0;
    for (std::vector<RenderLight *>::iterator it = lights.begin(); it != lights.end(); ++it) {
      delete *it;
    }
    for (std::vector<RenderPart *>::iterator it = parts.begin(); it != parts.end(); ++it) {
      delete *it;
    }
    clearPresets();
    lights.clear();
    parts.clear();
}

void Renderer::addCamera(RenderCamera *cam) {
    this->camera = cam;
}

void Renderer::addLight(RenderLight *light) {
  this->lights.push_back(light);
}


void Renderer::addObject( RenderPart *part) {
  this->parts.push_back(part);
}

void Renderer::addObject(const char *PartName, const TopoDS_Shape &Shape, float meshDeviation)
{
    RenderPart *part = new RenderPart(PartName, Shape, meshDeviation);
    addObject(part);
}

void Renderer::attachRenderProcess(RenderProcess *Process)
{
  this->process = Process; // The baseclass will have to determine the process type
}

bool Renderer::getOutputStream(QTextStream &ts)
{
    //Check if temporary file can be open
    if(!inputFile.open())
        return false;

    ts.setDevice(&inputFile);
}

void Renderer::clearPresets(void)
{
    // For safety reset the render preset to null pointer
    preset = 0;

    for (std::vector<RenderPreset *>::iterator it = libraryPresets.begin(); it != libraryPresets.end(); ++it) {
      delete *it;
    }
    libraryPresets.empty();
}

std::vector<RenderPreset *> Renderer::parsePresetXML(QString filename)
{
    std::vector<RenderPreset *> fndPresets;
    // Load the XML File
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
      return fndPresets; // The file couldn't be read

    QFileInfo fileInfo(file);
    QDir fileDir = fileInfo.absoluteDir();

    QXmlStreamReader xml;
    // Set the xml stream to use the file
    xml.setDevice(&file);

    QString provider;
    while (xml.readNextStartElement()) {
        if (xml.name() == "Presets") {
            provider = xml.attributes().value(QString::fromAscii("render")).toString();
        } else if (xml.name() == "Preset") {
            // Process this Preset Attribute
            QString id, label, filename, description;
            id = xml.attributes().value(QString::fromAscii("id")).toString(); // possibly should be hased to ensure unique
            do {
                if (xml.name() == "label")
                    label = xml.readElementText();
                else if (xml.name() == "description")
                    description = xml.readElementText();
                else if (xml.name() == "filename")
                    filename = xml.readElementText();
            } while(xml.readNextStartElement());

            if(id.isEmpty() || label.isEmpty() || filename.isEmpty() ||
               description.isEmpty() || provider.isEmpty())
                continue;

            // Make the filename absolute
            filename = fileDir.absoluteFilePath(filename);
            RenderPreset *preset = new RenderPreset(id, label, filename, description, provider);

            // Append this preset
            fndPresets.push_back(preset);
        }
    }

    if (xml.hasError()) {
    //           ... // do error handling
    }
    file.close();

    return fndPresets;

}

void Renderer::setRenderPreset(const char *presetId)
{
    RenderPreset *fndPreset = getRenderPreset(presetId);
    if(!fndPreset)
      return; // Thrown an exception?

    preset = fndPreset;
}

void Renderer::scanPresets(void)
{
  // Clear the materials - since the user may want to refresh the libraries
    clearPresets();

    // Note this is not recursive, it will only find XML files one level down.
    // List of all XML files with absolute path name
    QStringList parseList;

      //Find a list of xml files within the
    QDir presetDir(QString::fromStdString(renderPresetsPath));

    QStringList fileExt;
    fileExt.push_back(QString::fromAscii("*.xml"));

    // Looks for xml files in directory
    QStringList xmlFiles = presetDir.entryList(fileExt);
    for (QStringList::const_iterator file=xmlFiles.begin(); file!=xmlFiles.end(); ++file){
        parseList.append(presetDir.absoluteFilePath(*file));
    }

    // Need to add parselist from  own library
    if(parseList.size() == 0)
      return;
    // Process all these xml file into Materials

        // Iterate through
    for (QStringList::const_iterator it=parseList.begin(); it!=parseList.end(); ++it)
    {
        //Parse the XML
        std::vector<RenderPreset *> fndPresets = parsePresetXML(*it);
        if(fndPresets.size() > 0)
        {
            //If successful store these materials in the collection
            for ( std::vector<RenderPreset *>::const_iterator preset = fndPresets.begin(); preset != fndPresets.end(); ++preset)
                libraryPresets.push_back(*preset);
        } else {
          // Throw an exception?
        }
    }
}

RenderPreset * Renderer::getRenderPreset(const char *id) const
{
    for (std::vector<RenderPreset *>::const_iterator it = libraryPresets.begin(); it != libraryPresets.end(); ++it) {
        if((*it)->getId() == QString::fromAscii(id))
            return *it;
    }
}

std::vector<RenderPreset *> Renderer::getRenderPresets(void) const
{
  return libraryPresets;
}

void Renderer::finish()
{
  if(!process || !process->isActive())
      return; // The process cannot be stopped because it's not active

  this->process->stop();
}

void Renderer::preview(int x1, int y1, int x2, int y2)
{
  reset();

  previewCoords[0] = x1;
  previewCoords[1] = y1;
  previewCoords[2] = x2;
  previewCoords[3] = y2;

  QTemporaryFile tempFile;
  tempFile.open();
  QString tmpFileName = tempFile.fileName();

  //An Extension is usually required.
  QString extension = QString::fromAscii(".png");
  tmpFileName.append(extension);

  outputPath.clear();
  outputPath = tmpFileName.toStdString();

  initRender(PREVIEW_AREA);
}

void Renderer::preview()
{
    reset();
    // Create a temporary file to store the render preview
    QTemporaryFile tempFile;
    tempFile.open();
    QString tmpFileName = tempFile.fileName();

    //An Extension is usually required.
    QString extension = QString::fromAscii(".png");
    tmpFileName.append(extension);

    outputPath.clear();
    outputPath = tmpFileName.toStdString();

    this->initRender(PREVIEW);
}

void Renderer::render()
{
    reset();
    this->initRender(RENDER);
}

void Renderer::reset()
{
    //Clear up any previously stored data associated with a render process or preview
     // Clear the previous contents and regenerate
    previewCoords[0] = -1;
    previewCoords[1] = -1;
    previewCoords[2] = -1;
    previewCoords[3] = -1;
}

void Renderer::initRender(RenderMode renderMode)
{
    if(this->process && this->process->isActive())
      return;

    if(!inputFile.open())
      return;
    inputFile.resize(0);

    mode = renderMode;

    generateScene();
    process->setInputPath(inputFile.fileName());

    QString filepath = QString::fromStdString(outputPath);
    // Create the output file
    QFile file(filepath);

    if(!file.open(QIODevice::ReadWrite))
      return; // Throw an exception output file couldn't be created
    file.close();
    process->setOutputPath(filepath);

    // Initialise the Process
    process->initialiseSettings();
    process->begin();
}

void Renderer::setCamera(const Base::Vector3d &camPos, const Base::Vector3d &camDir, const Base::Vector3d &lookAt, const Base::Vector3d &up) {
  if(!camera)
    return;

    camera->CamPos = camPos;
    camera->CamDir = camDir;
    camera->LookAt = lookAt;
    camera->Up     = up;
}

// void Renderer::loadSceneDefinition(const char *file) {}
// void Renderer::loadXML(Base::XMLReader &/*reader*/) {}

void Renderer::transferToArray(const TopoDS_Face& aFace,gp_Vec** vertices,gp_Vec** vertexnormals, long** cons,int &nbNodesInFace,int &nbTriInFace ) {
    TopLoc_Location aLoc;

    // doing the meshing and checking the result
    //BRepMesh_IncrementalMesh MESH(aFace,fDeflection);
    Handle(Poly_Triangulation) aPoly = BRep_Tool::Triangulation(aFace,aLoc);
    if (aPoly.IsNull()) {
        Base::Console().Log("Empty face trianglutaion\n");
        nbNodesInFace =0;
        nbTriInFace = 0;
        vertices = 0l;
        cons = 0l;
        return;
    }

    // geting the transformation of the shape/face
    gp_Trsf myTransf;
    Standard_Boolean identity = true;
    if (!aLoc.IsIdentity())  {
        identity = false;
        myTransf = aLoc.Transformation();
    }

    Standard_Integer i;
    // geting size and create the array
    nbNodesInFace = aPoly->NbNodes();
    nbTriInFace = aPoly->NbTriangles();
    *vertices = new gp_Vec[nbNodesInFace];
    *vertexnormals = new gp_Vec[nbNodesInFace];
    for (i=0; i < nbNodesInFace; i++) {
        (*vertexnormals)[i]= gp_Vec(0.0,0.0,0.0);
    }

    *cons = new long[3*(nbTriInFace)+1];

    // check orientation
    TopAbs_Orientation orient = aFace.Orientation();

    // cycling through the poly mesh
    const Poly_Array1OfTriangle& Triangles = aPoly->Triangles();
    const TColgp_Array1OfPnt& Nodes = aPoly->Nodes();
    for (i=1; i<=nbTriInFace; i++) {
        // Get the triangle
        Standard_Integer N1,N2,N3;
        Triangles(i).Get(N1,N2,N3);

        // change orientation of the triangles
        if ( orient != TopAbs_FORWARD )
        {
            Standard_Integer tmp = N1;
            N1 = N2;
            N2 = tmp;
        }

        gp_Pnt V1 = Nodes(N1);
        gp_Pnt V2 = Nodes(N2);
        gp_Pnt V3 = Nodes(N3);

        // transform the vertices to the place of the face
        if (!identity) {
            V1.Transform(myTransf);
            V2.Transform(myTransf);
            V3.Transform(myTransf);
        }

        // Calculate triangle normal
        gp_Vec v1(V1.X(),V1.Y(),V1.Z()),v2(V2.X(),V2.Y(),V2.Z()),v3(V3.X(),V3.Y(),V3.Z());
        gp_Vec Normal = (v2-v1)^(v3-v1);

        //Standard_Real Area = 0.5 * Normal.Magnitude();

        // add the triangle normal to the vertex normal for all points of this triangle
        (*vertexnormals)[N1-1] += gp_Vec(Normal.X(),Normal.Y(),Normal.Z());
        (*vertexnormals)[N2-1] += gp_Vec(Normal.X(),Normal.Y(),Normal.Z());
        (*vertexnormals)[N3-1] += gp_Vec(Normal.X(),Normal.Y(),Normal.Z());

        (*vertices)[N1-1].SetX((float)(V1.X()));
        (*vertices)[N1-1].SetY((float)(V1.Y()));
        (*vertices)[N1-1].SetZ((float)(V1.Z()));
        (*vertices)[N2-1].SetX((float)(V2.X()));
        (*vertices)[N2-1].SetY((float)(V2.Y()));
        (*vertices)[N2-1].SetZ((float)(V2.Z()));
        (*vertices)[N3-1].SetX((float)(V3.X()));
        (*vertices)[N3-1].SetY((float)(V3.Y()));
        (*vertices)[N3-1].SetZ((float)(V3.Z()));

        int j = i - 1;
        N1--;
        N2--;
        N3--;
        (*cons)[3*j] = N1;
        (*cons)[3*j+1] = N2;
        (*cons)[3*j+2] = N3;
    }

    // normalize all vertex normals
    for (i=0; i < nbNodesInFace; i++) {

        gp_Dir clNormal;

        try {
            Handle_Geom_Surface Surface = BRep_Tool::Surface(aFace);

            gp_Pnt vertex((*vertices)[i].XYZ());
//     gp_Pnt vertex((*vertices)[i][0], (*vertices)[i][1], (*vertices)[i][2]);
            GeomAPI_ProjectPointOnSurf ProPntSrf(vertex, Surface);
            Standard_Real fU, fV;
            ProPntSrf.Parameters(1, fU, fV);

            GeomLProp_SLProps clPropOfFace(Surface, fU, fV, 2, gp::Resolution());

            clNormal = clPropOfFace.Normal();
            gp_Vec temp = clNormal;
            if ( temp * (*vertexnormals)[i] < 0 )
                temp = -temp;
            (*vertexnormals)[i] = temp;

        }
        catch (...) {
        }

        (*vertexnormals)[i].Normalize();
    }
}
