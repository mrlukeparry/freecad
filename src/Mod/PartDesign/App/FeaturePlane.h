/***************************************************************************
 *   Copyright (c) 2010 Juergen Riegel <FreeCAD@juergen-riegel.net>        *
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
#ifndef PARTDESIGN_Plane_H
#define PARTDESIGN_Plane_H

#include <App/PropertyStandard.h>
#include <App/PropertyUnits.h>
#include "Feature.h"

namespace PartDesign
{

class PartDesignExport Plane : public PartDesign::Feature
{
    PROPERTY_HEADER(PartDesign::Plane);

public:
    Plane();
    App::PropertyEnumeration    Type;
    App::PropertyDistance       OffsetX;
    App::PropertyDistance       OffsetY;
    App::PropertyDistance       OffsetZ;
    App::PropertyAngle          Rotation;
    App::PropertyBool           Reversed;
    App::PropertyLinkSub        Entity1;
    App::PropertyLinkSub        Entity2;
    App::PropertyLinkSub        Entity3;

    /** @name methods override feature */
    //@{
    /// recalculate the feature
    void checkRefTypes(void);
    App::DocumentObjectExecReturn *execute(void);
    short mustExecute() const;
    const char* getViewProviderName(void) const {
        return "PartDesignGui::ViewProviderPlane";
    }

protected:
    void onChanged(const App::Property* prop);
private:
    static const char* TypeEnums[];
    int numFaces;
    int numVertices;
    int numEdges;
};

} //namespace PartDesign


#endif // PARTDESIGN_Plane_H
