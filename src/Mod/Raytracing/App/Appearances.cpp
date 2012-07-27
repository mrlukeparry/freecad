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

#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Sequencer.h>

#include "Appearances.h"

using namespace Raytracing;

namespace Raytracing
{
class AppearancesInstP
{
public:
  std::vector<Material *> materials;
  QString userMaterialsPath;
};
}

AppearancesInst* AppearancesInst::_pcSingleton = NULL;

AppearancesInst& AppearancesInst::instance(void)
{
    if (_pcSingleton == NULL)
    {
        _pcSingleton = new AppearancesInst;
    }

    return *_pcSingleton;
}

AppearancesInst::AppearancesInst(void)
{
  d = new AppearancesInstP;
}

AppearancesInst::~AppearancesInst(void)
{
    for (std::vector<Material *>::const_iterator it=d->materials.begin(); it!= d->materials.end(); ++it)
    {
      delete (*it);
    }
    d->materials.empty();
    delete d;
}

void AppearancesInst::destruct (void)
{
    if (_pcSingleton != 0)
    delete _pcSingleton;
    _pcSingleton = 0;
}

void AppearancesInst::setUserMaterialsPath(const char *path)
{
  d->userMaterialsPath = QString::fromAscii(path);
}

Material * AppearancesInst::readMaterialXML()
{
    // Create a new material on the heap
    Material *material = new Material();
    bool valid = true;

    // Currently on the material element so find attributes if any
    // e.g id="lux_extra_blackLeather" type="external" render="lux" version="0.8"
    material->id          = xml.attributes().value(QString::fromAscii("id")).toString(); // possibly should be hased to ensure unique
    material->provider    = xml.attributes().value(QString::fromAscii("render")).toString();
    material->providerVer = xml.attributes().value(QString::fromAscii("version")).toString(); // Doesn't have to be set

    //Check if an builtin or external material
    if(xml.attributes().value(QString::fromAscii("type")) == "external")
      material->source = Material::EXTERNAL;
    else
      material->source = Material::BUILTIN;

    // -- Look at <material> child elements --- //
    while (xml.readNextStartElement()) {
      if (xml.name() == "label")
          material->label = xml.readElementText();
      else if (xml.name() == "compat")
          material->compat = xml.readElementText();
      else if (xml.name() == "provides")
          material->provides = xml.readElementText();
      else if (xml.name() == "description")
          material->description = xml.readElementText();
      else if (xml.name() == "filename")
          material->filename = xml.readElementText();
      else if (xml.name() == "previewFilename")
          material->previewFilename = xml.readElementText();
    }

    // Check if the correct properties were set
    if(material->id.isEmpty()    || material->provider.isEmpty() ||
       material->label.isEmpty() || material->provider.isEmpty() ||
       (material->source == Material::EXTERNAL && material->filename.isEmpty()))
       valid = false;

    if(!valid) {
      delete material;
      material = 0;
    }

    return material;
}

// It may be possible that multiple materials are defined within an XML File
std::vector<Material *> AppearancesInst::parseXML(QString filename)
{
    std::vector<Material *> materials;
    // Load the XML File
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
      return materials; // The file couldn't be read

    QFileInfo fileInfo(file);
    QDir fileDir = fileInfo.absoluteDir();
        
    // Set the xml stream to use the file
    xml.setDevice(&file);
    while (!xml.atEnd()) {
      if (xml.name() == "Material") {
          Material *mat = readMaterialXML();
          if(mat)
          {
              //If file referneces make these absolute on the XML
              if(!mat->filename.isEmpty())
                  mat->filename = fileDir.absoluteFilePath(mat->filename);

              if(!mat->previewFilename.isEmpty())
                  mat->previewFilename = fileDir.absoluteFilePath(mat->previewFilename);

              d->materials.push_back(mat);
          }
      } 

      xml.readNextStartElement();

    }

    if (xml.hasError()) {
    //           ... // do error handling
    }
    file.close();

    return d->materials;
}

const Material * AppearancesInst::getMaterial(const char *provides, const char *provider)
{
  QString prov = QString::fromAscii(provides);
  QString render = QString::fromAscii(provider);
  // Search all available materials

  for (std::vector<Material *>::const_iterator it= d->materials.begin(); it!= d->materials.end(); ++it){
    if((*it)->provides == prov && (*it)->provider == render) {
      const Material *fndMat =  *it;
      return fndMat;
    }
  }

  return 0;
}

std::vector<Material *> AppearancesInst::getMaterialsByProvider(const char *provider)
{
  QString renderer = QString::fromAscii(provider);
  // Search all available materials
  std::vector<Material *> mats;
  for (std::vector<Material *>::const_iterator it= d->materials.begin(); it!= d->materials.end(); ++it){
    if( (*it)->provider == renderer) {
        mats.push_back(*it);
    }
  }

  return mats;
}

const Material * AppearancesInst::getMaterialById(const char *id)
{
  QString matId = QString::fromAscii(id);
  // Search all available materials
  Material *mat = 0;
  for (std::vector<Material *>::const_iterator it= d->materials.begin(); it!= d->materials.end(); ++it){
    if((*it)->id == matId) {
        const Material *fndMat =  *it;
        return fndMat;
    }
  }

  return 0;
}

void AppearancesInst::scanMaterials()
{

    // List of all XML files with absolute path name
    QStringList parseList;

      //Find a list of xml files within the
    QDir myDir(d->userMaterialsPath);

    //Find any subdirectories containing materials but only one level down
    QStringList dirlist = myDir.entryList(QDir::Dirs | QDir::NoDotDot);

    QStringList fileExt;
    fileExt.push_back(QString::fromAscii("*.xml"));

    // Iterate through
    for (QStringList::const_iterator it=dirlist.begin(); it!=dirlist.end(); ++it){
        QDir dir = myDir;
        dir.cd(*it);
        QStringList xmlFiles = dir.entryList(fileExt );
        for (QStringList::const_iterator file=xmlFiles.begin(); file!=xmlFiles.end(); ++file){
            parseList.append(dir.absoluteFilePath(*file));
        }
    }
//     QStringList files = myDir.entryList(fileExt);
//     for (QStringList::const_iterator file=xmlFiles.begin(); file!=xmlFiles.end(); ++file){
//         parseList.append(myDir.absoluteFilePath(*file));
//     }

    // Need to add parselist from  own library

    if(parseList.size() == 0)
      return;
    // Process all these xml file into Materials

        // Iterate through
    for (QStringList::const_iterator it=parseList.begin(); it!=parseList.end(); ++it)
    {
        //Parse the XML
        std::vector<Material *> mats = parseXML(*it);
        if(mats.size() > 0)
        {
            //If successful store these materials in the collection
            for ( std::vector<Material *>::const_iterator mat=mats.begin(); mat!=mats.end(); ++mat)
                d->materials.push_back(*mat);
        } else {
          // Throw an exception?
        }
    }
}