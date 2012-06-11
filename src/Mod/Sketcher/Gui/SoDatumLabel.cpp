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
# include <math.h>
#endif

#include <Inventor/elements/SoFontNameElement.h>
#include <Inventor/elements/SoFontSizeElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>

#include "SoDatumLabel.h"
#include <Gui/BitmapFactory.h>

using namespace SketcherGui;

// ------------------------------------------------------

SO_NODE_SOURCE(SoDatumLabel);

void SoDatumLabel::initClass()
{
    SO_NODE_INIT_CLASS(SoDatumLabel, SoShape, "Shape");
}


SoDatumLabel::SoDatumLabel()
{
    SO_NODE_CONSTRUCTOR(SoDatumLabel);
    SO_NODE_ADD_FIELD(string, (""));
    SO_NODE_ADD_FIELD(textColor, (SbVec3f(1.0f,1.0f,1.0f)));
    SO_NODE_ADD_FIELD(pnts, (SbVec3f(.0f,.0f,.0f)));
    
    SO_NODE_ADD_FIELD(name, ("Helvetica"));
    SO_NODE_ADD_FIELD(size, (12.f));

    SO_NODE_ADD_FIELD(datumtype, (SoDatumLabel::DISTANCE));

    SO_NODE_DEFINE_ENUM_VALUE(Type, DISTANCE);
    SO_NODE_DEFINE_ENUM_VALUE(Type, DISTANCEX);
    SO_NODE_DEFINE_ENUM_VALUE(Type, DISTANCEY);
    SO_NODE_DEFINE_ENUM_VALUE(Type, ANGLE);
    SO_NODE_DEFINE_ENUM_VALUE(Type, RADIUS);
    SO_NODE_SET_SF_ENUM_TYPE(datumtype, Type);

    this->bbx = 0;
    this->bby = 0;
    this->imgWidth = 0;
    this->imgHeight = 0;
}

void SoDatumLabel::drawImage()
{
    const SbString* s = string.getValues(0);
    int num = string.getNum();
    if (num == 0) {
        this->image = SoSFImage();
        return;
    }

    QFont font(QString::fromAscii(name.getValue()), size.getValue());
    QFontMetrics fm(font);
    QString str = QString::fromUtf8(s[0].getString());
    int w = fm.width(str);
    int h = fm.height();

    // No Valid text
    if (!w) {
        this->image = SoSFImage();
        return;
    }

    const SbColor& t = textColor.getValue();
    QColor front;
    front.setRgbF(t[0],t[1], t[2]);

    QImage image(w, h,QImage::Format_ARGB32_Premultiplied);
    image.fill(0x00000000);
    
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(front);
    painter.setFont(font);
    painter.drawText(0,0,w,h, Qt::AlignLeft , str);
    painter.end();

    SoSFImage sfimage;
    Gui::BitmapFactory().convert(image, sfimage);
    this->image = sfimage;
}

void SoDatumLabel::computeBBox(SoAction *action, SbBox3f &box, SbVec3f &center)
{
    SbVec2s size;
    int nc;

    const unsigned char * dataptr = this->image.getValue(size, nc);
    if (dataptr == NULL) {
        box.setBounds(SbVec3f(0,0,0), SbVec3f(0,0,0));
        center.setValue(0.0f,0.0f,0.0f);
        return;
    }

    float srcw = size[0];
    float srch = size[1];

    float height, width;
    float length =  this->param1.getValue();

    // Get the points stored
    const SbVec3f *pnts = this->pnts.getValues(0);
    SbVec3f p1 = pnts[0];
    SbVec3f p2 = pnts[1];

    if(action->getTypeId() == SoGLRenderAction::getClassTypeId()) {
         // Update using the GL state
        SoState *state =  action->getState();
        float srcw = size[0];
        float srch = size[1];

        const SbViewVolume & vv = SoViewVolumeElement::get(state);
        float scale = vv.getWorldToScreenScale(SbVec3f(0.f,0.f,0.f), 0.5f);

        float aspectRatio =  (float) srcw / (float) srch;
        this->imgHeight = scale / (float) srch;
        this->imgWidth  = aspectRatio * (float) this->imgHeight;

        // Change the offset and bounding box parameters depending on Datum Type
        if(this->datumtype.getValue() == DISTANCE || this->datumtype.getValue() == DISTANCEX || this->datumtype.getValue() == DISTANCEY ){
            float length =  this->param1.getValue();
            this->bbx =(p2-p1).length();
            this->bby = (length < 0) ? -length - this->imgHeight / 2: length + this->imgHeight / 2 ;
    
        } else if (this->datumtype.getValue() == RADIUS) {
            float length =  this->param1.getValue();

            float dist1 = (p2-p1).length();
            float offsetX = dist1 + length;

            //Line Parameters
            float margin = 0.01f;
            margin *= scale;

            float dist2 = offsetX + this->imgWidth / 2 + margin * 8;

            // Calaculate and store bounding box
            this->bbx = (dist2 > dist1) ? dist2 : dist1;
            this->bby = this->imgHeight;
        }
    }

    width = this->bbx;
    height = this->bby;
    if(this->datumtype.getValue() == DISTANCE || this->datumtype.getValue() == DISTANCEX || this->datumtype.getValue() == DISTANCEY ){
        SbVec3f min (-width / 2, 0.f, 0.f);
        SbVec3f max (width  / 2, ((length < 0)? -height : height), 0.f);
        box.setBounds(min, max);
        center.setValue(width/2, height/2, 0.f);
    } else if (this->datumtype.getValue() == RADIUS) {
        SbVec3f min (0, - height / 2, 0.f);
        SbVec3f max (width, height / 2, 0.f);
        box.setBounds(min, max);
        center.setValue(0.f, 0.f, 0.f);
    }
}

