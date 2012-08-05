/***************************************************************************
 *   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2010     *
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
#include <strstream>
#include "Mod/Raytracing/App/RenderCamera.h"
#include <Base/VectorPy.h>

#include "RenderCamera.h"
// inclusion of the generated files (generated out of RenderCameraPy.xml)
#include "RenderCameraPy.h"
#include "RenderCameraPy.cpp"

using namespace Raytracing;

PyObject *RenderCameraPy::PyMake(struct _typeobject *, PyObject *, PyObject *)  // Python wrapper
{
    // create a new instance of RenderCameraPy and the Twin object
    return new RenderCameraPy(new RenderCamera);
}

// constructor method
int RenderCameraPy::PyInit(PyObject* args, PyObject* /*kwd*/)
{
    if (PyArg_ParseTuple(args, "")) {
        return 0;
    }

    PyErr_Clear();

    char *CameraType;
    PyObject *pcObj1, *pcObj2, *pcObj3, *pcObj4;

    if (PyArg_ParseTuple(args, "O!O!O!O!s", &(Base::VectorPy::Type), &pcObj1, &(Base::VectorPy::Type), &pcObj1,
                                            &(Base::VectorPy::Type), &pcObj3, &(Base::VectorPy::Type), &pcObj4,
                                            &CameraType)) {
        // Get the vectors
        Base::Vector3d v1 = static_cast<Base::VectorPy*>(pcObj1)->value();
        Base::Vector3d v2 = static_cast<Base::VectorPy*>(pcObj2)->value();
        Base::Vector3d v3 = static_cast<Base::VectorPy*>(pcObj3)->value();
        Base::Vector3d v4 = static_cast<Base::VectorPy*>(pcObj4)->value();

        bool valid = false;
        if(strcmp("Perspective",CameraType) == 0) {
            valid = true;
            this->getRenderCameraPtr()->Type = RenderCamera::PERSPECTIVE;
        } else if(strcmp("Orthographic",CameraType) == 0) {
            valid = true;
            this->getRenderCameraPtr()->Type = RenderCamera::ORTHOGRAPHIC;
        } else if(strcmp("Environment",CameraType) == 0) {
            valid = true;
            this->getRenderCameraPtr()->Type = RenderCamera::ENVIRONMENT;
        } else if(strcmp("Realistic",CameraType) == 0) {
            valid = true;
            this->getRenderCameraPtr()->Type = RenderCamera::REALISTIC;
        }

        if (valid) {
            this->getRenderCameraPtr()->CamPos = v1;
            this->getRenderCameraPtr()->CamDir = v2;
            this->getRenderCameraPtr()->LookAt = v3;
            this->getRenderCameraPtr()->Up     = v4;
            return 0;
        }
    }

    PyErr_SetString(PyExc_TypeError, "Invalid constructor for RenderCamera:\n");
    return -1;
}

// returns a string which represents the object e.g. when printed in python
std::string RenderCameraPy::representation(void) const
{
    std::stringstream result;
    result << "<RenderCamera " ;
    switch(this->getRenderCameraPtr()->Type) {
      case RenderCamera::PERSPECTIVE   : result << "'Perspective'>";break;
        case RenderCamera::ORTHOGRAPHIC  : result << "'Orthographic'>";break;
        case RenderCamera::ENVIRONMENT   : result << "'Environment'>";break;
        case RenderCamera::REALISTIC     : result << "'Realistic'>";break;
        default                          : result << "'?'>";break;
    }
    return result.str();
}

Py::Float RenderCameraPy::getFocaldistance(void) const
{
    return Py::Float(this->getRenderCameraPtr()->Focaldistance);
}

void RenderCameraPy::setFocaldistance(Py::Float arg)
{
    this->getRenderCameraPtr()->Focaldistance = arg;
}

Py::Boolean RenderCameraPy::getAutofocus(void) const
{
    return Py::Boolean(this->getRenderCameraPtr()->Autofocus);
}

void RenderCameraPy::setAutofocus(Py::Boolean arg)
{
    this->getRenderCameraPtr()->Autofocus = arg;
}


PyObject *RenderCameraPy::getCustomAttributes(const char* /*attr*/) const
{
    return 0;
}

int RenderCameraPy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0; 
}
