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

#ifndef _RAYTRACING_RENDERCAMERA_h_
#define _RAYTRACING_RENDERCAMERA_h_

#include <Base/Vector3D.h>
#include <Base/Persistence.h>

namespace Raytracing {

// TODO make this camera undo-redo save. It probably requires changing into App::Properties
class RaytracingExport RenderCamera : public Base::Persistence
{
    TYPESYSTEM_HEADER();

public:
    enum CamType {
    PERSPECTIVE,
    ORTHOGRAPHIC,
    ENVIRONMENT,
    REALISTIC };

    RenderCamera() {};
    virtual RenderCamera * clone(void) const;
    RenderCamera(const RenderCamera&);
    RenderCamera(const Base::Vector3d& cCamPos,const Base::Vector3d& cCamDir,const Base::Vector3d& cLookAt, const Base::Vector3d& cUp, const CamType& cType = PERSPECTIVE)
                 : CamPos(cCamPos),
                   CamDir(cCamDir),
                   LookAt(cLookAt),
                   Up(cUp),
                   Type(cType)
    {
        Autofocus = false;
        Focaldistance = (float) (cLookAt - cCamPos).Length();
        Fov = 45; // TODO should we set this to 45 automatically?
    }


    void setType(const char*);

        // from base class
    virtual unsigned int getMemSize(void) const;
    virtual void Save(Base::Writer &/*writer*/) const;
    virtual void Restore(Base::XMLReader &/*reader*/);

    virtual PyObject *getPyObject(void);

    CamType Type;
    std::string Name;
    Base::Vector3d  CamPos;
    Base::Vector3d  CamDir;
    Base::Vector3d  LookAt;
    Base::Vector3d  Up;
    float Fov;
    float Focaldistance;
    bool Autofocus;
};

}
#endif //_RAYTRACING_RENDERCAMERA_h_