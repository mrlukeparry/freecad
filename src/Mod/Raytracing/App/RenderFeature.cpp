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

int RenderFeature::setRenderMaterial(const RenderMaterial *material)
{
    const std::vector< RenderMaterial * > &vals = this->MaterialsList.getValues();

    RenderMaterial *matNew = material->clone();
    const char *partName = material->Link.getValue()->getNameInDocument();

    int i = 0, idx = -1;
    //TODO make this for individual faces
    for (std::vector<RenderMaterial*>::const_iterator it=vals.begin();it!=vals.end();++it, i++) {
        if(strcmp((*it)->Link.getValue()->getNameInDocument(), partName) == 0)
            idx = i;
    }

    if(idx >= 0) {
        //Material Found
        std::vector< RenderMaterial * > newVals(vals);
        newVals[idx] = matNew;
        this->MaterialsList.setValues(newVals);
        return idx;
    } else {
      return 0;
    }

    delete matNew;
}

int RenderFeature::addRenderMaterial(const RenderMaterial *material)
{
    const std::vector< RenderMaterial * > &vals = this->MaterialsList.getValues();

    std::vector< RenderMaterial * > newVals(vals);
    RenderMaterial *matNew = material->clone();
    newVals.push_back(matNew);
    this->MaterialsList.setValues(newVals);

    std::stringstream stream;
    stream << "Attached Material " << material->getMaterial()->label.toStdString()
           << " on object "        << material->Link.getValue()->getNameInDocument() << "\n";

    Base::Console().Log(stream.str().c_str());
    delete matNew;
    return this->MaterialsList.getSize()-1;
}

// TODO Later need to give it a subvalues
int RenderFeature::removeRenderMaterialFromPart(const char *partName)
{
    const std::vector< RenderMaterial * > &vals = this->MaterialsList.getValues();

    int i = 0, idx = -1;
    //TODO make this for individual faces
    for (std::vector<RenderMaterial*>::const_iterator it=vals.begin();it!=vals.end();++it, i++) {
        if(strcmp((*it)->Link.getValue()->getNameInDocument(), partName) == 0)
            idx = i;
    }

    if(idx >= 0) {
        //Material Found
        std::vector< RenderMaterial * > newVals(vals);
        newVals.erase(newVals.begin() + idx);
        this->MaterialsList.setValues(newVals);
    }
    return 0;
}

// TODO Later need to give it a subvalues
const RenderMaterial * RenderFeature::getRenderMaterial(const char *partName) const
{
    const std::vector< RenderMaterial * > &vals = this->MaterialsList.getValues();

    int i = 0;
    //TODO make this for individual faces
    for (std::vector<RenderMaterial*>::const_iterator it=vals.begin();it!=vals.end();++it, i++) {
        if(strcmp((*it)->Link.getValue()->getNameInDocument(), partName) == 0)
            return (*it);
    }
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
    renderer->attachRenderMaterials(matListCopy->getValues());
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
    renderer->attachRenderMaterials(MaterialsList.getValues());
    renderer->setRenderPreset(Preset.getValue());
    renderer->setUpdateInteval(UpdateInterval.getValue());
    renderer->setRenderSize(OutputX.getValue(), OutputY.getValue());
    renderer->preview();
}

void RenderFeature::render()
{
    if(!isRendererReady())
        return;

    renderer->attachRenderMaterials(MaterialsList.getValues());
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
