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

#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Base/Console.h>
#include <Base/Writer.h>
#include <Base/Reader.h>
#include "RenderMaterial.h"

using namespace Base;
using namespace Raytracing;

TYPESYSTEM_SOURCE(Raytracing::RenderMaterial, Base::Persistence)

// Copy Constructor
RenderMaterial::RenderMaterial(const RenderMaterial& from)
{
    LinkIndex.setValue(from.LinkIndex.getValue());
    Name.setValue(from.Name.getValue());
    LibMaterialId.setValue(from.LibMaterialId.getValue());
    objRef = from.getObjRef();

    // Copy all the properties. New object must be made
    QMap<QString, MaterialProperty *>::const_iterator it = from.Properties.constBegin();
    while (it != from.Properties.constEnd()) {
        switch(it.value()->getType()) {
          case MaterialParameter::COLOR: {
            MaterialColorProperty *ref  = static_cast<MaterialColorProperty*>(it.value());
            MaterialColorProperty *prop = new MaterialColorProperty(ref->getValue());
            Properties.insert(it.key(), prop);
          } break;
          case MaterialParameter::BOOL: {
            MaterialBoolProperty *ref  = static_cast<MaterialBoolProperty*>(it.value());
            MaterialBoolProperty *prop = new MaterialBoolProperty(ref->getValue());
            Properties.insert(it.key(), prop);
          } break;
          case MaterialParameter::FLOAT: {
            MaterialFloatProperty *ref  = static_cast<MaterialFloatProperty*>(it.value());
            MaterialFloatProperty *prop = new MaterialFloatProperty(ref->getValue());
            Properties.insert(it.key(), prop);
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
    writer.Stream() << writer.ind() << "<RenderMaterial "
    << "Name=\""          <<  Name.getValue()            << "\" "
    << "LibMatId=\""      <<  LibMaterialId.getValue()   << "\" "
    << "LinkIndex=\""     <<  LinkIndex.getValue()        << "\" >\n";

    writer.incInd();

    // Save the Material Properties by iterating through properties
    // TODO should we make individual methods for save

    QMap<QString, MaterialProperty *>::const_iterator it = Properties.begin();


    writer.Stream() << writer.ind() << "<Properties "
    << "Count=\""   << Properties.size()      << "\">\n";

    writer.incInd();

    while (it != Properties.end()) {
        switch(it.value()->getType()) {
          case MaterialParameter::COLOR: {
            MaterialColorProperty *prop = static_cast<MaterialColorProperty*>(it.value());

            float *colorVal = prop->getValue();

            writer.Stream() <<  writer.ind()                << "<Property "
            << "Name=\""    <<  it.key().toStdString()      << "\" "
            << "Type=\""    <<  "Color"                     << "\" "
            << "r=\""       <<  colorVal[0]                 << "\" "
            << "g=\""       <<  colorVal[1]                 << "\" "
            << "b=\""       <<  colorVal[2]                 << "\" />\n";
          } break;
          case MaterialParameter::BOOL: {
            MaterialBoolProperty *prop = static_cast<MaterialBoolProperty*>(it.value());

            std::string val = (prop->getValue()) ? "true" : "false";

            writer.Stream() <<  writer.ind()              << "<Property "
            << "Name=\""    <<  it.key().toStdString()    << "\" "
            << "Type=\""    <<  "Bool"                    << "\" "
            << "Value=\""   <<  val                       << "\" />\n";
          } break;
          case MaterialParameter::FLOAT: {
            MaterialFloatProperty *prop = static_cast<MaterialFloatProperty*>(it.value());

            float val = prop->getValue();
            writer.Stream() <<  writer.ind()             << "<Property "
            << "Name=\""    <<  it.key().toStdString()   << "\" "
            << "Type=\""    <<  "Float"                  << "\" "
            << "Value=\""   <<  val                      << "\" />\n";
          } break;
          default:
            break;
        }
        ++it;
    }

    writer.decInd();
    writer.Stream() << writer.ind() << "</Properties>\n";

    writer.decInd();
    writer.Stream() << writer.ind() << "</RenderMaterial>\n";
}

void RenderMaterial::Restore(XMLReader &reader)
{
    reader.readElement("RenderMaterial");

    Name.setValue(reader.getAttribute("Name"));
    LibMaterialId.setValue(reader.getAttribute("LibMatId"));
    LinkIndex.setValue(reader.getAttributeAsInteger("LinkIndex"));

    reader.readElement("Properties");
    // get the value of my attribute
    int count = reader.getAttributeAsInteger("Count");

    for (int i = 0; i < count; i++) {
        reader.readElement("Property");
        if(!reader.getAttribute("Type"))
            continue;

        QString name = QString::fromAscii(reader.getAttribute("Name"));
        if(name.length() == 0)
            continue;

        std::string type = reader.getAttribute("Type");
        if(type == "Bool") {
            std::string b = reader.getAttribute("Value");
            bool val = (b == "true") ? true : false;

            MaterialBoolProperty *prop = new MaterialBoolProperty(val);
            Properties.insert(name, prop);

        } else if(type == "Float") {
            float val = reader.getAttributeAsFloat("Value");
            MaterialFloatProperty *prop = new MaterialFloatProperty(val);

            Properties.insert(name, prop);

        } else if(type == "Color") {
            float color[3];
            color[0] = reader.getAttributeAsFloat("r");
            color[1] = reader.getAttributeAsFloat("g");
            color[2] = reader.getAttributeAsFloat("b");

            MaterialColorProperty *prop = new MaterialColorProperty(color);

            Properties.insert(name, prop);
        } else {
        }
    }

    reader.readEndElement("Properties");
    reader.readEndElement("RenderMaterialList");
}