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

#include <Base/Writer.h>
#include <Base/Reader.h>
#include <Base/Tools.h>
#include <Base/Console.h>

// Each Render Plugin listed below
#include "renderer/lux/LuxRender.h"

#include "Renderer.h"
#include "RenderFeature.h"
#include "RenderFeaturePy.h"

using namespace Raytracing;
using namespace Base;

const char* RenderFeature::TypeEnums[]= {"Lux","Povray",NULL};

PROPERTY_SOURCE(Raytracing::RenderFeature, App::DocumentObject)

RenderFeature::RenderFeature()
{
    ADD_PROPERTY(RendererType,((long)0));
    RendererType.setEnums(TypeEnums);
//     ADD_PROPERTY_TYPE(Geometry,        (0)  ,"Sketch",(App::PropertyType)(App::Prop_None),"Sketch geometry");
//     ADD_PROPERTY_TYPE(Constraints,     (0)  ,"Sketch",(App::PropertyType)(App::Prop_None),"Sketch constraints");
//     ADD_PROPERTY_TYPE(ExternalGeometry,(0,0),"Sketch",(App::PropertyType)(App::Prop_None),"Sketch external geometry");
}

RenderFeature::~RenderFeature()
{
    removeRenderer();
}

void RenderFeature::setRenderer(const char *renderType)
{
    // Clear the previous Renderer
    this->removeRenderer();

    if(renderType == "Lux") {
        this->renderer = new LuxRender();
    } else if(renderType == "Povray") {
        Base::Console().Log("Currently not implemented");
    }

}

void RenderFeature::removeRenderer(void)
{
    // Determine the type to ensure the correct renderer destructor is called
    if(this->renderer->getTypeId() == LuxRender::getClassTypeId())
    {
        LuxRender *myRenderer = static_cast<LuxRender *>(renderer);
        delete myRenderer;
    }
    this->renderer = 0; // Make it a null pointer
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
