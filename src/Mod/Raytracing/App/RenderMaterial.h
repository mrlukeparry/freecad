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

#ifndef _RAYTRACING_RENDERMATERIAL_h_
#define _RAYTRACING_RENDERMATERIAL_h_

#include <App/PropertyLinks.h>
#include <App/PropertyStandard.h>
#include <Base/Persistence.h>

#include "Appearances.h"
#include "LibraryMaterial.h"

namespace Raytracing {

// These are for storing actual material properties within a hash and are designed to be lightweight
class MaterialProperty
{
public:
  MaterialProperty(MaterialParameter::Type propType): type(propType) {};
  ~MaterialProperty() {};
  MaterialParameter::Type getType() { return type;}
private:
  MaterialParameter::Type type;
};

// Template class used for storing material types
// Property Type must be either FLOAT, COLOR, BOOL, TEXTURE, STRING
class MaterialBoolProperty : public MaterialProperty
{
public:
  MaterialBoolProperty (bool val)
  : MaterialProperty(MaterialParameter::BOOL), value(val) {}
  ~MaterialBoolProperty (){}

  void setValue(bool val) { value = val;}
  bool getValue(void) { return value; }
private:
  bool value;
};

class MaterialFloatProperty : public MaterialProperty
{
public:
  MaterialFloatProperty(float val)
  : MaterialProperty(MaterialParameter::FLOAT), value(val) {}
  ~MaterialFloatProperty(){}

  void setValue(float val) { value = val;}
  float getValue(void) { return value; }
private:
  float value;
};

class MaterialColorProperty : public MaterialProperty
{
public:
  MaterialColorProperty(int r, int b, int g)
  : MaterialProperty(MaterialParameter::COLOR) { setValue(r,g,b); }

  MaterialColorProperty(float *val)
  : MaterialProperty(MaterialParameter::COLOR) { setValue(val); }
  ~MaterialColorProperty(){}

  void setValue(float *val )
  {
      color[0] = val[0];
      color[1] = val[1];
      color[2] = val[2];
  }
  void setValue(int r, int g, int b)
  {
      color[0] = (float) r / 255;
      color[1] = (float) g / 255;
      color[2] = (float) b / 255;
  }
  float * getValue(void) { return color; }
private:
  float color[3];
};

class RaytracingExport RenderMaterial: public Base::Persistence
{
    TYPESYSTEM_HEADER();

public:
  App::PropertyInteger LinkIndex;
  App::PropertyString  Name;
  App::PropertyString  LibMaterialId;

  RenderMaterial(){ objRef = 0;}
  RenderMaterial(const LibraryMaterial *mat) { LibMaterialId.setValue(mat->id.toStdString()); objRef = 0; }
  RenderMaterial(const RenderMaterial&);
  ~RenderMaterial();

  /// Convenience method for setting both the link id and document object
  void setLink(int idx, App::DocumentObject *obj) { objRef = obj; LinkIndex.setValue(idx); }
  App::DocumentObject* getObjRef() const { return objRef; }

  virtual RenderMaterial * clone(void) const;

  // from base class
  virtual unsigned int getMemSize(void) const;
  virtual void Save(Base::Writer &/*writer*/) const;
  virtual void Restore(Base::XMLReader &/*reader*/);

//   virtual PyObject *getPyObject(void);
  QMap<QString, MaterialProperty *> Properties;
  const LibraryMaterial * getMaterial() const { return Appearances().getMaterialById(LibMaterialId.getValue()); }

private:
    // Only used for reference when onChanged is called in RenderFeature
    App::DocumentObject *objRef;
};

}

#endif //_RAYTRACING_RENDERMATERIAL_h_