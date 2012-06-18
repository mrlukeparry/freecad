/***************************************************************************
 *   Copyright (c) 2011                                                    *
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
# include <Standard_math.hxx>
# ifdef FC_OS_WIN32
# include <windows.h>
# undef min
# undef max
# endif
# ifdef FC_OS_MACOSX
# include <OpenGL/gl.h>
# else
# include <GL/gl.h>
# endif
# include <cfloat>
# include <algorithm>
# include <QFontMetrics>
# include <QGLWidget>
# include <QPainter>
# include <QPen>
# include <Inventor/SoPrimitiveVertex.h>
# include <Inventor/actions/SoGLRenderAction.h>
# include <Inventor/misc/SoState.h>
# include <Inventor/fields/SoSFFloat.h>
# include <Inventor/fields/SoSFUShort.h>
# include <math.h>
#endif

#include <Inventor/SbBox3f.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>

#include "SoLineVisual.h"

using namespace SketcherGui;
// Custom So ode to draw infinite lines for the Sketcher Draw Handler that isn't pure hackery
// ------------------------------------------------------

SO_NODE_SOURCE(SoLineVisual);

void SoLineVisual::initClass()
{
    SO_NODE_INIT_CLASS(SoLineVisual, SoShape, "Shape");
}


SoLineVisual::SoLineVisual()
{
    SO_NODE_CONSTRUCTOR(SoLineVisual);
    SO_NODE_ADD_FIELD(colors, (SbVec3f(1.0f,1.0f,1.0f)));
    SO_NODE_ADD_FIELD(pnts, (SbVec3f(.0f,.0f,.0f)));
    SO_NODE_ADD_FIELD(widths,(1.f));
    SO_NODE_ADD_FIELD(patterns, (0xffff));
}


void SoLineVisual::computeBBox(SoAction *action, SbBox3f &box, SbVec3f &center)
{
    SoState *state = action->getState();
    
    const SbViewportRegion & vp = SoViewportRegionElement::get(state);
    SbVec2s vpsize = vp.getViewportSizePixels();

    float minX = 0.f, minY = 0.f, maxX, maxY;

    const SbMatrix & mat = SoModelMatrixElement::get(state);

    SbMatrix inv = mat.inverse(); //Transform back to object space

    //FIX ME: Unsure if a bounding box is needed since project onto screen.
    maxX = FLT_EPSILON;
    maxY = FLT_EPSILON;
      
//     if(this->fndPnts.size() > 1) {
//           //Initialise boundaries
//           SbVec3f pnt(this->fndPnts[0][0], this->fndPnts[0][1], 0.f);
//           inv.multVecMatrix(pnt, pnt);
//           minX = pnt[0];
//           minY = pnt[1];
//           maxX = pnt[0];
//           maxY = pnt[1];
//         for (std::vector<SbVec2f>::const_iterator it=this->fndPnts.begin();
//                                         it!=this->fndPnts.end();++it) {
//             SbVec3f pnt((*it)[0], (*it)[1], 0.f);
//             inv.multVecMatrix(pnt, pnt);
//             minX = (pnt[0] < minX) ? pnt[0] : minX;
//             minY = (pnt[1] < minY) ? pnt[1] : minY;
//             maxX = (pnt[0] > maxX) ? pnt[0] : maxX;
//             maxY = (pnt[1] < minY) ? pnt[1] : maxY;
// 
//         }
//     } else {
//       maxX = FLT_EPSILON;
//       maxY = FLT_EPSILON;
//     }

    box.setBounds(SbVec3f(minX, minY, 0.f), SbVec3f(maxX, maxY, 0.f) );
    center.setValue(maxX / 2, maxY / 2, 0.f);
}


void SoLineVisual::generatePrimitives(SoAction * action)
{
// Supposibly Empty

//     this->beginShape(action, SoShape::LINES);
//     for (std::vector<SbVec2f>::const_iterator it=this->fndPnts.begin();
//                                     it!=this->fndPnts.end();++it) {
// 
//         // Primitive Shape is only for text as this should only be selectable
//         SoPrimitiveVertex pv;
//         pv.setNormal( SbVec3f(0.f, 0.f, 1.f) ); //Dummy Normal
//         // Set coordinates
//         pv.setPoint( SbVec3f((*it)[0],(*it)[1], 0.f)  );
//         shapeVertex(&pv);
//     }
//     this->endShape();

}

void SoLineVisual::GLRender(SoGLRenderAction * action)
{
    SoState *state = action->getState();
// Creates rendering problems. Need to learn if problems with bounding box
//     if (!shouldGLRender(action))
//         return;
    if (action->handleTransparency(true))
      return;
    
    int num = this->pnts.getNum();

    // A pair of points is needed to render atleast one line
    if(num < 2) {
      this->fndPnts.clear(); //Clear the found points and return
      return;
    }

    const SbVec3f    *pnts     = this->pnts.getValues(0);
    const float      *widths   = this->widths.getValues(0);
    const SbColor    *colors   = this->colors.getValues(0);
    const ushort     *patterns = this->patterns.getValues(0);

    // Load Projection Matrices
    const SbMatrix & mat = SoModelMatrixElement::get(state);
    const SbViewVolume &vv = SoViewVolumeElement::get(state);
    const SbViewportRegion & vp = SoViewportRegionElement::get(state);

    state->push();

    SbVec2s vpsize = vp.getViewportSizePixels();
    std::vector<SbVec3f> pntCont;

    // Iterate Points and calculate project points
    for(int i = 0; i < num; i++){
      SbVec3f pnt = pnts[i];
      SbVec3f projPnt;
      mat.multVecMatrix(pnt, pnt);
      vv.projectToScreen(pnt, pnt);

      pnt[0] = pnt[0] * float(vpsize[0]);
      pnt[1] = pnt[1] * float(vpsize[1]);
      pnt[2] = pnt[2] * 2.0f - 1.0f; // change z range from [0,1] to [-1,1]

      pntCont.push_back(pnt);
    }

    // Projection Settings
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, vpsize[0], 0, vpsize[1], -1.0f, 1.0f);

    this->fndPnts.clear();

    glBegin(GL_LINES);
    //glEnable(GL_LINE_STIPPLE);

    //For algorithm later
    //Concave normals
    SbVec2f p[4]; SbVec2f n[4];

    p[0] = SbVec2f(0.f,0.f);       n[0] = SbVec2f(1.f,0.f); // Left Side
    p[1] = SbVec2f(0.f,vpsize[1]); n[1] = SbVec2f(0.f,-1.f); // Top Side
    p[2] = SbVec2f(vpsize[0],0.f); n[2] = SbVec2f(-1.f,0.f); // Right Side
    p[3] = SbVec2f(0.f,0.f);       n[3] = SbVec2f(0.f,1.f); // Bottom Side

    for(int i = 0; i < num/2; i++) {

      //glLineStipple(1, patterns[i]);

      //Set Line Properties
      glLineWidth(widths[i]);

      SbColor color = colors[i];
      glColor3f(color[0], color[1], color[2]);

      // Select pairs of points to construct line;
      SbVec3f pnt1 = pntCont[2*i];
      SbVec3f pnt2 = pntCont[2*i+1];

      SbVec3f dir = (pnt2 -pnt1);
      dir.normalize();
      //Arbitrary start length: maxlength length is the diagonal
      float length = sqrt((float) (vpsize[0]*vpsize[0] + vpsize[1] * vpsize[1]));

      SbVec3f a = pnt2 + dir * length;
      SbVec3f b = pnt2 - dir * length;

      SbVec2f A(a[0], a[1]);
      SbVec2f B(b[0], b[1]);
      //Clipping normals

      //Liang-Barsky Algorithm for clipping lines
      for (int j = 0; j < 4; j++) {
            float wecA = (A-p[j]).dot(n[j]);
            float wecB = (B-p[j]).dot(n[j]);

            if ( wecA < 0 && wecB < 0 )
              break;
            if ( wecA > 0 && wecB > 0 )
              continue;

            float t = wecA / (wecA - wecB);
            if ( wecA < 0 )
                A = A + t*(B-A);
            else
                B = A + t*(B-A);
      }
      this->fndPnts.push_back(A);
      this->fndPnts.push_back(B);

      //Draw Line Pair
      glVertex2f(A[0], A[1]);
      glVertex2f(B[0], B[1]);
    }

    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    state->pop();
}