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

//     this->bbx = 0;
//     this->bby = 0;
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
    }


    if(this->datumtype.getValue() == DISTANCE || this->datumtype.getValue() == DISTANCEX || this->datumtype.getValue() == DISTANCEY ){
        box.setBounds(this->bbox.getMin(),this->bbox.getMax() );
        SbVec3f center = this->bbox.getCenter();
        center.setValue(center[0], center[1], center[2]);
    } else if (this->datumtype.getValue() == RADIUS) {
        box.setBounds(this->bbox.getMin(),this->bbox.getMax() );
        SbVec3f center = this->bbox.getCenter();
        center.setValue(center[0], center[1], center[2]);
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

    } 
    // Get the points stored
    const SbVec3f *pnts = this->pnts.getValues(0);
    SbVec3f p1 = pnts[0];
    SbVec3f p2 = pnts[1];

    float offsetX, offsetY;

    // Change the offset and bounding box parameters depending on Datum Type
    if(this->datumtype.getValue() == DISTANCE || this->datumtype.getValue() == DISTANCEX || this->datumtype.getValue() == DISTANCEY ){

        float length = this->param1.getValue();
        SbVec3f dir, norm;
        if (this->datumtype.getValue() == DISTANCE) {
            dir = (p2-p1);
        } else if (this->datumtype.getValue() == DISTANCEX) {
            dir = SbVec3f( (p2[0] - p1[0] >= FLT_EPSILON) ? 1 : -1, 0, 0);
        } else if (this->datumtype.getValue() == DISTANCEY) {
            dir = SbVec3f(0, (p2[1] - p1[1] >= FLT_EPSILON) ? 1 : -1, 0);
        }

        dir.normalize();
        norm = SbVec3f (-dir[1],dir[0],0);

        float normproj12 = (p2-p1).dot(norm);
        SbVec3f p1_ = p1 + normproj12 * norm;

        SbVec3f midpos = (p1_ + p2)/2;
        // Get magnitude of angle between horizontal
        float angle = atan2f(dir[1],dir[0]);

        SbVec3f img1 = SbVec3f(-this->imgWidth / 2, -this->imgHeight / 2, 0.f);
        SbVec3f img2 = SbVec3f(-this->imgWidth / 2,  this->imgHeight / 2, 0.f);
        SbVec3f img3 = SbVec3f( this->imgWidth / 2, -this->imgHeight / 2, 0.f);
        SbVec3f img4 = SbVec3f( this->imgWidth / 2,  this->imgHeight / 2, 0.f);

        // Rotate thru Angle
        float s = sin(angle);
        float c = cos(angle);

        img1 = SbVec3f((img1[0] * c) - (img1[1] * s), (img1[0] * s) + (img1[1] * c), 0.f);

        img2 = SbVec3f((img2[0] * c) - (img2[1] * s), (img2[0] * s) + (img2[1] * c), 0.f);
 
        img3 = SbVec3f((img3[0] * c) - (img3[1] * s), (img3[0] * s) + (img3[1] * c), 0.f);

        img4 = SbVec3f((img4[0] * c) - (img4[1] * s), (img4[0] * s) + (img4[1] * c), 0.f);

        SbVec3f textOffset = midpos + norm * length;

        img1 += textOffset;
        img2 += textOffset;
        img3 += textOffset;
        img4 += textOffset;

        // Primitive Shape is only for text as this should only be selectable
        SoPrimitiveVertex pv;

        this->beginShape(action, QUADS);

        pv.setNormal( SbVec3f(0.f, 0.f, 1.f) );

        // Set coordinates
        pv.setPoint( img1 );
        shapeVertex(&pv);

        pv.setPoint( img2 );
        shapeVertex(&pv);

        pv.setPoint( img3 );
        shapeVertex(&pv);

        pv.setPoint( img4 );
        shapeVertex(&pv);

        this->endShape();

    } else if (this->datumtype.getValue() == RADIUS) {

        SbVec3f dir = (p2-p1);
        dir.normalize();
        SbVec3f norm (-dir[1],dir[0],0);

        float length = this->param1.getValue();
        SbVec3f pos = p2 + length*dir;

        float angle = atan2f(dir[1],dir[0]);

        SbVec3f img1 = SbVec3f(-this->imgWidth / 2, -this->imgHeight / 2, 0.f);
        SbVec3f img2 = SbVec3f(-this->imgWidth / 2,  this->imgHeight / 2, 0.f);
        SbVec3f img3 = SbVec3f( this->imgWidth / 2, -this->imgHeight / 2, 0.f);
        SbVec3f img4 = SbVec3f( this->imgWidth / 2,  this->imgHeight / 2, 0.f);

        // Rotate thru Angle
        float s = sin(angle);
        float c = cos(angle);

        img1 = SbVec3f((img1[0] * c) - (img1[1] * s), (img1[0] * s) + (img1[1] * c), 0.f);

        img2 = SbVec3f((img2[0] * c) - (img2[1] * s), (img2[0] * s) + (img2[1] * c), 0.f);

        img3 = SbVec3f((img3[0] * c) - (img3[1] * s), (img3[0] * s) + (img3[1] * c), 0.f);

        img4 = SbVec3f((img4[0] * c) - (img4[1] * s), (img4[0] * s) + (img4[1] * c), 0.f);

        SbVec3f textOffset = pos;

        img1 += textOffset;
        img2 += textOffset;
        img3 += textOffset;
        img4 += textOffset;

        // Primitive Shape is only for text as this should only be selectable
        SoPrimitiveVertex pv;

        this->beginShape(action, QUADS);

        pv.setNormal( SbVec3f(0.f, 0.f, 1.f) );

        // Set coordinates
        pv.setPoint( img1 );
        shapeVertex(&pv);

        pv.setPoint( img2 );
        shapeVertex(&pv);

        pv.setPoint( img3 );
        shapeVertex(&pv);

        pv.setPoint( img4 );
        shapeVertex(&pv);

        this->endShape();
    }

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
    float offsetX, offsetY, angle;

    SbVec3f textOffset;
    if(this->datumtype.getValue() == DISTANCE || this->datumtype.getValue() == DISTANCEX || this->datumtype.getValue() == DISTANCEY )
        {
        float length = this->param1.getValue();
        const SbVec3f *pnts = this->pnts.getValues(0);
        SbVec3f p1 = pnts[0];
        SbVec3f p2 = pnts[1];

        SbVec3f dir, norm;
        if (this->datumtype.getValue() == DISTANCE) {
            dir = (p2-p1);
        } else if (this->datumtype.getValue() == DISTANCEX) {
            dir = SbVec3f( (p2[0] - p1[0] >= FLT_EPSILON) ? 1 : -1, 0, 0);
        } else if (this->datumtype.getValue() == DISTANCEY) {
            dir = SbVec3f(0, (p2[1] - p1[1] >= FLT_EPSILON) ? 1 : -1, 0);
        }

        dir.normalize();
        norm = SbVec3f (-dir[1],dir[0],0);

        // when the datum line is not parallel to p1-p2 the projection of
        // p1-p2 on norm is not zero, p2 is considered as reference and p1
        // is replaced by its projection p1_
        float normproj12 = (p2-p1).dot(norm);
        SbVec3f p1_ = p1 + normproj12 * norm;

        SbVec3f midpos = (p1_ + p2)/2;

        float offset1 = (length + normproj12 < 0) ? -0.02  : 0.02;
        float offset2 = (length < 0) ? -0.02  : 0.02;

        // Get magnitude of angle between horizontal
        angle = atan2f(dir[1],dir[0]);
        bool flip=false;
        if (angle > M_PI_2+M_PI/12) {
            angle -= M_PI;
            flip = true;
        } else if (angle <= -M_PI_2+M_PI/12) {
            angle += M_PI;
            flip = true;
        }

        textOffset = midpos + norm * length;

        // Get the colour
        const SbColor& t = textColor.getValue();

        // Set GL Properties
        glLineWidth(2.f);
        glColor3f(t[0], t[1], t[2]);
        float margin = 0.01f;
        margin *= scale;

        SbVec3f perp1 = p1_ + norm * (length + offset1 * scale);
        SbVec3f perp2 = p2  + norm * (length + offset2 * scale);

        // Calculate the coordinates for the parallel datum lines
        SbVec3f par1 = p1_ + norm * length;
        SbVec3f par2 = midpos + norm * length - dir * (this->imgWidth / 2 + margin);
        SbVec3f par3 = midpos + norm * length + dir * (this->imgWidth / 2 + margin);
        SbVec3f par4 = p2  + norm * length;

        // Perp Lines
        glBegin(GL_LINES);
        glVertex2f(p1[0], p1[1]);
        glVertex2f(perp1[0], perp1[1]);
        glEnd();

        glBegin(GL_LINES);
        glVertex2f(p2[0], p2[1]);
        glVertex2f(perp2[0], perp2[1]);
        glEnd();

        glBegin(GL_LINES);
        glVertex2f(par1[0], par1[1]);
        glVertex2f(par2[0], par2[1]);
        glEnd();

        glBegin(GL_LINES);
        glVertex2f(par3[0], par3[1]);
        glVertex2f(par4[0], par4[1]);
        glEnd();

        SbVec3f ar1 = par1 + dir * 0.866 * 2 * margin;
        SbVec3f ar2 = ar1 + norm * margin;
                ar1 -= norm * margin;

        SbVec3f ar3 = par4 - dir * 0.866 * 2 * margin;
        SbVec3f ar4 = ar3 + norm * margin ;
                ar3 -= norm * margin;

        //Draw a pretty arrowhead (Equilateral) (Eventually could be improved to other shapes?)
        glBegin(GL_TRIANGLES);
          glVertex2f(par1[0], par1[1]);
          glVertex2f(ar1[0], ar1[1]);
          glVertex2f(ar2[0], ar2[1]);
        glEnd();

        glBegin(GL_TRIANGLES);
          glVertex2f(par4[0], par4[1]);
          glVertex2f(ar3[0], ar3[1]);
          glVertex2f(ar4[0], ar4[1]);
        glEnd();

        // BOUNDING BOX CALCULATION - IMPORTANT
        // Finds the mins and maxes
        std::vector<SbVec3f> corners;
        corners.push_back(p1);
        corners.push_back(p2);
        corners.push_back(perp1);
        corners.push_back(perp2);

        float minX = p1[0], minY = p1[1], maxX = p1[0] , maxY = p1[1];
        for (std::vector<SbVec3f>::iterator it=corners.begin(); it != corners.end(); ++it) {
            minX = ((*it)[0] < minX) ? (*it)[0] : minX;
            minY = ((*it)[1] < minY) ? (*it)[1] : minY;
            maxX = ((*it)[0] > maxX) ? (*it)[0] : maxX;
            maxY = ((*it)[1] > maxY) ? (*it)[1] : maxY;
        }
        //Store the bounding box
        this->bbox.setBounds(SbVec3f(minX, minY, 0.f), SbVec3f (maxX, maxY, 0.f));

    } else if (this->datumtype.getValue() == RADIUS) {
        SbVec3f dir = (p2-p1);
        dir.normalize();
        SbVec3f norm (-dir[1],dir[0],0);

        float length = this->param1.getValue();
        SbVec3f pos = p2 + length*dir;

        // Get magnitude of angle between horizontal
        angle = atan2f(dir[1],dir[0]);
        bool flip=false;
        if (angle > M_PI_2+M_PI/12) {
            angle -= M_PI;
            flip = true;
        } else if (angle <= -M_PI_2+M_PI/12) {
            angle += M_PI;
            flip = true;
        }

        textOffset = pos;

          // Get the colour
        const SbColor& t = textColor.getValue();

        // Set GL Properties
        glLineWidth(2.f);
        glColor3f(t[0], t[1], t[2]);

        float margin = 0.01f;
        margin *= scale;

        // Create the arrowhead
        SbVec3f ar0  = p2;
        SbVec3f ar1  = p2 - dir * 0.866 * 2 * margin;
        SbVec3f ar2  = ar1 + norm * margin;
                ar1 -= norm * margin;

        SbVec3f p3 = pos +  dir * (this->imgWidth / 2 + margin);
        if ((p3-p1).length() > (p2-p1).length())
        p2 = p3;

        // Calculate the points
        SbVec3f pnt1 = pos - dir * (margin + this->imgWidth / 2);
        SbVec3f pnt2 = pos + dir * (margin + this->imgWidth / 2);

        // Draw the Lines
        glBegin(GL_LINES);
        glVertex2f(p1[0], p1[1]);
        glVertex2f(pnt1[0], pnt1[1]);
        glEnd();

        glBegin(GL_LINES);
        glVertex2f(pnt2[0], pnt2[1]);
        glVertex2f(p2[0], p2[1]);
        glEnd();

        glBegin(GL_TRIANGLES);
          glVertex2f(ar0[0], ar0[1]);
          glVertex2f(ar1[0], ar1[1]);
          glVertex2f(ar2[0], ar2[1]);
        glEnd();
        
        // BOUNDING BOX CALCULATION - IMPORTANT
        // Finds the mins and maxes
        std::vector<SbVec3f> corners;
        corners.push_back(p1);
        corners.push_back(p2);
        corners.push_back(pnt1);
        corners.push_back(pnt2);

        float minX = p1[0], minY = p1[1], maxX = p1[0] , maxY = p1[1];
        for (std::vector<SbVec3f>::iterator it=corners.begin(); it != corners.end(); ++it) {
            minX = ((*it)[0] < minX) ? (*it)[0] : minX;
            minY = ((*it)[1] < minY) ? (*it)[1] : minY;
            maxX = ((*it)[0] > maxX) ? (*it)[0] : maxX;
            maxY = ((*it)[1] > maxY) ? (*it)[1] : maxY;
        }
        //Store the bounding box
        this->bbox.setBounds(SbVec3f(minX, minY, 0.f), SbVec3f (maxX, maxY, 0.f));

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

    // Apply a rotation and translation matrix
    glTranslatef(textOffset[0],textOffset[1], textOffset[2]);
    glRotatef((GLfloat) angle * 180 / M_PI, 0,0,1);
    glBegin(GL_QUADS);
    glColor3f(1.f, 1.f, 1.f);
    glTexCoord2f(0.f, 1.f); glVertex2f(-this->imgWidth / 2,  this->imgHeight / 2);
    glTexCoord2f(0.f, 0.f); glVertex2f(-this->imgWidth / 2, -this->imgHeight / 2);
    glTexCoord2f(1.f, 0.f); glVertex2f( this->imgWidth / 2, -this->imgHeight / 2);
    glTexCoord2f(1.f, 1.f); glVertex2f( this->imgWidth / 2,  this->imgHeight / 2);

    glEnd();

    // Reset the Mode

    glPopAttrib();
    state->pop();

}
