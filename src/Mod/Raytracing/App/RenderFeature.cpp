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

#include <string>
#include <Base/Writer.h>
#include <Base/Reader.h>
#include <Base/Tools.h>
#include <Base/Console.h>

// Each Render Plugin listed below
#include "renderer/lux/LuxRender.h"

#include "Renderer.h"
#include "RenderProcess.h"

#include "RenderFeature.h"
#include "RenderFeaturePy.h"

#include "Appearances.h"

// Exists for testing purposes
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <App/Material.h>
#include <Mod/Part/App/PartFeature.h>

using namespace Raytracing;
using namespace Base;

const char* RenderFeature::TypeEnums[]= {"Lux","Povray",NULL};

PROPERTY_SOURCE(Raytracing::RenderFeature, App::DocumentObject)

RenderFeature::RenderFeature()
{
    ADD_PROPERTY(RendererType,((long)0));
    ADD_PROPERTY(Preset,(""));
    ADD_PROPERTY(SceneTemplate,(""));
    ADD_PROPERTY(OutputX,(800));
    ADD_PROPERTY(OutputY,(800));
    ADD_PROPERTY(UpdateInterval,(3000));
    ADD_PROPERTY_TYPE(ExternalGeometry,(0,0),"RenderFeature",(App::PropertyType)(App::Prop_None),"External geometry");
    ADD_PROPERTY_TYPE(MaterialsList,     (0)  ,"RenderFeature",(App::PropertyType)(App::Prop_None),"Render materials");
    RendererType.setEnums(TypeEnums);
    
    renderer = 0;

//     ADD_PROPERTY_TYPE(Constraints,     (0)  ,"Sketch",(App::PropertyType)(App::Prop_None),"Sketch constraints");
//     ADD_PROPERTY_TYPE(ExternalGeometry,(0,0),"Sketch",(App::PropertyType)(App::Prop_None),"Sketch external geometry");
}

RenderFeature::~RenderFeature()
{
    removeRenderer();
}

void RenderFeature::attachRenderCamera(RenderCamera *cam)
{
  if(!renderer)
    return;

  if(renderer->hasCamera())
      Base::Console().Error("A camera is already set\n");
  renderer->addCamera(cam);
}

void RenderFeature::setRenderer(const char *rendererType)
{
    // Clear the previous Renderer
    this->removeRenderer();

    if(strcmp("Lux", rendererType) == 0) {
        LuxRender *render = new LuxRender();
        this->renderer = render;
    } else if(strcmp("Povray", rendererType) == 0) {
            Base::Console().Log("Currently not implemented\n");
    }

    // Reload the materials
    // Appearances().scanMaterials();
}

void RenderFeature::removeRenderer(void)
{
    if(!hasRenderer())
        return;

    // Determine the type to ensure the correct renderer destructor is called
    if(this->renderer->getTypeId() == LuxRender::getClassTypeId())
    {
        LuxRender *myRenderer = static_cast<LuxRender *>(renderer);
        delete myRenderer;
    }
    this->renderer = 0; // Make it a null pointer
}

// TODO later this should be converted into a LinkProperty so it can have subvalues
App::DocumentObject * RenderFeature::getRenderMaterialLink(RenderMaterial *material)
{
    int linkId = material->LinkIndex.getValue();
    assert(linkId < 0);

    std::vector<DocumentObject*> Objects     = ExternalGeometry.getValues();
    std::vector<std::string>     SubElements = ExternalGeometry.getSubValues();

    const std::vector<DocumentObject*> originalObjects = Objects;
    const std::vector<std::string>     originalSubElements = SubElements;

    const std::vector< RenderMaterial * > &vals = this->MaterialsList.getValues();

    int i = 0;
    //TODO make this for individual faces
    for (std::vector<DocumentObject *>::const_iterator it= originalObjects.begin();it!= originalObjects.end();++it, i++) {
        if(linkId == i)
            return (*it);
    }
    return 0;
}

int RenderFeature::setRenderMaterial(const RenderMaterial *material)
{
    int idx = material->LinkIndex.getValue();
    assert(idx >= 0);

    const std::vector< RenderMaterial * > &vals = this->MaterialsList.getValues();
    RenderMaterial *matNew = material->clone();

    std::vector< RenderMaterial * > newVals(vals);
    newVals[idx] = matNew;
    this->MaterialsList.setValues(newVals);

    delete matNew;
    // We don't have to alter the MaterialsLink fields since we are setting
    return idx;
}

