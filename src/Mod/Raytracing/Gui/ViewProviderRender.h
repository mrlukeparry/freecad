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

#ifndef RAYTRACINGGUI_VIEWPROVIDERRENDER_H
#define RAYTRACINGGUI_VIEWPROVIDERRENDER_H

#include <Gui/ViewProviderFeature.h>
#include <Mod/Raytracing/App/RenderFeature.h>
#include <Gui/ViewProviderDocumentObjectGroup.h>

#include <Gui/View3DInventorViewer.h>

namespace RaytracingGui {

class RaytracingGuiExport ViewProviderRender : public Gui::ViewProviderDocumentObjectGroup
{
    PROPERTY_HEADER(RaytracingGui::ViewProviderRender);

public:
    /// constructor
    ViewProviderRender();
    /// destructor
    virtual ~ViewProviderRender();

    virtual void attach(App::DocumentObject *);
    virtual void setDisplayMode(const char* ModeName);

    /// returns a list of all possible modes
    virtual std::vector<std::string> getDisplayModes(void) const;
    virtual Raytracing::RenderFeature * getRenderFeature(void) const;

    /// Is called by the tree if the user double click on the object
    virtual bool doubleClicked(void);
    void setupContextMenu(QMenu*, QObject*, const char*);
    virtual void updateData(const App::Property*);

    void getRenderBBox(SbBox3f &box);
    bool mouseMove(const SbVec3f &pos, const SbVec3f &norm, const SoPickedPoint* pp);
protected:
    bool setEdit(int ModNum);
    void unsetEditViewer(Gui::View3DInventorViewer* viewer);
    void setEditViewer(Gui::View3DInventorViewer* viewer, int ModNum);

};

} // namespace RaytracingGui

#endif // RAYTRACINGGUI_VIEWPROVIDERRENDER_H

