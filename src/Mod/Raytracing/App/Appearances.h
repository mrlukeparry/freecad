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


#ifndef _Appearances_h_
#define _Appearances_h_

#include <Base/BaseClass.h>
#include <QXmlStreamReader>
#include <QFile>
#include "Material.h"

namespace Raytracing
{

class AppRaytracingExport Appearances : public Base::BaseClass
{
    TYPESYSTEM_HEADER();
public:
//     static void initialiseMaterials();
    Appearances(void);
    ~Appearances(void);
    void scanMaterials();
    void setUserMaterialsPath(const char *);

//     static Material getMaterial();
protected:
  std::vector<Material *> parseXML(QString filename);
  std::vector<Material *> parseXML(const QFile &file);
  Material * readMaterialXML();

  QXmlStreamReader xml;

  std::vector<Material *> materials;
  QString userMaterialsPath;
};

}
#endif //_Appearances_h_