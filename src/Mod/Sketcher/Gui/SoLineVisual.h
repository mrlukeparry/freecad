/***************************************************************************
 *                                                                         *
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

#ifndef SKETCHERGUI_SOLINEVISUAL_H
#define SKETCHERGUI_SOLINEVISUAL_H

#include <vector>
#include <Inventor/fields/SoSubField.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoMFColor.h>

#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFName.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoMFUShort.h>
#include <Inventor/fields/SoMFVec2f.h>
#include <Inventor/fields/SoMFVec3f.h>

#include <3rdParty/salomesmesh/inc/Rn.h>

namespace SketcherGui {

class SketcherGuiExport SoLineVisual : public SoShape {
    typedef SoShape inherited;

    SO_NODE_HEADER(SoLineVisual);

public:

    static void initClass();
    SoLineVisual();
    SoMFColor   colors;
    SoMFVec3f   pnts;
    SoMFFloat   widths;
    SoMFUShort  patterns;


protected:
    virtual ~SoLineVisual() {};
    virtual void GLRender(SoGLRenderAction *action);
    virtual void computeBBox(SoAction *, SbBox3f &box, SbVec3f &center);
    virtual void generatePrimitives(SoAction * action);

private:
  std::vector<SbVec2f> fndPnts;
};


}
#endif // SKETCHERGUI_SOLINEVISUAL_H