int RenderFeature::addRenderMaterial(const RenderMaterial *material, DocumentObject *pcObj)
{
    // Find the actual link for the Render Material
    // get the actual lists of the externals
    std::vector<DocumentObject*> Objects     = ExternalGeometry.getValues();
    std::vector<std::string>     SubElements = ExternalGeometry.getSubValues();
    const std::vector< RenderMaterial * > &vals = this->MaterialsList.getValues();

    if(!pcObj) {
        Base::Console().Warning("The part could not be found");
        return -1;
    }

    // Currently only can add to whole objects - add a new material and get its index to store
    int matIndex = addMatLink(pcObj, "");

    std::vector< RenderMaterial * > newVals(vals);
    RenderMaterial *matNew = material->clone();

    // Assign the material index
    matNew->LinkIndex.setValue(matIndex);

    newVals.push_back(matNew);
    this->MaterialsList.setValues(newVals);

    std::stringstream stream;
    stream << "Attached Material " << matNew->getMaterial()->label.toStdString()
           << " on object "        << pcObj->getNameInDocument() << "\n";

    Base::Console().Log(stream.str().c_str());
    delete matNew;
    return this->MaterialsList.getSize()-1;
}

// TODO Later need to give it a subvalues
int RenderFeature::removeRenderMaterialFromPart(const char *partName)
{
    std::vector<DocumentObject*> Objects     = ExternalGeometry.getValues();
    std::vector<std::string>     SubElements = ExternalGeometry.getSubValues();

    const std::vector<DocumentObject*> originalObjects = Objects;
    const std::vector<std::string>     originalSubElements = SubElements;

    const std::vector< RenderMaterial * > &vals = this->MaterialsList.getValues();

    int i = 0, idx = -1;
    //TODO make this for individual faces
    for (std::vector<DocumentObject *>::const_iterator it= originalObjects.begin();it!= originalObjects.end();++it, i++) {
        if(strcmp((*it)->getNameInDocument(), partName) == 0)
            idx = i;
    }

    if(idx >= 0) {
        //Index Found
        //Search through the rendermaterials for any matching to idx
        int matIndex = -1;

        int i = 0;
        for (std::vector<RenderMaterial *>::const_iterator it= vals.begin();it!=vals.end();++it, i++) {
            if((*it)->LinkIndex.getValue() == idx)
                matIndex = i;
        }

        // Currently one material can be assigned to each object (logical)
        if(matIndex >= 0) {
            delMatLink(idx);
            std::vector< RenderMaterial * > newVals(vals);
            newVals.erase(newVals.begin() + matIndex);
            this->MaterialsList.setValues(newVals);
            return 1;
        }
    }
    return 0;
}

// TODO Later need to give it a subvalues
const RenderMaterial * RenderFeature::getRenderMaterial(const char *partName) const
{
    std::vector<DocumentObject*> Objects     = ExternalGeometry.getValues();
    std::vector<std::string>     SubElements = ExternalGeometry.getSubValues();

    const std::vector<DocumentObject*> originalObjects = Objects;
    const std::vector<std::string>     originalSubElements = SubElements;
    
    const std::vector< RenderMaterial * > &vals = this->MaterialsList.getValues();

    int i = 0, idx = -1;
    //TODO make this for individual faces
    for (std::vector<DocumentObject *>::const_iterator it= originalObjects.begin();it!= originalObjects.end();++it, i++) {
        if(strcmp((*it)->getNameInDocument(), partName) == 0)
            idx = i;
    }

    if(idx >= 0) {
        //Index Found
         // Currently one material can be assigned to each object (logical)
        //Search through the rendermaterials for any matching to idx
        int matIndex = -1;

        for (std::vector<RenderMaterial *>::const_iterator it= vals.begin();it!=vals.end();++it, i++) {
            if((*it)->LinkIndex.getValue() == idx)
                return (*it);
        }
    }
    return 0;
}

RenderProcess * RenderFeature::getActiveRenderProcess() const
{
    if(!hasRenderer())
        return 0;

    if(!renderer->getRenderProcess() || !renderer->getRenderProcess()->isActive())  {
//         Base::Console().Error("Render Process is not available\n");
        return 0;
    }
    return renderer->getRenderProcess();
}

