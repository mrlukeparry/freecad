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


#ifndef _AppearancesInst_h_
#define _AppearancesInst_h_

#include <Base/Factory.h>
#include <QXmlStreamReader>
#include <QFile>
#include "LibraryMaterial.h"

namespace Raytracing
{

class AppearancesInstP;

class RaytracingExport AppearancesInst : public Base::Factory
{
public:
//     static void initialiseMaterials();
    static AppearancesInst& instance(void);
    static void destruct (void);
    AppearancesInst(void);
    ~AppearancesInst(void);
    const LibraryMaterial * getMaterial(const char *provides, const char *provider);
    const LibraryMaterial * getMaterialById(const char *id);
    std::vector<LibraryMaterial *> getMaterialsByProvider(const char *provider);
    void scanMaterials();
    void setUserMaterialsPath(const char *);

//     static Material getMaterial();
private:
  void clearMaterials(void);
  std::vector<LibraryMaterial *> parseXML(QString filename);
  std::vector<LibraryMaterial *> parseXML(const QFile &file);
  LibraryMaterial * readMaterialXML();
  MaterialParameter * readMaterialParamXML();

  QXmlStreamReader xml;
  static AppearancesInst* _pcSingleton;

  AppearancesInstP* d;
};

/// Get the global instance
inline AppearancesInst& Appearances(void)
{
    return AppearancesInst::instance();
}
}
#endif //_AppearancesInst_h_