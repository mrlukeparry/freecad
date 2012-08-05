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

#include "RenderCamera.h"
#include "RenderCameraPy.h"

using namespace Raytracing;
using namespace Base;

TYPESYSTEM_SOURCE(Raytracing::RenderCamera, Base::Persistence)

RenderCamera::RenderCamera(const RenderCamera& from)
: Type(from.Type),
  Name(from.Name),
  Fov(from.Fov),
  Focaldistance(from.Focaldistance),
  Autofocus(from.Autofocus),
  CamPos(from.CamPos),
  CamDir(from.CamDir),
  LookAt(from.LookAt),
  Up(from.Up)
{
}

RenderCamera *RenderCamera::clone(void) const
{
    return new RenderCamera(*this);
}

void RenderCamera::setType(const char *type)
{
    if(strcmp("Perspective", type) == 0)
        Type = PERSPECTIVE;
    else if(strcmp("Orthographic", type) == 0)
        Type = ORTHOGRAPHIC;
    else if(strcmp("Environment", type) == 0)
        Type = ENVIRONMENT;
    else if(strcmp("Realistic", type) == 0)
        Type = REALISTIC;
}

PyObject *RenderCamera::getPyObject(void)
{
    return new RenderCameraPy(new RenderCamera(*this));
}

unsigned int RenderCamera::getMemSize (void) const
{
    return 0;
}


void RenderCamera::Save (Writer &writer) const
{
    writer.Stream() << writer.ind() << "<RenderCamera "
    << "Name=\""          <<  Name            << "\" "
    << "Type=\""          <<  (int)Type       << "\" "
    << "Fov =\""          <<  Fov             << "\" "
    << "FocalDistance=\"" <<  Focaldistance   << "\" "
    << "AutoFocus=\""     <<  Autofocus<< "\" />"
    << std::endl;
}

void RenderCamera::Restore(XMLReader &reader)
{
    reader.readElement("RenderCamera");
    Name           = reader.getAttribute("Name");
    Focaldistance  = reader.getAttributeAsFloat("FocalDistance");
    Fov            = reader.getAttributeAsFloat("Fov");
    Autofocus      = (bool) reader.getAttributeAsInteger("AutoFocus");
}
