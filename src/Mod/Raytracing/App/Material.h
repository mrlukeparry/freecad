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

#ifndef _Material_h_
#define _Material_h_

#include <QString>

namespace Raytracing
{

class MaterialParameter
{

};

class AppRaytracingExport Material
{
public:
    enum MaterialSource {
    BUILTIN,
    EXTERNAL};

    Material();
    ~Material();

  Material * copy() const;
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
#endif //_Material_h_