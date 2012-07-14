/***************************************************************************
 *   Copyright (c) 2012 Luke Parry           <l.parry@warwick.ac.uk>       *
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


#ifndef PARTDESIGN_SketchPlane_H
#define PARTDESIGN_SketchPlane_H

#include <App/PropertyStandard.h>
#include <App/PropertyUnits.h>

#include <Mod/Part/App/Part2DObject.h>

namespace PartDesign
{

class SketchPlane : public Part::Part2DObject
{
    PROPERTY_HEADER(PartDesign::SketchPlane);

public:
    SketchPlane();
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
      void positionBySupport(void);
      virtual short mustExecute() const;
    //@}
    /// recalculate the feature
    void checkRefTypes(void);
    
    App::DocumentObjectExecReturn *execute(void);

    const char* getViewProviderName(void) const {
        return "PartDesignGui::ViewProviderSketchPlane";
    }

protected:
    void onChanged(const App::Property* prop);
private:
    static const char* TypeEnums[];
    int numFaces;
    int numVertices;
    int numEdges;
};

typedef App::FeaturePythonT<SketchPlane> SketchPlanePython;
} //namespace PartDesign

#endif // PARTDESIGN_SketchPlane_H