void SoDatumLabel::generatePrimitives(SoAction * action)
{
    // Get the size
    SbVec2s size;
    int nc;

    const unsigned char * dataptr = this->image.getValue(size, nc);
    if (dataptr == NULL)
        return;
    
    float width, height;

    if (action->getTypeId() == SoGLRenderAction::getClassTypeId()) {
         // Update using the GL state
        SoState *state =  action->getState();
        float srcw = size[0];
        float srch = size[1];

        const SbViewVolume & vv = SoViewVolumeElement::get(state);
        float scale = vv.getWorldToScreenScale(SbVec3f(0.f,0.f,0.f), 0.5f);

        float aspectRatio =  (float) srcw / (float) srch;
        this->imgHeight = scale / (float) srch;
        this->imgWidth  = aspectRatio * (float) height;

    } else {
        // Update Primitives using stored dimensions
        width = this->bbx;
        height = this->bby;
    }

    // Get the points stored
    const SbVec3f *pnts = this->pnts.getValues(0);
    SbVec3f p1 = pnts[0];
    SbVec3f p2 = pnts[1];

    float offsetX, offsetY;

    // Change the offset and bounding box parameters depending on Datum Type
    if(this->datumtype.getValue() == DISTANCE || this->datumtype.getValue() == DISTANCEX || this->datumtype.getValue() == DISTANCEY ){
        float length =  this->param1.getValue();
        offsetX = 0;
        offsetY = length;

        // Update Bounding Box Accordingly
//         this->bbx = length * 2;
//         this->bby = ((length < 0) ? -length : length) + this->imgHeight / 2;
    } else if (this->datumtype.getValue() == RADIUS) {
        float length =  this->param1.getValue();
        offsetX = length + (p2-p1).length();
        offsetY = 0;

        float margin = 0.02f;
//         this->bbx = length + this->imgWidth / 2 + margin;
//         this->bby = this->imgHeight;
    }
    // Primitive Shape is only for text as this should only be selectable
    SoPrimitiveVertex pv;

    this->beginShape(action, QUADS);

    pv.setNormal( SbVec3f(0.f, 0.f, 1.f) );

    // Set coordinates
    pv.setPoint( SbVec3f(offsetX - this->imgWidth / 2, offsetY + (this->imgHeight / 2), 0.f) );
    shapeVertex(&pv);

    pv.setPoint( SbVec3f(offsetX - this->imgWidth / 2, offsetY - (this->imgHeight/ 2), 0.f) );
    shapeVertex(&pv);

    pv.setPoint( SbVec3f(offsetX + this->imgWidth / 2, offsetY - (this->imgHeight / 2), 0.f) );
    shapeVertex(&pv);
    
    pv.setPoint( SbVec3f(offsetX + this->imgWidth / 2, offsetY  + (this->imgHeight / 2), 0.f) );
    shapeVertex(&pv);

    this->endShape();
}

