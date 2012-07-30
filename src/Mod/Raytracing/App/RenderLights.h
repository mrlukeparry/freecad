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
    enum LightType {
    AREA,
    DISTANT,
    MESH,
    INFINITE,
    POINT,
    SPOT };

    RenderLight()
    {
      color[0] = 1.f;
      color[1] = 1.f;
      color[2] = 1.f;

      power = 100.;
    }
    ~RenderLight(){}


    const float * getColor(void) const { return color;}
    float getPower(void) const { return power;}
    LightType getType(void) { return lightType; }
    void setPower(float pow) { power = pow;}
    void setPlacement(const Base::Vector3d &pos, const Base::Rotation &rot) {
      placement = Base::Placement(pos, rot);
    }
    void setColor(int r, int g, int b)
    {
      color[0] = (float) r / 255;
      color[1] = (float) g / 255;
      color[2] = (float) b / 255;
    }
    const Base::Placement & getPlacement() { return placement; }
    Base::Vector3d  Pos;

protected:
  LightType lightType;
  Base::Placement placement;
  float power;
  float color[3];
};

// An area light is a rectangular or square planar light source
class RenderAreaLight : public RenderLight
{
public:
    RenderAreaLight()
    {
      lightType = AREA;
      width  = 1;
      height = 1;
    }
    ~RenderAreaLight(){}
    void setHeight(float size) { height = size; }
    void setWidth(float size)  { width = size; }
    float getHeight() const { return height; }
    float getWdith() const { return width; }

    void generateGeometry(Base::Vector3d *pnts)
    {
        //Generate Transformed Plane
        placement.getRotation().multVec(Base::Vector3d(-width/2,  height/2, 0), pnts[0]);
        placement.getRotation().multVec(Base::Vector3d(-width/2, -height/2, 0), pnts[1]);
        placement.getRotation().multVec(Base::Vector3d( width/2,  height/2, 0), pnts[2]);
        placement.getRotation().multVec(Base::Vector3d( width/2, -height/2, 0), pnts[3]);
        Base::Vector3d offset =  placement.getPosition();
        pnts[0] += offset;
        pnts[1] += offset;
        pnts[2] += offset;
        pnts[3] += offset;
    }
private:
  float width;
  float height;
};

}
#endif //_RendererLights_h_