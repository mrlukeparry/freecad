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


#ifndef _RendererLights_h_
#define _RendererLights_h_

#include <Base/Vector3D.h>
#include <Base/Placement.h>

# include <gp_Vec.hxx>
#include <vector>

namespace Raytracing
{


class RenderLight
{
public:
    enum LightingType {
    AREA,
    DISTANT,
    MESH,
    INFINITE,
    POINT,
    SPOT };

    RenderLight()
    {
      Color[0] = 1.f;
      Color[1] = 1.f;
      Color[2] = 1.f;

      Power = 100.;
    }
    ~RenderLight(){}

    void setPlacement(const Base::Vector3d &pos, const Base::Rotation &rot) {
        Placement = Base::Placement(pos, rot);
    }
    void setColor(int r, int g, int b)
    {
      Color[0] = (float) r / 255;
      Color[1] = (float) g / 255;
      Color[2] = (float) b / 255;
    }

    Base::Vector3d  Pos;
    LightingType LightType;
    Base::Placement Placement;
    float Power;
    float Color[3];

};


// An area light is a rectangular or square planar light source
class RenderAreaLight : public RenderLight
{
public:
    RenderAreaLight(){ LightType = AREA; }
    RenderAreaLight(int x, int y) : Width(x), Height(y)
    {
        LightType = AREA;
    }

    ~RenderAreaLight(){}

    float Width;
    float Height;
    std::string Name;

    void generateGeometry(Base::Vector3d *pnts)
    {
        //Generate Transformed Plane
        Placement.getRotation().multVec(Base::Vector3d(-Width/2,  Height/2, 0), pnts[0]);
        Placement.getRotation().multVec(Base::Vector3d(-Width/2, -Height/2, 0), pnts[1]);
        Placement.getRotation().multVec(Base::Vector3d( Width/2,  Height/2, 0), pnts[2]);
        Placement.getRotation().multVec(Base::Vector3d( Width/2, -Height/2, 0), pnts[3]);
        Base::Vector3d offset =  Placement.getPosition();
        pnts[0] += offset;
        pnts[1] += offset;
        pnts[2] += offset;
        pnts[3] += offset;
    }
};
}
#endif //_RendererLights_h_