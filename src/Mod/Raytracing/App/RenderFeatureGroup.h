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

#ifndef _RAYTRACING_RENDERFEATUREGROUP_h_
#define _RAYTRACING_RENDERFEATUREGROUP_h_

#include <App/DocumentObjectGroup.h>
#include <App/PropertyStandard.h>
#include <App/PropertyLinks.h>

namespace Raytracing
{


/** Base class of all View Features in the drawing module
 */
class RaytracingExport RenderFeatureGroup : public App::DocumentObjectGroup
{
    PROPERTY_HEADER(RenderFeature::RenderFeatureGroup);

public:
    /// Constructor
    RenderFeatureGroup(void);
    virtual ~RenderFeatureGroup();

    App::PropertyLinkList Features;

    /// returns the type name of the ViewProvider
    virtual const char* getViewProviderName(void) const {
        return "RaytracingGui::ViewProviderRender";
    }
};


} //namespace Raytracing
#endif