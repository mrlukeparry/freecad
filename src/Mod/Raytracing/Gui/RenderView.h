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

#ifndef RAYTRACINGGUI_RENDERVIEW_H
#define RAYTRACINGGUI_RENDERVIEW_H

#include <Gui/MDIView.h>
#include <QDeclarativeView>
#include <Mod/Raytracing/App/Renderer.h>

using namespace Raytracing;
namespace RaytracingGui
{

class RenderView : public Gui::MDIView
{
    Q_OBJECT

public:
    RenderView(Gui::Document* doc, QWidget* parent = 0);

public Q_SLOTS:
  void updateOutput();

Q_SIGNALS:
  void updatePreview(QVariant);

public:
    bool onMsg(const char* pMsg,const char** ppReturn);
    bool onHasMsg(const char* pMsg) const;
    void attachRender(Renderer *renderer);

    PyObject* getPyObject();

protected:
//     void contextMenuEvent(QContextMenuEvent *event);

private:
    QDeclarativeView *view;
    Renderer *render;
};

} // namespace RaytracingGui

#endif // RAYTRACINGGUI_RENDERVIEW_H
