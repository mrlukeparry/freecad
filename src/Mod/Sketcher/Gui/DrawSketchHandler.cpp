/***************************************************************************
 *   Copyright (c) 2010 Jürgen Riegel <juergen.riegel@web.de>              *
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
# include <QPainter>
#endif

/// Here the FreeCAD includes sorted by Base,App,Gui......
#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Interpreter.h>
#include <Base/Vector3D.h>
#include <Gui/Application.h>
#include <Gui/BitmapFactory.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/Macro.h>
#include <Gui/MainWindow.h>
#include <Gui/View3DInventorViewer.h>
#include <Gui/View3DInventor.h>

#include <Mod/Part/App/Geometry.h>
#include <Mod/Sketcher/App/SketchObject.h>

#include "DrawSketchHandler.h"
#include "ViewProviderSketch.h"


using namespace SketcherGui;
using namespace Sketcher;

//**************************************************************************
// Construction/Destruction

DrawSketchHandler::DrawSketchHandler()
        : sketchgui(0)
{

}

DrawSketchHandler::~DrawSketchHandler()
{

}

void DrawSketchHandler::quit(void)
{
    assert(sketchgui);
    sketchgui->drawEdit(std::vector<Base::Vector2D>());
    clearHintLines();
    
    resetPositionText();

    unsetCursor();
    sketchgui->purgeHandler();
}

//**************************************************************************
// Helpers

int DrawSketchHandler::getHighestVertexIndex(void)
{
    return sketchgui->getSketchObject()->getHighestVertexIndex();
}

int DrawSketchHandler::getHighestCurveIndex(void)
{
    return sketchgui->getSketchObject()->getHighestCurveIndex();
}

void DrawSketchHandler::setCursor(const QPixmap &p,int x,int y)
{
    Gui::MDIView* view = Gui::getMainWindow()->activeWindow();
    if (view && view->isDerivedFrom(Gui::View3DInventor::getClassTypeId())) {
        Gui::View3DInventorViewer* viewer = static_cast<Gui::View3DInventor*>(view)->getViewer();

        oldCursor = viewer->getWidget()->cursor();
        QCursor cursor(p, x, y);
        actCursor = cursor;

        viewer->getWidget()->setCursor(cursor);
    }
}

void DrawSketchHandler::applyCursor(void)
{
    applyCursor(actCursor);
}

void DrawSketchHandler::applyCursor(QCursor &newCursor)
{
    Gui::MDIView* view = Gui::getMainWindow()->activeWindow();
    if (view && view->isDerivedFrom(Gui::View3DInventor::getClassTypeId())) {
        Gui::View3DInventorViewer* viewer = static_cast<Gui::View3DInventor*>(view)->getViewer();
        viewer->getWidget()->setCursor(newCursor);
    }
}

void DrawSketchHandler::unsetCursor(void)
{
    Gui::MDIView* view = Gui::getMainWindow()->activeWindow();
    if (view && view->isDerivedFrom(Gui::View3DInventor::getClassTypeId())) {
        Gui::View3DInventorViewer* viewer = static_cast<Gui::View3DInventor*>(view)->getViewer();
        viewer->getWidget()->setCursor(oldCursor);
    }
}

int DrawSketchHandler::seekAutoConstraint(std::vector<AutoConstraint> &suggestedConstraints,
                                          const Base::Vector2D& Pos, const Base::Vector2D& Dir, Type type)
{
    std::vector<AutoConstraint> prevConstr;
    // Save past previous constraints to use as reference
    if(suggestedConstraints.size() > 0)
      prevConstr = suggestedConstraints;
    suggestedConstraints.clear();

    if (!sketchgui->Autoconstraints.getValue())
        return 0; // If Autoconstraints property is not set quit

    // Get Preselection
    // Currently only considers objects in current Sketcher
    if (type == VERTEX && sketchgui->getPreselectPoint() != -1) {
        AutoConstraint coincident;
        coincident.Type       = Sketcher::Coincident;
        coincident.Index      = sketchgui->getPreselectPoint();
        suggestedConstraints.push_back(coincident);
    }
    else if (type == CURVE && sketchgui->getPreselectPoint() != -1) {
        AutoConstraint pointOnObject;
        pointOnObject.Type       = Sketcher::PointOnObject;
        pointOnObject.Index      = sketchgui->getPreselectPoint();
        suggestedConstraints.push_back(pointOnObject);
    }
    else if (type == VERTEX && sketchgui->getPreselectCurve() != -1) {
        AutoConstraint pointOnObject;
        pointOnObject.Type       = Sketcher::PointOnObject;
        pointOnObject.Index      = sketchgui->getPreselectCurve();
        suggestedConstraints.push_back(pointOnObject);
    }
    else if (type == CURVE && sketchgui->getPreselectCurve() != -1) {
        AutoConstraint tangent;
        tangent.Type       = Sketcher::Tangent;
        tangent.Index      = sketchgui->getPreselectCurve();
        suggestedConstraints.push_back(tangent);
    }

    if (Dir.Length() < 1)
        // Direction not set so return;
        return suggestedConstraints.size();

    // Suggest vertical and horizontal constraints

    // Number of Degree of deviation from horizontal or vertical lines
    const double angleDev = 2;
    const double angleDevRad = angleDev *  M_PI / 180.;

    double angle = std::abs(atan2(Dir.fY, Dir.fX));
    if (angle < angleDevRad || (M_PI - angle) < angleDevRad ) {
        // Suggest horizontal constraint
        AutoConstraint horConstr;
        horConstr.Index = -1;
        horConstr.Type = Horizontal;
        suggestedConstraints.push_back(horConstr);
    }
    else if (std::abs(angle - M_PI_2) < angleDevRad) {
        // Suggest vertical constraint
        AutoConstraint vertConstr;
        vertConstr.Index = -1;
        vertConstr.Type = Vertical;
        suggestedConstraints.push_back(vertConstr);
    }

    // Get geometry list
    const std::vector<Part::Geometry *> geomlist = sketchgui->getSketchObject()->getCompleteGeometry();

    //Store Index for later if geometry is connected to arc or curve
    int prevLastIndex = Constraint::GeoUndef;
    
    //Check if perpendicular to a lines, not calculated if previous constraint is vertical or horizontal
    if(prevConstr.size() > 0) {
        const AutoConstraint & lastConstr = prevConstr.back();
        prevLastIndex = (lastConstr.Index >= 0) ? lastConstr.Index : lastConstr.Index + geomlist.size();
        if( suggestedConstraints.size() > 0 &&
           (suggestedConstraints.back().Type == Sketcher::Horizontal || suggestedConstraints.back().Type == Sketcher::Vertical)) {
          //Do Nothing
        } else if(lastConstr.Type == Sketcher::PointOnObject && type == VERTEX) {
            // FIX ME - need to use GeoById somehow
            const Part::Geometry *geo = geomlist[(lastConstr.Index >= 0) ? lastConstr.Index : lastConstr.Index + geomlist.size() ];

            if (geo->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
                const Part::GeomLineSegment *lineSeg = dynamic_cast<const Part::GeomLineSegment *>(geo);
                Base::Vector3d p1, p2;
                p1 = lineSeg->getStartPoint();
                p2 = lineSeg->getEndPoint();
                Base::Vector3d dir = (p2-p1).Normalize();
                Base::Vector2D norm((float) -dir.y, (float) dir.x);

                // Find angle difference
                float angle = norm.GetAngle(Dir);
                if(angle > M_PI_2)
                  angle = std::abs(angle - M_PI);
                if (angle < angleDevRad) {
                    //Perpendicular Constraint
                      // Suggest vertical constraint
                    AutoConstraint perpConstr;
                    perpConstr.Index   = lastConstr.Index;
                    perpConstr.Type    = Perpendicular;
                    perpConstr.HintDir = norm;
                    suggestedConstraints.push_back(perpConstr);
                }
            } else if (geo->getTypeId() == Part::GeomCircle::getClassTypeId()) {

                const Part::GeomCircle *circle = dynamic_cast<const Part::GeomCircle *>(geo);
                Base::Vector3d center = circle->getCenter();
                Base::Vector2D pnt1 = Pos - Dir;
                Base::Vector2D pnt2((float) center.x, (float) center.y);

                float radius = (float) circle->getRadius();
                Base::Vector2D norm = pnt2 - pnt1;
                norm.Normalize();
                Base::Vector2D tang(-norm.fY, norm.fX);

                // Find angle difference
                float normAngle = norm.GetAngle(Dir);
                if(normAngle > M_PI_2)
                  normAngle = std::abs(normAngle - M_PI);

                // Find angle difference
                float tangAngle = tang.GetAngle(Dir);
                if(tangAngle > M_PI_2)
                  tangAngle = std::abs(tangAngle - M_PI);

                if (normAngle < angleDevRad) {
                    //Perpendicular Constraint
                    AutoConstraint perpConstr;
                    perpConstr.Index   = lastConstr.Index;
                    perpConstr.Type    = Perpendicular;
                    perpConstr.HintDir = norm;
                    suggestedConstraints.push_back(perpConstr);
                } else if (tangAngle < angleDevRad) {
                    //Tangent Constraint
                    // Direction vector
                    norm.Scale(radius);
                    AutoConstraint tangConstr;
                    tangConstr.Index   = lastConstr.Index;
                    tangConstr.Type    = Tangent;
                    tangConstr.HintDir = tang;
                    suggestedConstraints.push_back(tangConstr);
                }
            }
        }
    }

    // Find if there are tangent constraints (currently arcs and circles)
    // FIXME needs to consider when zooming out?
    const float tangDeviation = 2.;

    int tangId = Constraint::GeoUndef;
    float smlTangDist = 1e15f;
    
    // Iterate through geometry
    int i = 0;
    Base::Vector2D fndPnt(0.f,0.f);
    for (std::vector<Part::Geometry *>::const_iterator it=geomlist.begin(); it != geomlist.end(); ++it, i++) {

        if(prevLastIndex == i)
          continue; // This is the geometry that should be skipped

        // If the line is attached to a curve disable tangent finding
        if ((*it)->getTypeId() == Part::GeomCircle::getClassTypeId()) {
            const Part::GeomCircle *circle = dynamic_cast<const Part::GeomCircle *>((*it));

            Base::Vector3d center = circle->getCenter();
            Base::Vector3d tmpPos(Pos.fX, Pos.fY, 0.f);

            float radius = (float) circle->getRadius();

            Base::Vector3d projPnt(0.f, 0.f, 0.f);
            projPnt = projPnt.ProjToLine(center - tmpPos, Base::Vector3d(Dir.fX, Dir.fY));
            float projDist = projPnt.Length();

            if ( (projDist < radius + tangDeviation ) && (projDist > radius - tangDeviation)) {
                // Find if nearest
                if (projDist < smlTangDist) {
                    tangId = i; 
                    smlTangDist = projDist;

                    projPnt.Normalize();
                    projPnt = center + projPnt * radius;
                    fndPnt = Base::Vector2D((float) projPnt.x, (float) projPnt.y);
                }
            }

        } else if ((*it)->getTypeId() == Part::GeomArcOfCircle::getClassTypeId()) {
            const Part::GeomArcOfCircle *arc = dynamic_cast<const Part::GeomArcOfCircle *>((*it));

            Base::Vector3d center = arc->getCenter();
            double radius = arc->getRadius();

            Base::Vector3d projPnt(0.f, 0.f, 0.f);
            Base::Vector3d tmpPos(Pos.fX, Pos.fY, 0.f);

            projPnt = projPnt.ProjToLine(center - tmpPos, Base::Vector3d(Dir.fX, Dir.fY));
            float projDist = projPnt.Length();
            

            if ( projDist < radius + tangDeviation && projDist > radius - tangDeviation) {
                double startAngle, endAngle;
                arc->getRange(startAngle, endAngle);
                if(startAngle > 2 * M_PI)
                    startAngle -= 2 * M_PI;
                if(endAngle > 2 * M_PI)
                    endAngle -= 2 * M_PI;
                double angle = atan2(projPnt.y, projPnt.x);

                if(angle < 0)
                  angle = M_PI + std::abs(angle);

                bool flip = (startAngle > endAngle);
                projPnt += center;
                // if the pnt is on correct side of arc and find if nearest
                if ((projDist < smlTangDist) && ( (flip && angle < startAngle && angle < endAngle) || (!flip && angle > startAngle && angle < endAngle))) {
                    tangId = i;
                    smlTangDist = projDist;
                    projPnt -= center;
                    projPnt.Normalize();
                    projPnt = center + projPnt * radius;
                    fndPnt = Base::Vector2D((float) projPnt.x, (float) projPnt.y);
                }
            }
        }
    }

    if (tangId != Constraint::GeoUndef) {
        if (tangId > getHighestCurveIndex()) // external Geometry
            tangId = getHighestCurveIndex() - tangId;
        // Suggest vertical constraint
        AutoConstraint tangConstr;
        tangConstr.Index   = tangId;
        tangConstr.Type    = Tangent;
        tangConstr.HintPnt = fndPnt;
        suggestedConstraints.push_back(tangConstr);
    }

    return suggestedConstraints.size();
}

void DrawSketchHandler::createAutoConstraints(const std::vector<AutoConstraint> &autoConstrs,
                                              int geoId1, Sketcher::PointPos posId1)
{
    if (!sketchgui->Autoconstraints.getValue())
        return; // If Autoconstraints property is not set quit

    if (autoConstrs.size() > 0) {
        // Open the Command
        Gui::Command::openCommand("Add auto constraints");

        // Iterate through constraints
        std::vector<AutoConstraint>::const_iterator it = autoConstrs.begin();
        for (; it != autoConstrs.end(); ++it) {
            switch (it->Type)
            {
            case Sketcher::Coincident: {
                if (posId1 == Sketcher::none)
                    continue;
                // If the auto constraint has a point create a coincident otherwise it is an edge on a point
                Sketcher::PointPos posId2;
                int geoId2;
                sketchgui->getSketchObject()->getGeoVertexIndex(it->Index, geoId2, posId2);

                Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Coincident',%i,%i,%i,%i)) "
                                        ,sketchgui->getObject()->getNameInDocument()
                                        ,geoId1, posId1, geoId2, posId2
                                        );
                } break;
            case Sketcher::PointOnObject: {
                int index = it->Index;
                if (posId1 == Sketcher::none) {
                    // Auto constraining an edge so swap parameters
                    index = geoId1;
                    sketchgui->getSketchObject()->getGeoVertexIndex(it->Index, geoId1, posId1);
                }

                Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('PointOnObject',%i,%i,%i)) "
                                        ,sketchgui->getObject()->getNameInDocument()
                                        ,geoId1, posId1, index
                                       );
                } break;
            case Sketcher::Horizontal: {
                Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Horizontal',%i)) "
                                        ,sketchgui->getObject()->getNameInDocument()
                                        ,geoId1
                                       );
                } break;
            case Sketcher::Vertical: {
                Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Vertical',%i)) "
                                        ,sketchgui->getObject()->getNameInDocument()
                                        ,geoId1
                                       );
                } break;
            case Sketcher::Tangent: {
                Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Tangent',%i, %i)) "
                                        ,sketchgui->getObject()->getNameInDocument()
                                        ,geoId1, it->Index
                                       );
                } break;
            case Sketcher::Perpendicular: {
                Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Perpendicular',%i, %i)) "
                                        ,sketchgui->getObject()->getNameInDocument()
                                        ,geoId1, it->Index
                                       );
                } break;
            default:
              continue;
            }

            Gui::Command::commitCommand();
            Gui::Command::updateActive();
        }
    }
}

void DrawSketchHandler::renderHintLines(std::vector<AutoConstraint> &suggestedConstraints, Base::Vector2D &pnt)
{

      std::vector<Base::Vector2D> pnts;    // Iterate through AutoConstraints type
      if(suggestedConstraints.size() > 0) {
          std::vector<AutoConstraint>::iterator it=suggestedConstraints.begin();
          for (; it != suggestedConstraints.end(); ++it) {
            switch (it->Type)
            {
            case Horizontal: {
                Base::Vector2D pnt2(pnt.fX + 1.f, pnt.fY);
                pnts.push_back(pnt);
                pnts.push_back(pnt2);
            } break;
            case Vertical: {
                Base::Vector2D pnt2(pnt.fX, pnt.fY + 1.f);
                pnts.push_back(pnt);
                pnts.push_back(pnt2);
            } break;
            case Perpendicular: {
                // HintPnt is actually a norm direction.
                Base::Vector2D pnt2, revNorm, norm;
                bool useDir = (it->HintDir.fX != 0.f && it->HintDir.fY != 0.f);
                pnt2 = (useDir) ? pnt + it->HintDir : it->HintPnt;
                pnts.push_back(pnt);
                pnts.push_back(pnt2);

                norm = pnt2 - pnt;
                revNorm = Base::Vector2D(-norm.fY, norm.fX);
                revNorm.Normalize();
                pnts.push_back(pnt);
                pnts.push_back(pnt+revNorm);
            } break;
            case Tangent: {
                bool useDir = (it->HintDir.fX != 0.f && it->HintDir.fY != 0.f);
                pnts.push_back(pnt);
                pnts.push_back((useDir) ? pnt + it->HintDir : it->HintPnt);
            } break;
            
            default:
                pnts.clear();
                break;
            }
        }
      }
    sketchgui->drawHintLines(pnts);
}

void DrawSketchHandler::clearHintLines()
{
    std::vector<Base::Vector2D> pnts;
    sketchgui->drawHintLines(pnts);
}

void DrawSketchHandler::renderSuggestConstraintsCursor(std::vector<AutoConstraint> &suggestedConstraints)
{
    // Auto Constrait icon size in px
    int iconSize = 16;

    // Create a pixmap that will contain icon and each autoconstraint icon
    QPixmap baseIcon = actCursor.pixmap();
    QPixmap newIcon(baseIcon.width() + suggestedConstraints.size() * iconSize,
                    baseIcon.height());
    newIcon.fill(Qt::transparent);

    QPainter qp;
    qp.begin(&newIcon);

    qp.drawPixmap(0,0, baseIcon);

    // Iterate through AutoConstraints type and add icons to the cursor pixmap
    std::vector<AutoConstraint>::iterator it=suggestedConstraints.begin();
    int i = 0;
    for (; it != suggestedConstraints.end(); ++it, i++) {
        QString iconType;
        switch (it->Type)
        {
        case Horizontal:
            iconType = QString::fromAscii("Constraint_Horizontal");
            break;
        case Perpendicular:
            iconType = QString::fromAscii("Constraint_Perpendicular");
            break;
        case Vertical:
            iconType = QString::fromAscii("Constraint_Vertical");
            break;
        case Coincident:
            iconType = QString::fromAscii("Constraint_PointOnPoint");
            break;
        case PointOnObject:
            iconType = QString::fromAscii("Constraint_PointOnObject");
            break;
        case Tangent:
            iconType = QString::fromAscii("Constraint_Tangent");
            break;
        default:
            break;
        }

        QPixmap icon = Gui::BitmapFactory().pixmap(iconType.toAscii()).scaledToWidth(iconSize);
        qp.drawPixmap(QPoint(baseIcon.width() + i * iconSize, baseIcon.height() - iconSize), icon);
    }

    qp.end(); // Finish painting

    // Create the new cursor with the icon.
    QPoint p=actCursor.hotSpot();
    QCursor newCursor(newIcon, p.x(), p.y());
    applyCursor(newCursor);
}

void DrawSketchHandler::setPositionText(const Base::Vector2D &Pos, const std::string &text)
{
    sketchgui->setPositionText(Pos, text);
}


void DrawSketchHandler::setPositionText(const Base::Vector2D &Pos)
{
    sketchgui->setPositionText(Pos);
}

void DrawSketchHandler::resetPositionText(void)
{
    sketchgui->resetPositionText();
}