/// Methods
void RenderFeature::setCamera(const Base::Vector3d &v1, const Base::Vector3d &v2, const Base::Vector3d &v3, const Base::Vector3d &v4, const char *camType)
{
    if(!hasRenderer())
        return;

    if(!renderer->hasCamera()) {
        Base::Console().Error("Renderer doesn't have a camera set\n");
        return;
    }

    renderer->getCamera()->setType(camType);
    renderer->setCamera(v1, v2, v3, v4);
}

void RenderFeature::setCamera(RenderCamera *cam)
{
    // Delete the previous camera
    RenderCamera *originalCamera = renderer->getCamera();
    *originalCamera = *cam;
}

bool RenderFeature::hasRenderer() const
{
    if(!renderer) {
        Base::Console().Error("Renderer is not available\n");
        return false;
    }

    return true;
}


void RenderFeature::setBBox(const Base::Vector3d &min, const Base::Vector3d &max)
{
    if(!hasRenderer())
        return;

    renderer->setBBox(min, max);
}

RenderCamera * RenderFeature::getCamera(void)
{
    if(!hasRenderer())
        return 0;

    return renderer->getCamera();
}

void RenderFeature::preview(int x1, int y1, int x2, int y2)
{
    if(!isRendererReady())
        return;

    // Make a copy of the materials

    PropertyRenderMaterialList *matListCopy = static_cast<PropertyRenderMaterialList *> (MaterialsList.Copy());
    // Argument Variables are temporary
    renderer->setRenderTemplate(SceneTemplate.getValue());
    renderer->attachRenderMaterials(matListCopy->getValues(), ExternalGeometry.getValues());
    renderer->setUpdateInteval(UpdateInterval.getValue());
    renderer->setRenderPreset(Preset.getValue());
    renderer->setRenderSize(OutputX.getValue(), OutputY.getValue());
    renderer->preview(x1, y1, x2, y2);
}

void RenderFeature::preview()
{
    if(!isRendererReady())
      return;

    // Adding stuff for testing
//     RenderAreaLight *light = new RenderAreaLight();
//     light->setColor(255, 255, 255);
//     light->Height = 100;
//     light->Width = 100;
// 
//     Base::Rotation lightRot = Base::Rotation(Base::Vector3d(0, 1, 0), 0.);
//     Base::Vector3d lightPos = Base::Vector3d(-50., -50., 200);
//     light->setPlacement(lightPos, lightRot);
// 
//     renderer->addLight(light);

    renderer->setRenderTemplate(SceneTemplate.getValue());
    renderer->attachRenderMaterials(MaterialsList.getValues(), ExternalGeometry.getValues());
    renderer->setRenderPreset(Preset.getValue());
    renderer->setUpdateInteval(UpdateInterval.getValue());
    renderer->setRenderSize(OutputX.getValue(), OutputY.getValue());
    renderer->preview();
}

int RenderFeature::addMatLink(App::DocumentObject *Obj, const char* SubName)
{
    // get the actual lists of the externals
    std::vector<DocumentObject*> Objects     = ExternalGeometry.getValues();
    std::vector<std::string>     SubElements = ExternalGeometry.getSubValues();

   // TODO not sure if we should have unique indexes for each Render Material
    // add the new ones
    Objects.push_back(Obj);
    SubElements.push_back(std::string(SubName));

    // set the Link list.
    ExternalGeometry.setValues(Objects,SubElements);

    return ExternalGeometry.getValues().size()-1;
}

int RenderFeature::delMatLink(int linkId)
{
    // get the actual lists of the externals
    std::vector<DocumentObject*> Objects     = ExternalGeometry.getValues();
    std::vector<std::string>     SubElements = ExternalGeometry.getSubValues();

    if (linkId < 0 || linkId >= int(SubElements.size()))
        return -1;

    const std::vector<DocumentObject*> originalObjects = Objects;
    const std::vector<std::string>     originalSubElements = SubElements;

    Objects.erase(Objects.begin()+linkId);
    SubElements.erase(SubElements.begin()+linkId);

    const std::vector< RenderMaterial* > &renderMaterials = MaterialsList.getValues();
    std::vector< RenderMaterial * > newRenderMaterials(0);

    for (std::vector<RenderMaterial *>::const_iterator it = renderMaterials.begin(); it != renderMaterials.end(); ++it) {
        RenderMaterial *cloneMat = (*it)->clone();
        int idx = cloneMat->LinkIndex.getValue();
        if(idx > linkId)
        cloneMat->LinkIndex.setValue(--idx);
        newRenderMaterials.push_back(cloneMat);
    }

    ExternalGeometry.setValues(Objects,SubElements);
    MaterialsList.setValues(newRenderMaterials);
    return 0;
}


