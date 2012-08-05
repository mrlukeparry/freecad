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

using namespace Raytracing;
using namespace Base;

const char* RenderFeature::TypeEnums[]= {"Lux","Povray",NULL};

PROPERTY_SOURCE(Raytracing::RenderFeature, App::DocumentObject)

RenderFeature::RenderFeature()
{
    ADD_PROPERTY(RendererType,((long)0));
    ADD_PROPERTY(OutputX,(800));
    ADD_PROPERTY(OutputY,(800));
    RendererType.setEnums(TypeEnums);

    renderer = 0;

//     ADD_PROPERTY_TYPE(Geometry,        (0)  ,"Sketch",(App::PropertyType)(App::Prop_None),"Sketch geometry");
//     ADD_PROPERTY_TYPE(Constraints,     (0)  ,"Sketch",(App::PropertyType)(App::Prop_None),"Sketch constraints");
//     ADD_PROPERTY_TYPE(ExternalGeometry,(0,0),"Sketch",(App::PropertyType)(App::Prop_None),"Sketch external geometry");
}

RenderFeature::~RenderFeature()
{
    removeRenderer();
}

void RenderFeature::attachRenderCamera(RenderCamera *cam)
{
  if(!cam && !renderer)
    return;

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
        Base::Console().Log("Currently not implemented");
    }

    // Reload the materials
    Appearances().scanMaterials();
}

void RenderFeature::removeRenderer(void)
{
    if(!this->renderer)
      return;

    // Determine the type to ensure the correct renderer destructor is called
    if(this->renderer->getTypeId() == LuxRender::getClassTypeId())
    {
        LuxRender *myRenderer = static_cast<LuxRender *>(renderer);
        delete myRenderer;
    }
    this->renderer = 0; // Make it a null pointer
}

/// Methods
void RenderFeature::setCamera(const Base::Vector3d &v1, const Base::Vector3d &v2, const Base::Vector3d &v3, const Base::Vector3d &v4, const char *camType)
{
    if(!renderer)
        Base::Console().Error("Renderer is not available");

    if(!renderer->hasCamera())
        Base::Console().Error("Renderer doesn't have a camera set");

    renderer->getCamera()->setType(camType);
    renderer->setCamera(v1, v2, v3, v4);
}


void RenderFeature::preview(int x1, int y1, int x2, int y2)
{
    if(!isRendererReady())
        return;

    // Argument Variables are temporary
    renderer->setRenderSize(OutputX.getValue(), OutputY.getValue());
    renderer->preview(x1, y1, x2, y2);
}

void RenderFeature::preview()
{
    if(!isRendererReady())
      return;

    renderer->setRenderSize(OutputX.getValue(), OutputY.getValue());
    renderer->preview();
}

void RenderFeature::finish()
{
    if(!renderer)
        Base::Console().Error("Renderer is not available");

    RenderProcess *process = renderer->getRenderProcess();
    if(!process || !process->isActive())
        Base::Console().Error("Renderer process doesn't exist or isn't active");

    renderer->finish();
}

void RenderFeature::reset()
{
    if(!renderer)
      Base::Console().Error("Renderer is not available");

    renderer->reset();
}

bool RenderFeature::isRendererReady()
{
    bool status = false;
    if(!this->renderer) {
        Base::Console().Error("Renderer is not available");
        return status;
    }

    if(!this->renderer->hasCamera())
        Base::Console().Error("Camera has not been set for the render");
}

void RenderFeature::render()
{
    if(!isRendererReady())
        return;
    
    renderer->setRenderSize(OutputX.getValue(), OutputY.getValue());
    renderer->render();
}

void RenderFeature::setRenderSize(int x, int y)
{
    if(x < 0 || y < 0)
          Base::Console().Error("Render size values are incorrect");

    OutputX.setValue(x);
    OutputY.setValue(y);
}

void RenderFeature::setOutputPath(const char * outputPath)
{
    if(!renderer)
       Base::Console().Error("Renderer is not available");

    renderer->setOutputPath(outputPath);
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

}

void RenderFeature::Restore(XMLReader &reader)
{
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
