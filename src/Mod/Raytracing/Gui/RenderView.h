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

#include <QDeclarativeItem>
#include <QGraphicsSceneWheelEvent>

class ImageProvider;
using namespace Raytracing;
namespace RaytracingGui
{

/// QML Mouse wheel is not support so create custom type see http://qt-project.org/doc/qt-4.8/qml-mousearea.html
class WheelArea : public QDeclarativeItem
{
    Q_OBJECT

public:
    explicit WheelArea(QDeclarativeItem *parent = 0) : QDeclarativeItem(parent) {}

protected:
    void wheelEvent(QGraphicsSceneWheelEvent *event) {
        switch(event->orientation())
        {
            case Qt::Horizontal:
                Q_EMIT horizontalWheel(event->delta());
                break;
            case Qt::Vertical:
                Q_EMIT verticalWheel(event->delta());
                break;
            default:
                event->ignore();
                break;
        }
    }

Q_SIGNALS:
    void verticalWheel(int delta);
    void horizontalWheel(int delta);
};


class RenderView : public Gui::MDIView
{
    Q_OBJECT

public:
    RenderView(Gui::Document* doc, QWidget* parent = 0);

public Q_SLOTS:
  void updateOutput();
  void stopRender();
  void saveRender();

Q_SIGNALS:
  void updatePreview();

public:
    void attachRender(Renderer *renderer);
    bool canClose(void);
    bool onMsg(const char* pMsg,const char** ppReturn);
    bool onHasMsg(const char* pMsg) const;
    PyObject* getPyObject();

protected:
//     void contextMenuEvent(QContextMenuEvent *event);

    void closeEvent(QCloseEvent *e);

private:
    ImageProvider *imgProv;
    QDeclarativeView *view;
    Renderer *render;
};

} // namespace RaytracingGui

#endif // RAYTRACINGGUI_RENDERVIEW_H
