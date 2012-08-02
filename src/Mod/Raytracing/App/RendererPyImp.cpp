/***************************************************************************
 *   Copyright (c) Luke Parry            (l.parry@warwick.ac.uk)  2012     *
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

#include <Base/VectorPy.h>
#include "Renderer.h"

// inclusion of the generated files (generated out of RendererPy.xml)
#include "RendererPy.h"
#include "RendererPy.cpp"
// other python types


using namespace Raytracing;

// returns a string which represents the object e.g. when printed in python
std::string RendererPy::representation(void) const
{
    return "<Raytracing::Renderer>";
}

PyObject* RendererPy::preview(PyObject *args)
{
    int x1, y1, x2, y2;
    if (PyArg_ParseTuple(args, "iiii", &x1, &y1, &x2, &y2))
        this->getRendererPtr()->preview(x1,y1,x2,y2);
    else
        this->getRendererPtr()->preview();
    Py_Return;
}

PyObject* RendererPy::render(PyObject *args)
{
    this->getRendererPtr()->render();
    Py_Return;
}

PyObject* RendererPy::finish(PyObject *args)
{
    this->getRendererPtr()->finish();
    Py_Return;
}

PyObject* RendererPy::setRenderSize(PyObject *args)
{
    int x,y;
    if (!PyArg_ParseTuple(args, "ii", &x, &y))
        return 0;

    this->getRendererPtr()->setRenderSize(x,y);
    Py_Return;
}

PyObject* RendererPy::setOutputPath(PyObject *args)
{
    char *outputPath;
    if (!PyArg_ParseTuple(args, "s", outputPath))
        return 0;

    this->getRendererPtr()->setOutputPath(outputPath);
    Py_Return;
}

PyObject* RendererPy::setCamera(PyObject *args)
{
    PyObject *pcObj1, *pcObj2, *pcObj3, *pcObj4;

     // Two Lines, radius
    if (PyArg_ParseTuple(args, "O!O!O!O!", &(Base::VectorPy::Type), &pcObj1, &(Base::VectorPy::Type), &pcObj2,
                                           &(Base::VectorPy::Type), &pcObj3, &(Base::VectorPy::Type), &pcObj4 )) {

        Base::Vector3d v1 = static_cast<Base::VectorPy*>(pcObj1)->value();
        Base::Vector3d v2 = static_cast<Base::VectorPy*>(pcObj2)->value();
        Base::Vector3d v3 = static_cast<Base::VectorPy*>(pcObj3)->value();
        Base::Vector3d v4 = static_cast<Base::VectorPy*>(pcObj4)->value();

        this->getRendererPtr()->setCamera(v1, v2, v3, v4);
        Py_Return;
    }
}

PyObject* RendererPy::clear(PyObject *args)
{
    this->getRendererPtr()->finish();
    Py_Return;
}


// +++ custom attributes implementer ++++++++++++++++++++++++++++++++++++++++


PyObject *RendererPy::getCustomAttributes(const char* /*attr*/) const
{
    return 0;
}

int RendererPy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0;
}