void RenderFeature::render()
{
    if(!isRendererReady())
        return;

    renderer->attachRenderMaterials(MaterialsList.getValues(), ExternalGeometry.getValues());
    renderer->setRenderTemplate(SceneTemplate.getValue());
    renderer->setRenderPreset(Preset.getValue());
    renderer->setUpdateInteval(UpdateInterval.getValue());
    renderer->setRenderSize(OutputX.getValue(), OutputY.getValue());
    renderer->render();
}

void RenderFeature::finish()
{
    if(!hasRenderer())
        return;

    RenderProcess *process = renderer->getRenderProcess();
    if(!process || !process->isActive())
        Base::Console().Error("Renderer process doesn't exist or isn't active\n");

    renderer->finish();
}

void RenderFeature::reset()
{
    if(!hasRenderer())
        return;

    renderer->reset();
}

bool RenderFeature::isRendererReady() const
{
    if(!hasRenderer())
        return false;

    if(!this->renderer->hasCamera()) {
        Base::Console().Error("Camera has not been set for the render\n");
        return false;
    }

    return true;
}

void RenderFeature::setRenderSize(int x, int y)
{
    if(x < 0 || y < 0)
          Base::Console().Error("Render size values are incorrect\n");

    OutputX.setValue(x);
    OutputY.setValue(y);
}

void RenderFeature::setRenderPreset(const char * presetName)
{
    if(!hasRenderer())
        return;

    // Check if the Render Preset exists for the given render backend
    RenderPreset *fndPreset = renderer->getRenderPreset(presetName);
    if(!fndPreset)
        Base::Console().Error("The Render Preset couldn't be found\n");

    // Finally Set the Render Preset
    Preset.setValue(presetName);
}

void RenderFeature::setRenderTemplate(const char * templateName)
{
    if(!hasRenderer())
        return;

    // Check if the Render Preset exists for the given render backend
    RenderTemplate *fndTemplate = renderer->getRenderTemplate(templateName);
    if(!fndTemplate)
        Base::Console().Error("The Render Template couldn't be found\n");

    // Finally Set the Render Preset
    SceneTemplate.setValue(templateName);
}

void RenderFeature::setOutputPath(const char * outputPath)
{
    if(!hasRenderer())
        return;

    renderer->setOutputPath(outputPath);
}

void RenderFeature::setUpdateInterval(int interval)
{
  this->UpdateInterval.setValue(interval);
}

App::DocumentObjectExecReturn *RenderFeature::execute(void)
{

    return App::DocumentObject::StdReturn;
}

PyObject *RenderFeature::getPyObject(void)
{
    if (PythonObject.is(Py::_None())) {
        // ref counter is set to 1
        PythonObject = Py::Object(new RenderFeaturePy(this),true);
    }
    return Py::new_reference_to(PythonObject);
}

unsigned int RenderFeature::getMemSize(void) const
{
    return 0;
}

void RenderFeature::Save(Writer &writer) const
{
    App::DocumentObject::Save(writer);
    renderer->getCamera()->Save(writer);
}

void RenderFeature::Restore(XMLReader &reader)
{
    App::DocumentObject::Restore(reader);

    RenderCamera *cam = new RenderCamera();
    cam->Restore(reader);

    renderer->addCamera(cam);
}

void RenderFeature::onChanged(const App::Property* prop)
{
    Base::Console().Log("On Changed Called");
    if (prop == &RendererType) {
        setRenderer(RendererType.getValueAsString()); // Reload the Render Plugin
    }
}

void RenderFeature::onDocumentRestored()
{

}

void RenderFeature::onFinishDuplicating()
{
}


// Python RenderFeature ---------------------------------------------------------

namespace App {
/// @cond DOXERR
PROPERTY_SOURCE_TEMPLATE(Raytracing::RenderFeaturePython, Raytracing::RenderFeature)
template<> const char* Raytracing::RenderFeaturePython::getViewProviderName(void) const {
    return "RaytracingGui::ViewProviderPython";
}
/// @endcond

// explicit template instantiation
template class RaytracingExport FeaturePythonT<Raytracing::RenderFeature>;
}
