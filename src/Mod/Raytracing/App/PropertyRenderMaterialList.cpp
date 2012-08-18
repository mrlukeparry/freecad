/***************************************************************************
 *   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2010     *
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
#   include <assert.h>
#endif

/// Here the FreeCAD includes sorted by Base,App,Gui......

#include <Base/Exception.h>
#include <Base/Reader.h>
#include <Base/Writer.h>

#include "PropertyRenderMaterialList.h"
// #include "ConstraintPy.h"

using namespace App;
using namespace Base;
using namespace std;
using namespace Raytracing;


//**************************************************************************
// PropertyRenderMaterialList
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(Raytracing::PropertyRenderMaterialList, App::PropertyLists);

//**************************************************************************
// Construction/Destruction


PropertyRenderMaterialList::PropertyRenderMaterialList()
{

}

PropertyRenderMaterialList::~PropertyRenderMaterialList()
{
    for (std::vector<RenderMaterial*>::iterator it = _lValueList.begin(); it != _lValueList.end(); ++it)
        if (*it) delete *it;
}

void PropertyRenderMaterialList::setSize(int newSize)
{
    for (unsigned int i = newSize; i < _lValueList.size(); i++)
        delete _lValueList[i];
    _lValueList.resize(newSize);
}

int PropertyRenderMaterialList::getSize(void) const
{
    return static_cast<int>(_lValueList.size());
}

void PropertyRenderMaterialList::setValue(const RenderMaterial* lValue)
{
    if (lValue) {
        aboutToSetValue();
        RenderMaterial* newVal = lValue->clone();
        for (unsigned int i = 0; i < _lValueList.size(); i++)
            delete _lValueList[i];
        _lValueList.resize(1);
        _lValueList[0] = newVal;
        hasSetValue();
    }
}

void PropertyRenderMaterialList::setValues(const std::vector<RenderMaterial*>& lValue)
{
    aboutToSetValue();
    applyValues(lValue);
    hasSetValue();
}

void PropertyRenderMaterialList::applyValues(const std::vector<RenderMaterial*>& lValue)
{
    std::vector<RenderMaterial*> oldVals(_lValueList);
    _lValueList.resize(lValue.size());
    // copy all objects
    for (unsigned int i = 0; i < lValue.size(); i++)
        _lValueList[i] = lValue[i]->clone();
    for (unsigned int i = 0; i < oldVals.size(); i++)
        delete oldVals[i];
}

PyObject *PropertyRenderMaterialList::getPyObject(void)
{

}

void PropertyRenderMaterialList::setPyObject(PyObject *value)
{

}

void PropertyRenderMaterialList::Save(Writer &writer) const
{
    writer.Stream() << writer.ind() << "<RenderMaterialList count=\"" << getSize() <<"\">" << endl;
    writer.incInd();
    for (int i = 0; i < getSize(); i++)
        _lValueList[i]->Save(writer);
    writer.decInd();
    writer.Stream() << writer.ind() << "</RenderMaterialList>" << endl ;
}

void PropertyRenderMaterialList::Restore(Base::XMLReader &reader)
{
    reader.readElement("RenderMaterialList");
    // get the value of my attribute
    int count = reader.getAttributeAsInteger("count");

    std::vector<RenderMaterial*> values;
    values.reserve(count);
    for (int i = 0; i < count; i++) {
        RenderMaterial *newMat = new RenderMaterial();
        newMat->Restore(reader);
        values.push_back(newMat);
    }

    reader.readEndElement("RenderMaterialList");

    // assignment
    setValues(values);
}

Property *PropertyRenderMaterialList::Copy(void) const
{
    PropertyRenderMaterialList *p = new PropertyRenderMaterialList();
    p->applyValues(_lValueList);
    return p;
}

void PropertyRenderMaterialList::Paste(const Property &from)
{
    const PropertyRenderMaterialList& FromList = dynamic_cast<const PropertyRenderMaterialList&>(from);
    aboutToSetValue();
    applyValues(FromList._lValueList);
    hasSetValue();
}

unsigned int PropertyRenderMaterialList::getMemSize(void) const
{
    int size = sizeof(PropertyRenderMaterialList);
    for (int i = 0; i < getSize(); i++)
        size += _lValueList[i]->getMemSize();
    return size;
}

std::vector<RenderMaterial *> PropertyRenderMaterialList::_emptyValueList(0);