void SoDatumLabel::GLRender(SoGLRenderAction * action)
{
    SoState *state = action->getState();
    if (!shouldGLRender(action))
        return;
    if (action->handleTransparency(true))
      return;
    drawImage();

    SbVec2s size;
    int nc;
    const unsigned char * dataptr = this->image.getValue(size, nc);
    if (dataptr == NULL) return; // no image

    int srcw = size[0];
    int srch = size[1];

      // Create the quad to hold texture
    const SbViewVolume & vv = SoViewVolumeElement::get(state);
    float scale = vv.getWorldToScreenScale(SbVec3f(0.f,0.f,0.f), 0.4f);

    float aspectRatio =  (float) srcw / (float) srch;
    this->imgHeight = scale / (float) srch;
    this->imgWidth  = aspectRatio * (float) this->imgHeight;


    // Get the points stored
    const SbVec3f *pnts = this->pnts.getValues(0);
    SbVec3f p1 = pnts[0];
    SbVec3f p2 = pnts[1];

    state->push();

    // Position for Datum Text Label
    float offsetX, offsetY;

    if(this->datumtype.getValue() == DISTANCE || this->datumtype.getValue() == DISTANCEX || this->datumtype.getValue() == DISTANCEY )
    {
      // Datum Parameters
      float length =  this->param1.getValue();

      // Half of datum width
      float hw = (p2-p1).length() / 2;

      offsetX = 0;
      offsetY = length;

      //Line Parameters
      float margin = 0.02f;
      margin *= scale;

      // Calaculate and store bounding box
      this->bbx = hw * 2;
      this->bby = ((length < 0) ? -length : length) +  this->imgHeight / 2;

      glLineWidth(2.f);

      // Perp Lines
      glBegin(GL_LINES);
      glVertex2f(-hw, 0);
      glVertex2f(-hw, length + margin * ((length < 0) ? -1 : 1));
      glEnd();

      glBegin(GL_LINES);
      glVertex2f(hw, 0);
      glVertex2f(hw, length + margin * ((length < 0) ? -1 : 1));
      glEnd();

      //Parallel Lines
      glBegin(GL_LINES);
      glVertex2f(-hw, length);
      glVertex2f(-this->imgWidth / 2 - margin, length);
      glEnd();

      glBegin(GL_LINES);
      glVertex2f(hw, length);
      glVertex2f(this->imgWidth / 2 + margin, length);
      glEnd();

      //Draw some pretty arrowheads (Equilateral) (Eventually could be improved to other shapes?)
      glBegin(GL_TRIANGLES);
        glVertex2f(-hw, length);
        glVertex2f(-hw + 0.866 * margin, length + margin / 2);
        glVertex2f(-hw + 0.866 * margin, length - margin / 2);
      glEnd();

      glBegin(GL_TRIANGLES);
        glVertex2f(hw, length);
        glVertex2f(hw - 0.866 * margin, length + margin / 2);
        glVertex2f(hw - 0.866 * margin, length - margin / 2);
      glEnd();
    } else if (this->datumtype.getValue() == RADIUS) {

      float dist1 = (p2-p1).length();
      float length = this->param1.getValue();

      offsetX = dist1 + length;
      offsetY = 0;

      //Line Parameters
      float margin = 0.01f;
      margin *= scale;

      float dist2 = offsetX + this->imgWidth / 2 + margin * 8;

      // Calaculate and store bounding box
      this->bbx = (dist2 > dist1) ? dist2 : dist1;
      this->bby = this->imgHeight;

      glLineWidth(2.f);

      glBegin(GL_LINES);
      glVertex2f(0.f, 0.f);
      glVertex2f(offsetX - ( this->imgWidth / 2 + margin) , 0.f);
      glEnd();

      glBegin(GL_LINES);
      glVertex2f(offsetX + ( this->imgWidth / 2 + margin), 0.f);
      glVertex2f((dist2 > dist1) ? dist2 : dist1, 0.f );
      glEnd();

      //Draw a pretty arrowhead (Equilateral) (Eventually could be improved to other shapes?)
      glBegin(GL_TRIANGLES);
        glVertex2f(dist1, 0);
        glVertex2f(dist1 - 0.866 * 2 * margin,  margin);
        glVertex2f(dist1 - 0.866 * 2 * margin, -margin);
      glEnd();
    }

    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D); // Enable Textures
    glEnable(GL_BLEND); 
    // Copy the text bitmap into memory and bind
    GLuint myTexture;
    // generate a texture
    glGenTextures(1, &myTexture);

    glBindTexture(GL_TEXTURE_2D, myTexture);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, nc, srcw, srch, 0, GL_RGBA, GL_UNSIGNED_BYTE,(const GLvoid*)  dataptr);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glColor3f(1.f, 1.f, 1.f);
    glTexCoord2f(0.f, 1.f); glVertex2f(offsetX - this->imgWidth / 2, offsetY + this->imgHeight / 2);
    glTexCoord2f(0.f, 0.f); glVertex2f(offsetX - this->imgWidth / 2, offsetY - this->imgHeight / 2);
    glTexCoord2f(1.f, 0.f); glVertex2f(offsetX + this->imgWidth / 2, offsetY - this->imgHeight / 2);
    glTexCoord2f(1.f, 1.f); glVertex2f(offsetX + this->imgWidth / 2, offsetY + this->imgHeight / 2);

    glEnd();

    // Reset the Mode

    glPopAttrib();
    state->pop();

}
