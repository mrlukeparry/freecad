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
#include "RenderMaterial.h"

using namespace Base;
using namespace Raytracing;

TYPESYSTEM_SOURCE(Raytracing::RenderMaterial, Base::Persistence)

// Copy Constructor
RenderMaterial::RenderMaterial(const RenderMaterial& from)
: Name(from.Name)
{
    LibMaterial = from.getMaterial();
    Link.setValue(from.Link.getValue());

    // Copy all the properties. New object must be made
    QMap<QString, MaterialProperty *>::const_iterator it = from.Properties.constBegin();
    while (it != from.Properties.constEnd()) {
        switch(it.value()->getType()) {
          case MaterialParameter::COLOR: {
            MaterialColorProperty *ref  = static_cast<MaterialColorProperty*>(it.value());
            MaterialColorProperty *prop = new MaterialColorProperty(ref->getValue());
          } break;
          case MaterialParameter::BOOL: {
            MaterialBoolProperty *ref  = static_cast<MaterialBoolProperty*>(it.value());
            MaterialBoolProperty *prop = new MaterialBoolProperty(ref->getValue());
          } break;
          case MaterialParameter::FLOAT: {
            MaterialFloatProperty *ref  = static_cast<MaterialFloatProperty*>(it.value());
            MaterialFloatProperty *prop = new MaterialFloatProperty(ref->getValue());

          } break;
        }
        ++it;
    }
}


RenderMaterial::~RenderMaterial()
{
    // Delete all the parameters stored under the material
    QMap<QString, MaterialProperty *>::iterator it = Properties.begin();
    while (it != Properties.end()) {
        switch(it.value()->getType()) {
          case MaterialParameter::COLOR: {
            MaterialColorProperty *prop = static_cast<MaterialColorProperty*>(it.value());
            delete prop;
          } break;
          case MaterialParameter::BOOL: {
            MaterialBoolProperty *prop = static_cast<MaterialBoolProperty*>(it.value());
            delete prop;
          } break;
          case MaterialParameter::FLOAT: {
            MaterialFloatProperty *prop = static_cast<MaterialFloatProperty*>(it.value());
            delete prop;
          } break;
        }

        it.value() = 0;
        ++it;
    }
    Properties.clear();
}

RenderMaterial *RenderMaterial::clone(void) const
{
    return new RenderMaterial(*this);
}


unsigned int RenderMaterial::getMemSize (void) const
{
    return 0;
}


void RenderMaterial::Save (Writer &writer) const
{
//     writer.Stream() << writer.ind() << "<RenderCamera "
//     << "Name=\""          <<  Name            << "\" "
//     << "Type=\""          <<  (int)Type       << "\" "
//     << "Fov =\""          <<  Fov             << "\" "
//     << "FocalDistance=\"" <<  Focaldistance   << "\" "
//     << "AutoFocus=\""     <<  Autofocus<< "\">"
//     << "<CamPos valueX=\"" <<  CamPos.x << "\" valueY=\"" <<  CamPos.y << "\" valueZ=\"" <<  CamPos.z << "\"/>" << endl
//     << "<CamDir valueX=\"" <<  CamDir.x << "\" valueY=\"" <<  CamDir.y << "\" valueZ=\"" <<  CamDir.z << "\"/>" << endl
//     << "<LookAt valueX=\"" <<  LookAt.x << "\" valueY=\"" <<  LookAt.y << "\" valueZ=\"" <<  LookAt.z << "\"/>" << endl
//     << "<Up valueX=\""     <<  Up.x     << "\" valueY=\"" <<  Up.y     << "\" valueZ=\"" <<  Up.z     << "\"/>" << endl
//     << "</RenderCamera>" << endl
//     << endl;
}

void RenderMaterial::Restore(XMLReader &reader)
{

//     reader.readElement("RenderCamera");
//     Name           = reader.getAttribute("Name");
//     Focaldistance  = reader.getAttributeAsFloat("FocalDistance");
//     Fov            = reader.getAttributeAsFloat("Fov");
//     Autofocus      = (bool) reader.getAttributeAsInteger("AutoFocus");
//
//     reader.readElement("CamPos");
//     CamPos = Base::Vector3d (reader.getAttributeAsFloat("valueX"), reader.getAttributeAsFloat("valueX"), reader.getAttributeAsFloat("valueX"));
//
//     reader.readElement("CamDir");
//     CamDir = Base::Vector3d (reader.getAttributeAsFloat("valueX"), reader.getAttributeAsFloat("valueX"), reader.getAttributeAsFloat("valueX"));
//
//     reader.readElement("LookAt");
//     LookAt = Base::Vector3d (reader.getAttributeAsFloat("valueX"), reader.getAttributeAsFloat("valueX"), reader.getAttributeAsFloat("valueX"));
//
//     reader.readElement("Up");
//     Up = Base::Vector3d (reader.getAttributeAsFloat("valueX"), reader.getAttributeAsFloat("valueX"), reader.getAttributeAsFloat("valueX"));
}
