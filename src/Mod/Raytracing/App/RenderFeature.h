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
#include <App/DocumentObjectGroup.h>
#include <Base/Axis.h>
#include <Base/BoundBox.h>

#include "Renderer.h"
#include "PropertyRenderMaterialList.h"

namespace Raytracing
{

class RaytracingExport RenderFeature : public App::DocumentObjectGroup
{
    PROPERTY_HEADER(Raytracing::RenderFeature);

public:
    RenderFeature();
    virtual ~RenderFeature();

    /// Properties
    Raytracing::PropertyRenderMaterialList     MaterialsList;
    App::PropertyInteger                       OutputX;
    App::PropertyInteger                       OutputY;
    App::PropertyString                        Preset;
    App::PropertyString                        SceneTemplate;
    App::PropertyEnumeration                   RendererType;
    App::PropertyInteger                       UpdateInterval;
    App::PropertyLinkSubList                   ExternalGeometry;


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


    /// Render MaterialsList
    int addRenderMaterial(RenderMaterial *material, DocumentObject *pcObj);
    int removeRenderMaterial(int index);
    const RenderMaterial * getRenderMaterial(const char *partName) const;
    int removeRenderMaterialFromPart(const char *partName);
    int setRenderMaterial(const RenderMaterial *material);
    int addMatLink(DocumentObject *Obj, const char* SubName);
    int delMatLink(int linkId);
    void updateMatLinks();
    DocumentObject * getRenderMaterialLink(RenderMaterial *material);

    /// Render getters and setters
    void removeRenderer(void);
    void setRenderer(const char *);
    Renderer * getRenderer(void) const { return renderer; }

    bool hasRenderer(void) const;
    bool isRendererReady(void) const;
    void attachRenderCamera(RenderCamera *cam);

    RenderProcess * getActiveRenderProcess(void) const; // Should this be a const method?
    void finish();
    void preview();
    void preview(int x1, int y1, int x2, int y2);
    void render();
    void reset();
    void setCamera(const Base::Vector3d &v1, const Base::Vector3d &v2, const Base::Vector3d &v3, const Base::Vector3d &v4, const char *camType);
    void setCamera(RenderCamera *cam);
    void setBBox(const Base::Vector3d &min, const Base::Vector3d &max);

    RenderCamera * getCamera(void);
    void setRenderPreset(const char * presetName);
    void setRenderTemplate(const char * templateName);
    void setRenderSize(int x, int y);
    void setOutputPath(const char * outputPath);
    void setUpdateInterval(int interval);

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

    Base::BoundBox3f bbox;
    Renderer *renderer;
    RenderCamera *camera;

    static const char* TypeEnums[];
};

 typedef App::FeaturePythonT<RenderFeature> RenderFeaturePython;

} //namespace Raytracing

#endif // RAYTRACING_RENDERFEATURE_H