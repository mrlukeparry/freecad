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

#ifndef RAYTRACING_RENDERFEATURE_H
#define RAYTRACING_RENDERFEATURE_H

#include <App/PropertyStandard.h>
#include <App/PropertyFile.h>
#include <App/FeaturePython.h>
#include <App/DocumentObject.h>
#include <Base/Axis.h>

#include "Renderer.h"
// #include <Mod/Part/App/Part2DObject.h>
// #include <Mod/Part/App/PropertyGeometryList.h>
// #include <Mod/Sketcher/App/PropertyConstraintList.h>

namespace Raytracing
{

class RaytracingExport RenderFeature : public App::DocumentObject
{
    PROPERTY_HEADER(Raytracing::RenderFeature);

public:
    RenderFeature();
    ~RenderFeature();

    /// Properties
    App::PropertyEnumeration    RendererType;
    
//     Part    ::PropertyGeometryList   Geometry;
//     Sketcher::PropertyConstraintList Constraints;
//     App     ::PropertyLinkSubList    ExternalGeometry;
    /** @name methods overide Feature */
    //@{
    /// recalculate the Feature
    App::DocumentObjectExecReturn *execute(void);

    /// returns the type name of the ViewProvider
    const char* getViewProviderName(void) const {
        return "RaytracingGui::ViewProviderRender";
    }
    //@}

    void removeRenderer(void);
    void setRenderer(const char *);

    Renderer * getRenderer(void) const { return renderer; }

    // from base class
    virtual PyObject *getPyObject(void);
    virtual unsigned int getMemSize(void) const;
    virtual void Save(Base::Writer &/*writer*/) const;
    virtual void Restore(Base::XMLReader &/*reader*/);

protected:
    /// get called by the container when a property has changed
    virtual void onChanged(const App::Property* /*prop*/);
    virtual void onDocumentRestored();
    virtual void onFinishDuplicating();

    Renderer *renderer;
    static const char* TypeEnums[];
};

 typedef App::FeaturePythonT<RenderFeature> RenderFeaturePython;

} //namespace Raytracing

#endif // RAYTRACING_RENDERFEATURE_H