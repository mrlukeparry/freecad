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

#include <Base/VectorPy.h>

#include "Mod/Raytracing/App/RenderFeature.h"
#include "Mod/Raytracing/App/Renderer.h"
#include <App/DocumentObjectGroup.h>

// inclusion of the generated files
#include "RenderFeaturePy.h"
#include "RenderFeaturePy.cpp"

#include "RenderCameraPy.h"

using namespace Raytracing;

// returns a string which represents the object e.g. when printed in python
std::string RenderFeaturePy::representation(void) const
{
    return "<Raytracing::RenderFeature>";
}

PyObject* RenderFeaturePy::preview(PyObject *args)
{
    int x1, y1, x2, y2;
    if (PyArg_ParseTuple(args, "iiii", &x1, &y1, &x2, &y2))
        this->getRenderFeaturePtr()->preview(x1,y1,x2,y2);
    else
        this->getRenderFeaturePtr()->preview();
    Py_Return;
}

PyObject* RenderFeaturePy::render(PyObject *args)
{
    this->getRenderFeaturePtr()->render();
    Py_Return;
}

PyObject* RenderFeaturePy::finish(PyObject *args)
{
    this->getRenderFeaturePtr()->finish();
    Py_Return;
}

PyObject* RenderFeaturePy::attachRenderCamera(PyObject *args)
{
    PyObject *pcObj;
    if (!PyArg_ParseTuple(args, "O", &pcObj))
        return 0;

    if (PyObject_TypeCheck(pcObj, &(Raytracing::RenderCameraPy::Type))) {
        Raytracing::RenderCamera *cam = static_cast<Raytracing::RenderCameraPy*>(pcObj)->getRenderCameraPtr();
        this->getRenderFeaturePtr()->attachRenderCamera(cam);
    }
    Py_Return;
}


PyObject* RenderFeaturePy::setRenderSize(PyObject *args)
{
    int x,y;
    if (!PyArg_ParseTuple(args, "ii", &x, &y))
        return 0;

    this->getRenderFeaturePtr()->setRenderSize(x,y);
    Py_Return;
}

PyObject* RenderFeaturePy::setUpdateInterval(PyObject *args)
{
    int interval;
    if (!PyArg_ParseTuple(args, "i", &interval))
        return 0;

    this->getRenderFeaturePtr()->setUpdateInterval(interval);
    Py_Return;
}


PyObject* RenderFeaturePy::setRenderer(PyObject *args)
{
    char *renderer;
    if (!PyArg_ParseTuple(args, "s", &renderer))
        return 0;

    if(!renderer)
      return 0;
    this->getRenderFeaturePtr()->setRenderer(renderer);
    Py_Return;
}

PyObject* RenderFeaturePy::setRenderPreset(PyObject *args)
{
    char *presetName;
    if (!PyArg_ParseTuple(args, "s", &presetName))
        return 0;

    this->getRenderFeaturePtr()->setRenderPreset(presetName);
    Py_Return;
}

PyObject* RenderFeaturePy::setRenderTemplate(PyObject *args)
{
    char *templateName;
    if (!PyArg_ParseTuple(args, "s", &templateName))
        return 0;

    this->getRenderFeaturePtr()->setRenderTemplate(templateName);
    Py_Return;
}

PyObject* RenderFeaturePy::removeRenderMaterialFromPart(PyObject *args)
{
    char *partName;
    if (!PyArg_ParseTuple(args, "s", &partName))
        return 0;

    this->getRenderFeaturePtr()->removeRenderMaterialFromPart(partName);
    Py_Return;
}
PyObject* RenderFeaturePy::setOutputPath(PyObject *args)
{
    char *outputPath;
    if (!PyArg_ParseTuple(args, "s", &outputPath))
        return 0;

    this->getRenderFeaturePtr()->setOutputPath(outputPath);
    Py_Return;
}

PyObject* RenderFeaturePy::setCamera(PyObject *args)
{
    PyObject *pcObj1, *pcObj2, *pcObj3, *pcObj4;
    char * camType;
     // Two Lines, radius
    if (PyArg_ParseTuple(args, "O!O!O!O!s", &(Base::VectorPy::Type), &pcObj1, &(Base::VectorPy::Type), &pcObj2,
                                           &(Base::VectorPy::Type), &pcObj3, &(Base::VectorPy::Type), &pcObj4, &camType )) {

        Base::Vector3d v1 = static_cast<Base::VectorPy*>(pcObj1)->value(); // CamPos
        Base::Vector3d v2 = static_cast<Base::VectorPy*>(pcObj2)->value(); // CamDir
        Base::Vector3d v3 = static_cast<Base::VectorPy*>(pcObj3)->value(); // Up
        Base::Vector3d v4 = static_cast<Base::VectorPy*>(pcObj4)->value(); // LookAt

        this->getRenderFeaturePtr()->setCamera(v1, v2, v3, v4, camType);
        Py_Return;
    }
}

PyObject* RenderFeaturePy::setBBox(PyObject *args)
{
    PyObject *pcObj1, *pcObj2;

     // Two Lines, radius
    if (PyArg_ParseTuple(args, "O!O!", &(Base::VectorPy::Type), &pcObj1, &(Base::VectorPy::Type), &pcObj2)) {

        Base::Vector3d min = static_cast<Base::VectorPy*>(pcObj1)->value(); // CamPos
        Base::Vector3d max = static_cast<Base::VectorPy*>(pcObj2)->value(); // CamDir

        this->getRenderFeaturePtr()->setBBox(min, max);
        Py_Return;
    }
}

PyObject* RenderFeaturePy::clear(PyObject *args)
{
    this->getRenderFeaturePtr()->finish();
    Py_Return;
}

PyObject *RenderFeaturePy::getCustomAttributes(const char* /*attr*/) const
{
    return 0;
}

int RenderFeaturePy::setCustomAttributes(const char* attr, PyObject* obj)
{
    // search in PropertyList
    App::Property *prop = this->getRenderFeaturePtr()->getPropertyByName(attr);
    if (prop) {
        // Read-only attributes must not be set over its Python interface
        short Type =  this->getRenderFeaturePtr()->getPropertyType(prop);
        if (Type & App::Prop_ReadOnly) {
            std::stringstream s;
            s << "Object attribute '" << attr << "' is read-only";
            throw Py::AttributeError(s.str());
        }

        prop->setPyObject(obj);

        return 1;
    }

    return 0;
}
