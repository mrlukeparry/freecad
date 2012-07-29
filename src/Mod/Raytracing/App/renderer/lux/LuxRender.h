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


#ifndef _LuxRender_h_
#define _LuxRender_h_

#include "../../Renderer.h"
#include <vector>
#include <string>

namespace Raytracing
{

class AppRaytracingExport LuxRender : public Renderer
{
public:
  LuxRender(void);
  ~LuxRender(void);
  void generateScene();
  void render();

private:
    std::string genCamera(RenderCamera *camera) const;
    std::string genFace(const TopoDS_Face& aFace, int index);
    std::string genLight(RenderLight *light) const;
    std::string genMaterial(RenderMaterial *mat);
    std::string genObject(RenderPart *object);
    std::string genRenderProps();
};

} // namespace Raytracing

#endif // _LuxRender_h_
