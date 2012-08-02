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

/* For an idea:
 * a list of materials lux render provides
 * Matte
 * Matte Translucent
 * Glossy
 * Glossy Translucent
 * Metal
 * Shiny Metal
 * Mirror
 * Glass
 * Rough glass
 * Scaltter
 * Velvet
 **/

#ifndef _LibraryMaterial_h_
#define _LibraryMaterial_h_

#include <QString>
#include <QMap>

namespace Raytracing
{


/* Stores information about the property materials - this information is created from XML and is only for reference (UI)
 * Required fields:
 * id, type, labeb
 * optional are range, description, default value
 * */
class MaterialParameter
{
public:
  enum Type
  {
    FLOAT,
    COLOR,
    BOOL,
    TEXTURE,
    STRING
  };
  MaterialParameter(const QString &str, Type paramType, const QString &label, QString desc)
                  : id(str),
                    type(paramType),
                    label(label),
                    description(desc)
                    {}
  ~MaterialParameter() {}

  Type getType(void) const { return type;};
  QString getDescription() const { return description; }
  QString getLabel() const { return label; }
  QString getId() const { return id; }
protected:
  Type type;
  QString id;
  QString label;
  QString description;
};

class MaterialParameterFloat : public MaterialParameter
{
public:
  MaterialParameterFloat(const QString &str, Type paramType, QString label, QString desc, float def = 0)
                      : MaterialParameter(str, paramType, label, desc)
  {
      value = def;
      rangeSet = false;
  }
  ~MaterialParameterFloat(){}
  bool hasRange() {return rangeSet;}
  void getRange(float &min, float &max) const { min = range[0]; max = range[1];}

  float setRange(float low, float high) { range[0] = low; range[1] = high;}

private:
  float range[2];
  float value;
  bool rangeSet;
};

class MaterialParameterBool : public MaterialParameter
{
public:
  MaterialParameterBool(const QString &str, Type paramType, QString label, QString desc, bool def = false)
                      : MaterialParameter(str, paramType, label, desc) { value = def; }
  ~MaterialParameterBool(){}
  bool getValue(void) const { return value;}

private:
  bool value;
};

class MaterialParameterColor : public MaterialParameter
{
public:
  MaterialParameterColor(const QString &str, Type paramType, QString label, QString desc)
                       : MaterialParameter(str, paramType, label, desc) { color[0] = 1; color[1] = 1; color[2] = 1;}
  ~MaterialParameterColor(){}
  const float * getValue(void) const { return color;}
  void setValue(int r, int g, int b) { color[0] = (float) r/255; color[1] = (float) g/255; color[2] = (float) b/255;}

private:
  float color[3];
};

class RaytracingExport LibraryMaterial
{

public:
    enum MaterialSource {
    BUILTIN,
    EXTERNAL};

    LibraryMaterial();
    ~LibraryMaterial();

  QMap<QString, MaterialParameter *> parameters;
  QString label;
  QString id;
  QString provides;
  QString compat;
  QString description;
  QString provider; // What render provides this material
  QString providerVer;
  QString filename;
  QString previewFilename;
  MaterialSource source;
};

}
#endif //_LibraryMaterial_h_