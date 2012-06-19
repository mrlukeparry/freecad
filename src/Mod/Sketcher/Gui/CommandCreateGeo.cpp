/***************************************************************************
 *   Copyright (c) 2010 Jürgen Riegel (juergen.riegel@web.de)              *
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
#endif

#include <Base/Console.h>

#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/MainWindow.h>
#include <Gui/DlgEditFileIncludeProptertyExternal.h>
#include <Gui/Selection.h>
#include <Gui/SelectionFilter.h>
#include <Gui/SoFCUnifiedSelection.h>
#include <Gui/View3DInventor.h>
#include <Gui/View3DInventorViewer.h>

#include <Inventor/events/SoKeyboardEvent.h>

#include <Mod/Sketcher/App/SketchObject.h>

#include "ViewProviderSketch.h"
#include "DrawSketchHandler.h"

using namespace std;
using namespace SketcherGui;

/* helper functions ======================================================*/

void ActivateHandler(Gui::Document *doc,DrawSketchHandler *handler)
{
    if (doc) {
        if (doc->getInEdit() && doc->getInEdit()->isDerivedFrom
            (SketcherGui::ViewProviderSketch::getClassTypeId()))
            dynamic_cast<SketcherGui::ViewProviderSketch*>
            (doc->getInEdit())->activateHandler(handler);
    }
}

bool isCreateGeoActive(Gui::Document *doc)
{
    if (doc) {
        // checks if a Sketch Viewprovider is in Edit and is in no special mode
        if (doc->getInEdit() && doc->getInEdit()->isDerivedFrom
            (SketcherGui::ViewProviderSketch::getClassTypeId())) {
            if (dynamic_cast<SketcherGui::ViewProviderSketch*>(doc->getInEdit())->
                getSketchMode() == ViewProviderSketch::STATUS_NONE)
                return true;
        }
    }
    return false;
}

SketcherGui::ViewProviderSketch* getSketchViewprovider(Gui::Document *doc)
{
    if (doc) {
        if (doc->getInEdit() && doc->getInEdit()->isDerivedFrom
            (SketcherGui::ViewProviderSketch::getClassTypeId()) )
            return dynamic_cast<SketcherGui::ViewProviderSketch*>(doc->getInEdit());
    }
    return 0;
}

/* Sketch commands =======================================================*/

/* XPM */
static const char *cursor_createline[]={
"32 32 3 1",
"+ c white",
"# c red",
". c None",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"................................",
"+++++...+++++...................",
"................................",
"......+...............###.......",
"......+...............#.#.......",
"......+...............###.......",
"......+..............#..........",
"......+.............#...........",
"....................#...........",
"...................#............",
"..................#.............",
"..................#.............",
".................#..............",
"................#...............",
"................#...............",
"...............#................",
"..............#.................",
"..............#.................",
".............#..................",
"..........###...................",
"..........#.#...................",
"..........###...................",
"................................",
"................................",
"................................",
"................................",
"................................"};

class DrawSketchHandlerLine: public DrawSketchHandler
{
public:
    DrawSketchHandlerLine():Mode(STATUS_SEEK_First),EditCurve(2){}
    virtual ~DrawSketchHandlerLine(){}
    /// mode table
    enum SelectMode {
        STATUS_SEEK_First,      /**< enum value ----. */
        STATUS_SEEK_Second,     /**< enum value ----. */
        STATUS_End
    };

    virtual void activated(ViewProviderSketch *sketchgui)
    {
        setCursor(QPixmap(cursor_createline),7,7);
    }

    virtual void mouseMove(Base::Vector2D onSketchPos)
    {
        setPositionText(onSketchPos);

        if (Mode==STATUS_SEEK_First) {
            if (seekAutoConstraint(sugConstr1, onSketchPos, Base::Vector2D(0.f,0.f))) {
                renderSuggestConstraintsCursor(sugConstr1);
                return;
            }
        }
        else if (Mode==STATUS_SEEK_Second){
            EditCurve[1] = onSketchPos;
            sketchgui->drawEdit(EditCurve);
            sugConstr2 = sugConstr1; // Copy the previously found constraints
            if (seekAutoConstraint(sugConstr2, onSketchPos, onSketchPos - EditCurve[0])) {
                renderSuggestConstraintsCursor(sugConstr2);
                renderHintLines(sugConstr2, EditCurve[0]);
                return;
            } else
              clearHintLines();
        }
        applyCursor();
    }

    virtual bool pressButton(Base::Vector2D onSketchPos)
    {
        if (Mode==STATUS_SEEK_First){
            EditCurve[0] = onSketchPos;
            Mode = STATUS_SEEK_Second;
        }
        else {
            EditCurve[1] = onSketchPos;
            sketchgui->drawEdit(EditCurve);
            Mode = STATUS_End;
        }
        return true;
    }

    virtual bool releaseButton(Base::Vector2D onSketchPos)
    {
        if (Mode==STATUS_End){
            unsetCursor();
            resetPositionText();

            clearHintLines();
            EditCurve.clear();
            sketchgui->drawEdit(EditCurve);

            Gui::Command::openCommand("Add sketch line");
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addGeometry(Part.Line(App.Vector(%f,%f,0),App.Vector(%f,%f,0)))",
                      sketchgui->getObject()->getNameInDocument(),
                      EditCurve[0].fX,EditCurve[0].fY,EditCurve[1].fX,EditCurve[1].fY);
            Gui::Command::commitCommand();
            Gui::Command::updateActive();

            // add auto constraints for the line segment start
            if (sugConstr1.size() > 0) {
                createAutoConstraints(sugConstr1, getHighestCurveIndex(), Sketcher::start);
                sugConstr1.clear();
            }

            // add auto constraints for the line segment end
            if (sugConstr2.size() > 0) {
                createAutoConstraints(sugConstr2, getHighestCurveIndex(), Sketcher::end);
                sugConstr2.clear();
            }

            sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider
        }
        return true;
    }
protected:
    SelectMode Mode;
    std::vector<Base::Vector2D> EditCurve;
    std::vector<AutoConstraint> sugConstr1, sugConstr2;
};



DEF_STD_CMD_A(CmdSketcherCreateLine);

CmdSketcherCreateLine::CmdSketcherCreateLine()
  : Command("Sketcher_CreateLine")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Create line");
    sToolTipText    = QT_TR_NOOP("Create a line in the sketch");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_CreateLine";
    sAccel          = "L";
    eType           = ForEdit;
}

void CmdSketcherCreateLine::activated(int iMsg)
{
    ActivateHandler(getActiveGuiDocument(),new DrawSketchHandlerLine() );
}

bool CmdSketcherCreateLine::isActive(void)
{
    return isCreateGeoActive(getActiveGuiDocument());
}


/* Create Box =======================================================*/

/* XPM */
static const char *cursor_createbox[]={
"32 32 3 1",
"+ c white",
"# c red",
". c None",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"................................",
"+++++...+++++...................",
"................................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"................................",
"................................",
"..........................###...",
"...........################.#...",
"...........#..............###...",
"...........#...............#....",
"...........#...............#....",
"...........#...............#....",
"...........#...............#....",
"...........#...............#....",
"...........#...............#....",
"..........###..............#....",
"..........#.################....",
"..........###...................",
"................................",
"................................",
"................................",
"................................",
"................................"};

class DrawSketchHandlerBox: public DrawSketchHandler
{
public:
    DrawSketchHandlerBox():Mode(STATUS_SEEK_First),EditCurve(5){}
    virtual ~DrawSketchHandlerBox(){}
    /// mode table
    enum BoxMode {
        STATUS_SEEK_First,      /**< enum value ----. */
        STATUS_SEEK_Second,     /**< enum value ----. */
        STATUS_End
    };

    virtual void activated(ViewProviderSketch *sketchgui)
    {
        setCursor(QPixmap(cursor_createbox),7,7);
    }

    virtual void mouseMove(Base::Vector2D onSketchPos)
    {
        setPositionText(onSketchPos);

        if (Mode==STATUS_SEEK_First) {
            if (seekAutoConstraint(sugConstr1, onSketchPos, Base::Vector2D(0.f,0.f))) {
                renderSuggestConstraintsCursor(sugConstr1);
                return;
            }
        }
        else if (Mode==STATUS_SEEK_Second) {
            EditCurve[2] = onSketchPos;
            EditCurve[1] = Base::Vector2D(onSketchPos.fX ,EditCurve[0].fY);
            EditCurve[3] = Base::Vector2D(EditCurve[0].fX,onSketchPos.fY);
            sketchgui->drawEdit(EditCurve);

            if (seekAutoConstraint(sugConstr2, onSketchPos, Base::Vector2D(0.f,0.f))) {
                renderSuggestConstraintsCursor(sugConstr2);
                return;
            }
        }
        applyCursor();
    }

    virtual bool pressButton(Base::Vector2D onSketchPos)
    {
        if (Mode==STATUS_SEEK_First){
            EditCurve[0] = onSketchPos;
            EditCurve[4] = onSketchPos;
            Mode = STATUS_SEEK_Second;
        }
        else {
            EditCurve[2] = onSketchPos;
            EditCurve[1] = Base::Vector2D(onSketchPos.fX ,EditCurve[0].fY);
            EditCurve[3] = Base::Vector2D(EditCurve[0].fX,onSketchPos.fY);
            sketchgui->drawEdit(EditCurve);
            Mode = STATUS_End;
        }
        return true;
    }

    virtual bool releaseButton(Base::Vector2D onSketchPos)
    {
        if (Mode==STATUS_End){
            unsetCursor();
            resetPositionText();
            Gui::Command::openCommand("Add sketch box");
            int firstCurve = getHighestCurveIndex() + 1;
            // add the four line geos
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addGeometry(Part.Line(App.Vector(%f,%f,0),App.Vector(%f,%f,0)))",
                      sketchgui->getObject()->getNameInDocument(),
                      EditCurve[0].fX,EditCurve[0].fY,EditCurve[1].fX,EditCurve[1].fY);
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addGeometry(Part.Line(App.Vector(%f,%f,0),App.Vector(%f,%f,0)))",
                      sketchgui->getObject()->getNameInDocument(),
                      EditCurve[1].fX,EditCurve[1].fY,EditCurve[2].fX,EditCurve[2].fY);
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addGeometry(Part.Line(App.Vector(%f,%f,0),App.Vector(%f,%f,0)))",
                      sketchgui->getObject()->getNameInDocument(),
                      EditCurve[2].fX,EditCurve[2].fY,EditCurve[3].fX,EditCurve[3].fY);
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addGeometry(Part.Line(App.Vector(%f,%f,0),App.Vector(%f,%f,0)))",
                      sketchgui->getObject()->getNameInDocument(),
                      EditCurve[3].fX,EditCurve[3].fY,EditCurve[0].fX,EditCurve[0].fY);
            // add the four coincidents to ty them together
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Coincident',%i,2,%i,1)) "
                     ,sketchgui->getObject()->getNameInDocument()
                     ,firstCurve,firstCurve+1);
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Coincident',%i,2,%i,1)) "
                     ,sketchgui->getObject()->getNameInDocument()
                     ,firstCurve+1,firstCurve+2);
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Coincident',%i,2,%i,1)) "
                     ,sketchgui->getObject()->getNameInDocument()
                     ,firstCurve+2,firstCurve+3);
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Coincident',%i,2,%i,1)) "
                     ,sketchgui->getObject()->getNameInDocument()
                     ,firstCurve+3,firstCurve);
            // add the horizontal constraints
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Horizontal',%i)) "
                     ,sketchgui->getObject()->getNameInDocument()
                     ,firstCurve);
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Horizontal',%i)) "
                     ,sketchgui->getObject()->getNameInDocument()
                     ,firstCurve+2);
            // add the vertical constraints
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Vertical',%i)) "
                     ,sketchgui->getObject()->getNameInDocument()
                     ,firstCurve+1);
            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Vertical',%i)) "
                     ,sketchgui->getObject()->getNameInDocument()
                     ,firstCurve+3);

            Gui::Command::commitCommand();
            Gui::Command::updateActive();

            // add auto constraints at the start of the first side
            if (sugConstr1.size() > 0) {
                createAutoConstraints(sugConstr1, getHighestCurveIndex() - 3 , Sketcher::start);
                sugConstr1.clear();
            }

            // add auto constraints at the end of the second side
            if (sugConstr2.size() > 0) {
                createAutoConstraints(sugConstr2, getHighestCurveIndex() - 2, Sketcher::end);
                sugConstr2.clear();
            }

            EditCurve.clear();
            sketchgui->drawEdit(EditCurve);
            sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider
        }
        return true;
    }
protected:
    BoxMode Mode;
    std::vector<Base::Vector2D> EditCurve;
    std::vector<AutoConstraint> sugConstr1, sugConstr2;
};



DEF_STD_CMD_A(CmdSketcherCreateRectangle);

CmdSketcherCreateRectangle::CmdSketcherCreateRectangle()
  : Command("Sketcher_CreateRectangle")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Create rectangle");
    sToolTipText    = QT_TR_NOOP("Create a rectangle in the sketch");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_CreateRectangle";
    sAccel          = "R";
    eType           = ForEdit;
}

void CmdSketcherCreateRectangle::activated(int iMsg)
{
    ActivateHandler(getActiveGuiDocument(),new DrawSketchHandlerBox() );
}

bool CmdSketcherCreateRectangle::isActive(void)
{
    return isCreateGeoActive(getActiveGuiDocument());
}


// ======================================================================================

/* XPM */
static const char *cursor_createlineset[]={
"32 32 3 1",
"+ c white",
"# c red",
". c None",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"................................",
"+++++...+++++...................",
"................................",
"......+...............###.......",
"......+...............#.#.......",
"......+...............###.......",
"......+..............#..#.......",
"......+.............#....#......",
"....................#....#......",
"...................#......#.....",
"..................#.......#.....",
"..................#........#....",
".................#.........#....",
"................#..........###..",
"................#..........#.#..",
"......#........#...........###..",
".......#......#.................",
"........#.....#.................",
".........#...#..................",
"..........###...................",
"..........#.#...................",
"..........###...................",
"................................",
"................................",
"................................",
"................................",
"................................"};

class DrawSketchHandlerLineSet: public DrawSketchHandler
{
public:
    DrawSketchHandlerLineSet()
      : Mode(STATUS_LINE_SEEK_First),EditCurve(2),firstPoint(-1),previousCurve(-1),LineMode(LINE_MODE_Line) {
        keyArcValue=(int)SoKeyboardEvent::A;
      }
    virtual ~DrawSketchHandlerLineSet(){}
    /// mode table
    enum SelectMode {
        //----------------
        STATUS_LINE_SEEK_First,      /**< enum value ----. */
        STATUS_LINE_SEEK_Second,     /**< enum value ----. */
        STATUS_LINE_Do,
        STATUS_LINE_Close,
        //---------------
        STATUS_ARC_SEEK_First,
        STATUS_ARC_SEEK_Second, // Arc state only has two Seek states since center is calculated
        STATUS_ARC_Do,
        STATUS_ARC_Close,
        //---------------
        STATUS_TAN_LINE_SEEK_First,  //TANGENT LINE FOR WHEN PREVIOUS ELEMENT IS AN ARC
        STATUS_TAN_LINE_SEEK_Second,
        STATUS_TAN_LINE_Do,
        STATUS_TAN_LINE_Close
    };

    enum SelectLineMode
    {
        LINE_MODE_Arc,
        LINE_MODE_Line
    };

    enum ElementType
    {
        E_STRAIGHT_LINE,
        E_ARC_CW, // From the perspective of the user who is drawing
        E_ARC_CCW // Arcs are actually rendered ccw
    };

    virtual void registerPressedKey(bool pressed, int key)
    {
        if (pressed){
            if (key == keyArcValue)
                LineMode = LINE_MODE_Arc;
            else
                LineMode = LINE_MODE_Line;
        }
        else
            LineMode = LINE_MODE_Line;
    }

    virtual void activated(ViewProviderSketch *sketchgui)
    {
        setCursor(QPixmap(cursor_createlineset),7,7);
    }

    virtual void mouseMove(Base::Vector2D onSketchPos)
    {
        setPositionText(onSketchPos);
        // This addresses the Arc being enabled or disabled during
        switch (LineMode){
            case LINE_MODE_Arc:
                switch(Mode){
                    case STATUS_ARC_SEEK_First:
                    case STATUS_ARC_SEEK_Second:
                    case STATUS_ARC_Do:
                    case STATUS_ARC_Close:
                        // we are in Arc mode and we are in an arc state.
                        // All is well
                        break;
                    case STATUS_LINE_SEEK_Second:
                    case STATUS_TAN_LINE_SEEK_Second:
                        EditCurve.resize(32);
                        Mode =STATUS_ARC_SEEK_Second;
                        break;
                    default:
                        Mode =STATUS_ARC_SEEK_First;//Put us in the default arc state mode
                    }
                break;
            case LINE_MODE_Line:
                switch(Mode){
                    case STATUS_LINE_SEEK_First:
                    case STATUS_LINE_Do:
                    case STATUS_LINE_Close:
                    case STATUS_TAN_LINE_SEEK_First:
                    case STATUS_TAN_LINE_Do:
                    case STATUS_TAN_LINE_Close:
                        break;

                    case STATUS_TAN_LINE_SEEK_Second:
                        if(previousElementType==E_STRAIGHT_LINE){
                            EditCurve.resize(2);
                            Mode =STATUS_LINE_SEEK_Second;
                        }
                        assert(EditCurve.size()==3);
                        break;

                    case STATUS_LINE_SEEK_Second:
                        if (previousElementType!=E_STRAIGHT_LINE){
                            EditCurve.resize(3);
                            Mode =STATUS_TAN_LINE_SEEK_Second;
                        }
                        break;
                    case STATUS_ARC_SEEK_Second:
                        EditCurve.resize(2);
                        Mode =STATUS_LINE_SEEK_Second;
                        break;
                    default:
                        Mode= STATUS_LINE_SEEK_Second;
                }
                break;
        }
        
        switch(Mode){
            case STATUS_LINE_SEEK_First:
            case STATUS_ARC_SEEK_First:
            case STATUS_TAN_LINE_SEEK_First:
                if (seekAutoConstraint(sugConstr1, onSketchPos, Base::Vector2D(0.f,0.f))) {
                    renderSuggestConstraintsCursor(sugConstr1);
                    return;
                }
                break;

            // ----- SECOND MODE -----/

            case STATUS_LINE_SEEK_Second:
            EditCurve[1] = onSketchPos;
            sketchgui->drawEdit(EditCurve);
            kVec.Set(0.f,0.f,0.f);// We're not dealing with a arc... zero the z-axis
            sugConstr2 = sugConstr1; // Copy the previously found constraints
            if (seekAutoConstraint(sugConstr2, onSketchPos, onSketchPos - EditCurve[0])) {
                renderSuggestConstraintsCursor(sugConstr2);
                renderHintLines(sugConstr2, EditCurve[0]);
                return;
            } else
                renderHintLines(sugConstr2, EditCurve[0]);
            break;
            case STATUS_ARC_SEEK_Second:
            case STATUS_TAN_LINE_SEEK_Second:

                if (getHighestCurveIndex() < 0) {
                    //Need to figure if there was a previous element?? In order to find perpendicular to the tangent
                    //Otherwise there can be infinate solutions do drawing an element.
                    tangent.Set(1.f,0.f,0.f);//todo needs to be addressed
                } else {
                //Need to determine if the previous element was a line or an arc or ???
                    const Part::Geometry *geom = sketchgui->getSketchObject()->getGeometry(getHighestCurveIndex());
                    if (geom->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
                        assert(previousElementType ==E_STRAIGHT_LINE);
                        const Part::GeomLineSegment *lineSeg = dynamic_cast<const Part::GeomLineSegment *>(geom);
                        EditCurve[0] = Base::Vector2D(lineSeg->getEndPoint().x, lineSeg->getEndPoint().y);
                        tangent.Set(lineSeg->getEndPoint().x - lineSeg->getStartPoint().x,
                                    lineSeg->getEndPoint().y - lineSeg->getStartPoint().y,
                                    0.f);
                    }
                    else if(geom->getTypeId() == Part::GeomArcOfCircle::getClassTypeId()){
                        //The previous element is an arc.
                        const Part::GeomArcOfCircle *arcSeg = dynamic_cast<const Part::GeomArcOfCircle *>(geom);
                        Base::Vector3d tempRadius;
                        if (previousElementType == E_ARC_CCW){
                           tempRadius=arcSeg->getEndPoint()-arcSeg->getCenter();
                           tangent = (Base::Vector3d(0.f,0.f,1.0 ))% tempRadius  ;
                        } else {
                          assert(previousElementType == E_ARC_CW);
                          tempRadius=arcSeg->getStartPoint() - arcSeg->getCenter();
                          tangent = (Base::Vector3d(0.f,0.f,-1.0 )) % tempRadius;
                        }
                    } else {
                        //It is neither a point or a line.
                        //todo.. Need to figure out what to do here.
                        tangent.Set(1.f,0.f,0.f);//todo needs to be addressed
                    }

                    //At this point we need to solve for the previous
                }

                Distance = onSketchPos - EditCurve[0];
                tangent.Normalize();
                Base::Vector2D tangent2d(tangent.x,tangent.y);

                double theta =(double)tangent2d.GetAngle(Distance);

                //At this point we need a unit normal vector pointing torwards then center of the arc we are drawing.
                //CCW -1, CW =1
                // Derivation of the formula used here can be found here: http://people.richland.edu/james/lecture/m116/matrices/area.html
                // Since we can be potentially be dealing with a line or an arc, we need to build the area triangle from the tangent

                float x1 = EditCurve[0].fX;
                float y1 = EditCurve[0].fY;

                float x2 = x1+tangent2d.fX;
                float y2 = y1+tangent2d.fY;

                float x3 = onSketchPos.fX ;
                float y3 = onSketchPos.fY;

                float twoTimesArea = (x2*y3-x3*y2)-(x1*y3-x3*y1)+(x1*y2-x2*y1);


                if (twoTimesArea < 0)
                    kVec.Set(0.f,0.f,1.f); //CW
                else
                    kVec.Set(0.f,0.f,-1.f);//CCW

                if (Mode==STATUS_ARC_SEEK_Second){
                    radiusLength= (Distance.Length())/(2.0*sin(theta ));
                    Base::Vector3d centerVec;
                    centerVec =  tangent % kVec;

                    centerVec.Normalize();
                    centerVec.Scale(radiusLength,radiusLength,radiusLength);

                    CenterPoint.Set(centerVec.x,centerVec.y);
                    CenterPoint = CenterPoint + EditCurve[0];

                    /* /**********************************************
                    startAngle = atan2(EditCurve[0].fY - CenterPoint.fY,
                                       EditCurve[0].fX - CenterPoint.fX);
                    float angle1 = atan2(onSketchPos.fY - CenterPoint.fY,
                                         onSketchPos.fX - CenterPoint.fX) - startAngle;
                    float angle2 = angle1 + (angle1 < 0. ? 2 : -2) * M_PI ;

                    arcAngle = abs(angle1-arcAngle) < abs(angle2-arcAngle) ? angle1 : angle2;
                    if (arcAngle > 0)
                        endAngle = startAngle + arcAngle;
                    else {
                        endAngle = startAngle;
                        startAngle += arcAngle;
                    }
                    */ //-----------------------------

                    //
                    startAngle = atan2(EditCurve[0].fY - CenterPoint.fY,
                                       EditCurve[0].fX - CenterPoint.fX);

                    endAngle = atan2(onSketchPos.fY - CenterPoint.fY,
                                         onSketchPos.fX - CenterPoint.fX);

                    if (kVec.z == -1.f ){//ccw
                        if (startAngle <= 0.f && endAngle >= 0.f )
                            arcAngle = endAngle - startAngle ;
                        else if (startAngle >=0  && endAngle <=0 )
                            arcAngle = abs(endAngle+ 2 * M_PI - startAngle);
                        else if(( startAngle < 0 && endAngle < 0 )|| (startAngle > 0 && endAngle > 0 )){//BOTH START AND FINISH EITHER UPPER OR LOWER QUANDRANTS
                            if (endAngle>startAngle)
                                arcAngle =endAngle-startAngle;
                             else
                                 arcAngle =endAngle-startAngle + 2* M_PI;
                        }
                        else
                            arcAngle = abs(endAngle - startAngle);//Don't think this point is ever reached

                        startAngleDraw= startAngle;
                        endAngleDraw=endAngle;

                    } else {//cw rotation
                        if (startAngle <= 0.f && endAngle >= 0.f  )
                            arcAngle = abs(endAngle - startAngle - 2 * M_PI) ;
                        else if (startAngle >= 0.f  && endAngle <= 0.f )
                            arcAngle = abs(-endAngle + startAngle);
                        else if(( startAngle < 0.f && endAngle < 0.f )|| (startAngle > 0.f && endAngle > 0.f )){//BOTH START AND FINISH EITHER UPPER OR LOWER QUANDRANTS
                            if (endAngle>startAngle)
                                  arcAngle = abs(endAngle - startAngle - 2* M_PI);
                             else
                                  arcAngle = abs(endAngle-startAngle);
                        }
                        else
                            arcAngle = abs(endAngle - startAngle);

                        startAngleDraw = endAngle ;
                        endAngleDraw = startAngle;
                    }

                    rx = EditCurve[0].fX - CenterPoint.fX;
                    ry = EditCurve[0].fY - CenterPoint.fY;

                    for (int i=1; i <= 29; i++) {
                        float angle = i*arcAngle/29.0;
                        float dx,dy;
                        if (kVec.z ==-1){
                            dx = rx * cos(angle) - ry * sin(angle);
                            dy = rx * sin(angle) + ry * cos(angle);
                        }
                        else{
                            dx = rx * cos(-angle) - ry * sin(-angle);
                            dy = rx * sin(-angle) + ry * cos(-angle);
                        }
                            EditCurve[i] = Base::Vector2D(CenterPoint.fX + dx, CenterPoint.fY + dy);
                        }

                    EditCurve[30] = CenterPoint;
                    EditCurve[31] = EditCurve[0];

                    sketchgui->drawEdit(EditCurve);
                    if (seekAutoConstraint(sugConstr3, onSketchPos, Base::Vector2D(0.f,0.f))) {
                        renderSuggestConstraintsCursor(sugConstr3);
                        return;
                    }
                    break;
            } else {
                    assert(Mode==STATUS_TAN_LINE_SEEK_Second);
                    float hypotenuseLegx =Distance.Length()*cos(theta);
                    //float hypotenuseLegy =Distance.Length()*sin(theta);
                    EditCurve[1] = EditCurve[0] + Base::Vector2D(tangent.x * hypotenuseLegx,tangent.y* hypotenuseLegx);
                    EditCurve[2] = onSketchPos;

                    sketchgui->drawEdit(EditCurve);
                }
        }

        applyCursor();

    }

    virtual bool pressButton(Base::Vector2D onSketchPos)
    {
        switch (Mode){
            case STATUS_LINE_SEEK_First:
                // remember our first point
                firstPoint = getHighestVertexIndex() + 1;
                firstCurve = getHighestCurveIndex() + 1;
                EditCurve.resize(2);
                EditCurve[0] = onSketchPos;
                lastPos =onSketchPos;
                Mode = STATUS_LINE_SEEK_Second;
                break;
            case STATUS_TAN_LINE_SEEK_First:
                // remember our first point
                firstPoint = getHighestVertexIndex() + 1;
                firstCurve = getHighestCurveIndex() + 1;
                EditCurve.resize(3);
                EditCurve[0] = onSketchPos;
                lastPos =onSketchPos;
                Mode = STATUS_TAN_LINE_SEEK_Second;
                break;

            case STATUS_ARC_SEEK_First:
                // remember our first point
                firstPoint = getHighestVertexIndex() + 1;
                firstCurve = getHighestCurveIndex() + 1;

                EditCurve.resize(32);
                EditCurve[0] = onSketchPos;
                lastPos = onSketchPos;
                Mode = STATUS_ARC_SEEK_Second;
                break;

            case STATUS_LINE_SEEK_Second:
                EditCurve[1] = onSketchPos;
                sketchgui->drawEdit(EditCurve);
                applyCursor();
                // exit on clicking exactly at the same position (e.g. double click)
                if (EditCurve[1] == EditCurve[0]) {
                    unsetCursor();
                    EditCurve.clear();
                    resetPositionText();
                    sketchgui->drawEdit(EditCurve);
                    sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider
                }
                if (sketchgui->getPreselectPoint() == firstPoint)
                    Mode = STATUS_LINE_Close;
                else
                    Mode = STATUS_LINE_Do;
                break;

            case STATUS_ARC_SEEK_Second:
                //EditCurve[1] = onSketchPos;
                //sketchgui->drawEdit(EditCurve);
                applyCursor();
                // exit on clicking exactly at the same position (e.g. double click)
                if (onSketchPos == EditCurve[0]) {
                    unsetCursor();
                    EditCurve.clear();
                    resetPositionText();
                    sketchgui->drawEdit(EditCurve);
                    sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider
                }
                if (sketchgui->getPreselectPoint() == firstPoint)
                    Mode = STATUS_ARC_Close;
                else
                    Mode = STATUS_ARC_Do;
                break;

            case STATUS_TAN_LINE_SEEK_Second:
                EditCurve[2] = onSketchPos;
                //sketchgui->drawEdit(EditCurve);
                applyCursor();
                // exit on clicking exactly at the same position (e.g. double click)
                if (EditCurve[2] == EditCurve[0]) {
                    unsetCursor();
                    EditCurve.clear();
                    resetPositionText();
                    sketchgui->drawEdit(EditCurve);
                    sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider
                }
                if (sketchgui->getPreselectPoint() == firstPoint)
                    Mode = STATUS_TAN_LINE_Close;
                else
                    Mode = STATUS_TAN_LINE_Do;
                break;

            }
        return true;
    }

    virtual bool releaseButton(Base::Vector2D onSketchPos)
    {
        if(Mode==STATUS_LINE_Do || Mode==STATUS_LINE_Close ||
            Mode == STATUS_ARC_Do || Mode ==STATUS_ARC_Close ||
            Mode==STATUS_TAN_LINE_Do || Mode==STATUS_TAN_LINE_Close) {

            if (Mode==STATUS_LINE_Do || Mode==STATUS_LINE_Close ||
                Mode==STATUS_TAN_LINE_Do || Mode==STATUS_TAN_LINE_Close) {
                // open the transaction
                Gui::Command::openCommand("add sketch wire");
                // issue the geometry
                Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addGeometry(Part.Line(App.Vector(%f,%f,0),App.Vector(%f,%f,0)))",
                sketchgui->getObject()->getNameInDocument(),
                EditCurve[0].fX,EditCurve[0].fY,EditCurve[1].fX,EditCurve[1].fY);
            }
            else if (Mode==STATUS_ARC_Do || Mode==STATUS_ARC_Close) {//We're dealing with an Arc
                Gui::Command::openCommand("Add sketch arc");
                Gui::Command::doCommand(Gui::Command::Doc,
                "App.ActiveDocument.%s.addGeometry(Part.ArcOfCircle"
                "(Part.Circle(App.Vector(%f,%f,0),App.Vector(0,0,1),%f),"
                "%f,%f))", sketchgui->getObject()->getNameInDocument(),
                           CenterPoint.fX, CenterPoint.fY, radiusLength,
                           startAngleDraw, endAngleDraw); //arcAngle > 0 ? 0 : 1);
            }
            // issue the constraint

            if (previousCurve != -1){
                /* we need to figure which point on the previous curve we're connecting to
                 * if the we have a ccw arc we should be connecting to the second point
                 * if we're going cw the start/ends are reversed since the arc is rendered ccw
                 * A line in theory should be the second point, but if we're restarting the polyline is could
                 * todo need to check the previous curve(once this is all working
                 * if we don't have a match on the previous curve need to keep going back till find one to make it coincident */

                if (Mode == STATUS_ARC_Do || Mode == STATUS_ARC_Close ||
                    Mode == STATUS_TAN_LINE_Do || Mode == STATUS_TAN_LINE_Close){
                    int coincidentPoint;
                    if (previousElementType == E_ARC_CW)
                        coincidentPoint = 1;
                    else // CCW or straight line
                        coincidentPoint = 2;

                    const Part::Geometry *geom = sketchgui->getSketchObject()->getGeometry(previousCurve);
                    if (geom->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
                        const Part::GeomLineSegment *lineSeg = dynamic_cast<const Part::GeomLineSegment *>(geom);

                        if (coincidentPoint != 0){
                            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Tangent',%i,%i,%i,%i)) "
                            ,sketchgui->getObject()->getNameInDocument()
                            ,previousCurve-1,coincidentPoint,previousCurve,1);
                        }
                    }
                    else if(geom->getTypeId() == Part::GeomArcOfCircle::getClassTypeId()){
                        //The previous element is an arc.
                        const Part::GeomArcOfCircle *arcSeg = dynamic_cast<const Part::GeomArcOfCircle *>(geom);

                        if (coincidentPoint != 0) {
                            Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Tangent',%i,%i,%i,%i)) "
                                  ,sketchgui->getObject()->getNameInDocument()
                                  ,previousCurve-1,coincidentPoint,previousCurve,(kVec.z==-1.0?1:2)
                                  );
                        }
                    }

                    Gui::Command::doCommand(Gui::Command::Doc,"print App.ActiveDocument.Sketch.Constraints");
                }
                else if (Mode==STATUS_LINE_Do || Mode==STATUS_LINE_Close){

                // close the loop by constrain to the first curve pointConstraint

                    Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Coincident',%i,2,%i,1)) "
                              ,sketchgui->getObject()->getNameInDocument()
                              ,previousCurve-1,previousCurve
                             );
                }
            }
            if (Mode == STATUS_LINE_Close|| Mode == STATUS_TAN_LINE_Close||Mode == STATUS_ARC_Close){

                if (Mode == STATUS_LINE_Close) {
                    // close the loop by constrain to the first curve point
                    Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addConstraint(Sketcher.Constraint('Coincident',%i,2,%i,1)) "
                    ,sketchgui->getObject()->getNameInDocument()
                    ,previousCurve,firstCurve
                     );

                    Gui::Command::commitCommand();
                    Gui::Command::updateActive();

                    if (sugConstr2.size() > 0) {
                        // exclude any coincidence constraints
                        std::vector<AutoConstraint> sugConstr;
                        for (int i=0; i < sugConstr2.size(); i++) {
                            if (sugConstr2[i].Type != Sketcher::Coincident)
                                sugConstr.push_back(sugConstr2[i]);
                        }
                        createAutoConstraints(sugConstr, getHighestCurveIndex(), Sketcher::end);
                        sugConstr2.clear();
                    }
                }
                unsetCursor();
                EditCurve.clear();
                resetPositionText();
                sketchgui->drawEdit(EditCurve);
                sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider

            } else {
//               //remember the vertex for the next rounds constraint...
//               previousCurve = getHighestCurveIndex() + 1;

//                // setup for the next line segment
//                // Use updated endPoint as autoconstraints can modify the position
//                const Part::Geometry *geom = sketchgui->getSketchObject()->getGeometry(getHighestCurveIndex());
//                if (geom->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
//                    const Part::GeomLineSegment *lineSeg = dynamic_cast<const Part::GeomLineSegment *>(geom);
//                    EditCurve[0] = Base::Vector2D(lineSeg->getEndPoint().x, lineSeg->getEndPoint().y);
//                }
//                else
//                    EditCurve[0] = onSketchPos;

        // Add auto constraints
//           if (sugConstr1.size() > 0)
//            {
//                createAutoConstraints(sugConstr1, getHighestCurveIndex(), Sketcher::start);
//                sugConstr1.clear();
//            }

//            if (sugConstr2.size() > 0)
//            {
//                createAutoConstraints(sugConstr2, getHighestCurveIndex(), Sketcher::end);
//                sugConstr2.clear();
//            }


            Gui::Command::commitCommand();
            Gui::Command::updateActive();
            //remember the vertex for the next rounds constraint..
            previousCurve = getHighestCurveIndex() + 1;

            // setup for the next line segment
            // Use updated endPoint as autoconstraints can modify the position
            const Part::Geometry *geom = sketchgui->getSketchObject()->getGeometry(getHighestCurveIndex());
            if (geom->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
                const Part::GeomLineSegment *lineSeg = dynamic_cast<const Part::GeomLineSegment *>(geom);
                previousElementType = E_STRAIGHT_LINE;
                EditCurve[0] = Base::Vector2D(lineSeg->getEndPoint().x, lineSeg->getEndPoint().y);
            }
            else if (geom->getTypeId() == Part::GeomArcOfCircle::getClassTypeId()){
                const Part::GeomArcOfCircle *arcSeg = dynamic_cast<const Part::GeomArcOfCircle *>(geom);

                if (kVec.z == 1.f){
                    EditCurve[0] = Base::Vector2D(arcSeg->getStartPoint().x,arcSeg->getStartPoint().y);
                    previousElementType = E_ARC_CW;
                } else { //cw arc are rendered in reverse
                    EditCurve[0] = Base::Vector2D(arcSeg->getEndPoint().x,arcSeg->getEndPoint().y);
                    previousElementType = E_ARC_CCW;
                }
            }
              EditCurve.resize(2);

              applyCursor();

              Mode = STATUS_LINE_SEEK_Second; //LineMode is checked in mousemove and will reset mode to arc
                                              //And will resize Edit Curve to and Arc if required or Tan_line.
            }
        }
        return true;
    }
protected:
    SelectMode Mode;

    std::vector<Base::Vector2D> EditCurve;
    Base::Vector2D lastPos;
    int firstPoint;
    int firstCurve;
    int previousCurve;
    ElementType previousElementType;
    std::vector<AutoConstraint> sugConstr1, sugConstr2, sugConstr3;

    Base::Vector2D CenterPoint,Distance;
    Base::Vector3d tangent;
    Base::Vector3d kVec; //kVec.z ==-1.0 ccw,kVec.z ==1  cw
    float rx, ry, startAngle, endAngle, arcAngle,startAngleDraw,endAngleDraw;

private:
   int keyArcValue;
   float radiusLength;
   SelectLineMode LineMode;
};


DEF_STD_CMD_A(CmdSketcherCreatePolyline);

CmdSketcherCreatePolyline::CmdSketcherCreatePolyline()
  : Command("Sketcher_CreatePolyline")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Create polyline");
    sToolTipText    = QT_TR_NOOP("Create a polyline in the sketch");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_CreatePolyline";
    eType           = ForEdit;
}

void CmdSketcherCreatePolyline::activated(int iMsg)
{
    ActivateHandler(getActiveGuiDocument(),new DrawSketchHandlerLineSet() );
}

bool CmdSketcherCreatePolyline::isActive(void)
{
    return isCreateGeoActive(getActiveGuiDocument());
}

// ======================================================================================

/* XPM */
static const char *cursor_createarc[]={
"32 32 3 1",
"+ c white",
"# c red",
". c None",
"......+...........###...........",
"......+...........#.#...........",
"......+...........###...........",
"......+..............##.........",
"......+...............##........",
".......................#........",
"+++++...+++++...........#.......",
"........................##......",
"......+..................#......",
"......+..................#......",
"......+...................#.....",
"......+...................#.....",
"......+...................#.....",
"..........................#.....",
"..........................#.....",
"..........................#.....",
"..........................#.....",
".........................#......",
".........................#......",
"........................#.......",
"........................#.......",
"...###.................#........",
"...#.#................#.........",
"...###...............#..........",
"......##...........##...........",
".......###.......##.............",
"..........#######...............",
"................................",
"................................",
"................................",
"................................",
"................................"};

class DrawSketchHandlerArc : public DrawSketchHandler
{
public:
    DrawSketchHandlerArc()
      : Mode(STATUS_SEEK_First),EditCurve(2){}
    virtual ~DrawSketchHandlerArc(){}
    /// mode table
    enum SelectMode {
        STATUS_SEEK_First,      /**< enum value ----. */
        STATUS_SEEK_Second,     /**< enum value ----. */
        STATUS_SEEK_Third,      /**< enum value ----. */
        STATUS_End
    };

    virtual void activated(ViewProviderSketch *sketchgui)
    {
        setCursor(QPixmap(cursor_createarc),7,7);
    }

    virtual void mouseMove(Base::Vector2D onSketchPos)
    {
        if (Mode==STATUS_SEEK_First) {
            setPositionText(onSketchPos);
            if (seekAutoConstraint(sugConstr1, onSketchPos, Base::Vector2D(0.f,0.f))) {
                renderSuggestConstraintsCursor(sugConstr1);
                return;
            }
        }
        else if (Mode==STATUS_SEEK_Second) {
            float dx_ = onSketchPos.fX - EditCurve[0].fX;
            float dy_ = onSketchPos.fY - EditCurve[0].fY;
            for (int i=0; i < 16; i++) {
                float angle = i*M_PI/16.0;
                float dx = dx_ * cos(angle) + dy_ * sin(angle);
                float dy = -dx_ * sin(angle) + dy_ * cos(angle);
                EditCurve[1+i] = Base::Vector2D(EditCurve[0].fX + dx, EditCurve[0].fY + dy);
                EditCurve[17+i] = Base::Vector2D(EditCurve[0].fX - dx, EditCurve[0].fY - dy);
            }
            EditCurve[33] = EditCurve[1];

            // Display radius and start angle
            float radius = (onSketchPos - EditCurve[0]).Length();
            float angle = atan2f(dy_ , dx_) * 180 / M_PI;

            char buf[40];
            sprintf( buf, "R(%.1f),%.1f ", radius, angle);
            std::string text = buf;
            setPositionText(onSketchPos, text);

            sketchgui->drawEdit(EditCurve);
            if (seekAutoConstraint(sugConstr2, onSketchPos, Base::Vector2D(0.f,0.f))) {
                renderSuggestConstraintsCursor(sugConstr2);
                return;
            }
        }
        else if (Mode==STATUS_SEEK_Third) {
            float angle1 = atan2(onSketchPos.fY - CenterPoint.fY,
                                 onSketchPos.fX - CenterPoint.fX) - startAngle;
            float angle2 = angle1 + (angle1 < 0. ? 2 : -2) * M_PI ;
            arcAngle = abs(angle1-arcAngle) < abs(angle2-arcAngle) ? angle1 : angle2;
            for (int i=1; i <= 29; i++) {
                float angle = i*arcAngle/29.0;
                float dx = rx * cos(angle) - ry * sin(angle);
                float dy = rx * sin(angle) + ry * cos(angle);
                EditCurve[i] = Base::Vector2D(CenterPoint.fX + dx, CenterPoint.fY + dy);
            }
            // Display radius and end angle
            float radius = (onSketchPos - EditCurve[0]).Length();

            char buf[40];
            sprintf( buf, "R(%.1f),%.1f ", radius, arcAngle * 180 / M_PI);
            std::string text = buf;
            setPositionText(onSketchPos, text);
            
            sketchgui->drawEdit(EditCurve);
            if (seekAutoConstraint(sugConstr3, onSketchPos, Base::Vector2D(0.f,0.f))) {
                renderSuggestConstraintsCursor(sugConstr3);
                return;
            }
        }
        applyCursor();

    }

    virtual bool pressButton(Base::Vector2D onSketchPos)
    {
        if (Mode==STATUS_SEEK_First){
            CenterPoint = onSketchPos;
            EditCurve.resize(34);
            EditCurve[0] = onSketchPos;
            Mode = STATUS_SEEK_Second;
        }
        else if (Mode==STATUS_SEEK_Second){
            EditCurve.resize(31);
            EditCurve[0] = onSketchPos;
            EditCurve[30] = CenterPoint;
            rx = EditCurve[0].fX - CenterPoint.fX;
            ry = EditCurve[0].fY - CenterPoint.fY;
            startAngle = atan2(ry, rx);
            arcAngle = 0.;
            Mode = STATUS_SEEK_Third;
        }
        else {
            EditCurve.resize(30);
            float angle1 = atan2(onSketchPos.fY - CenterPoint.fY,
                                 onSketchPos.fX - CenterPoint.fX) - startAngle;
            float angle2 = angle1 + (angle1 < 0. ? 2 : -2) * M_PI ;
            arcAngle = abs(angle1-arcAngle) < abs(angle2-arcAngle) ? angle1 : angle2;
            if (arcAngle > 0)
                endAngle = startAngle + arcAngle;
            else {
                endAngle = startAngle;
                startAngle += arcAngle;
            }

            sketchgui->drawEdit(EditCurve);
            applyCursor();
            Mode = STATUS_End;
        }

        return true;
    }

    virtual bool releaseButton(Base::Vector2D onSketchPos)
    {
        if (Mode==STATUS_End) {
            unsetCursor();
            resetPositionText();
            Gui::Command::openCommand("Add sketch arc");
            Gui::Command::doCommand(Gui::Command::Doc,
                "App.ActiveDocument.%s.addGeometry(Part.ArcOfCircle"
                "(Part.Circle(App.Vector(%f,%f,0),App.Vector(0,0,1),%f),"
                "%f,%f))",
                      sketchgui->getObject()->getNameInDocument(),
                      CenterPoint.fX, CenterPoint.fY, sqrt(rx*rx + ry*ry),
                      startAngle, endAngle); //arcAngle > 0 ? 0 : 1);

            Gui::Command::commitCommand();
            Gui::Command::updateActive();

            // Auto Constraint center point
            if (sugConstr1.size() > 0) {
                createAutoConstraints(sugConstr1, getHighestCurveIndex(), Sketcher::mid);
                sugConstr1.clear();
            }

            // Auto Constraint first picked point
            if (sugConstr2.size() > 0) {
                createAutoConstraints(sugConstr2, getHighestCurveIndex(), (arcAngle > 0) ? Sketcher::start : Sketcher::end );
                sugConstr2.clear();
            }

            // Auto Constraint second picked point
            if (sugConstr3.size() > 0) {
                createAutoConstraints(sugConstr3, getHighestCurveIndex(), (arcAngle > 0) ? Sketcher::end : Sketcher::start);
                sugConstr3.clear();
            }

            EditCurve.clear();
            sketchgui->drawEdit(EditCurve);
            sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider
        }
        return true;
    }
protected:
    SelectMode Mode;
    std::vector<Base::Vector2D> EditCurve;
    Base::Vector2D CenterPoint;
    float rx, ry, startAngle, endAngle, arcAngle;
    std::vector<AutoConstraint> sugConstr1, sugConstr2, sugConstr3;
};

DEF_STD_CMD_A(CmdSketcherCreateArc);

CmdSketcherCreateArc::CmdSketcherCreateArc()
  : Command("Sketcher_CreateArc")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Create arc");
    sToolTipText    = QT_TR_NOOP("Create an arc in the sketch");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_CreateArc";
    eType           = ForEdit;
}

void CmdSketcherCreateArc::activated(int iMsg)
{
    ActivateHandler(getActiveGuiDocument(),new DrawSketchHandlerArc() );
}

bool CmdSketcherCreateArc::isActive(void)
{
    return isCreateGeoActive(getActiveGuiDocument());
}

// ======================================================================================

/* XPM */
static const char *cursor_createcircle[]={
"32 32 3 1",
"+ c white",
"# c red",
". c None",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"................................",
"+++++...+++++...................",
"................................",
"......+........#######..........",
"......+......##.......##........",
"......+.....#...........#.......",
"......+....#.............#......",
"......+...#...............#.....",
".........#.................#....",
"........#...................#...",
"........#...................#...",
".......#.....................#..",
".......#.....................#..",
".......#.........###.........#..",
".......#.........#.#.........#..",
".......#.........###.........#..",
".......#.....................#..",
".......#.....................#..",
"........#...................#...",
"........#...................#...",
".........#.................#....",
"..........#...............#.....",
"...........#.............#......",
"............#...........#.......",
".............##.......##........",
"...............#######..........",
"................................"};

class DrawSketchHandlerCircle : public DrawSketchHandler
{
public:
    DrawSketchHandlerCircle() : Mode(STATUS_SEEK_First),EditCurve(34){}
    virtual ~DrawSketchHandlerCircle(){}
    /// mode table
    enum SelectMode {
        STATUS_SEEK_First,      /**< enum value ----. */
        STATUS_SEEK_Second,     /**< enum value ----. */
        STATUS_Close
    };

    virtual void activated(ViewProviderSketch *sketchgui)
    {
        setCursor(QPixmap(cursor_createcircle),7,7);
    }

    virtual void mouseMove(Base::Vector2D onSketchPos)
    {

        if (Mode==STATUS_SEEK_First) {
            setPositionText(onSketchPos);
            if (seekAutoConstraint(sugConstr1, onSketchPos, Base::Vector2D(0.f,0.f))) {
                renderSuggestConstraintsCursor(sugConstr1);
                return;
            }
        }
        else if (Mode==STATUS_SEEK_Second) {
            float rx0 = onSketchPos.fX - EditCurve[0].fX;
            float ry0 = onSketchPos.fY - EditCurve[0].fY;
            for (int i=0; i < 16; i++) {
                float angle = i*M_PI/16.0;
                float rx = rx0 * cos(angle) + ry0 * sin(angle);
                float ry = -rx0 * sin(angle) + ry0 * cos(angle);
                EditCurve[1+i] = Base::Vector2D(EditCurve[0].fX + rx, EditCurve[0].fY + ry);
                EditCurve[17+i] = Base::Vector2D(EditCurve[0].fX - rx, EditCurve[0].fY - ry);
            }
            EditCurve[33] = EditCurve[1];

            // Display radius for user
            float radius = (onSketchPos - EditCurve[0]).Length();

            char buf[40];
            sprintf( buf, "R(%.1f)", radius);
            std::string text = buf;
            setPositionText(onSketchPos, text);

            sketchgui->drawEdit(EditCurve);
            if (seekAutoConstraint(sugConstr2, onSketchPos, Base::Vector2D(0.f,0.f), CURVE)) {
                renderSuggestConstraintsCursor(sugConstr2);
                return;
            }
        }
        applyCursor();
    }

    virtual bool pressButton(Base::Vector2D onSketchPos)
    {
        if (Mode==STATUS_SEEK_First){
            EditCurve[0] = onSketchPos;
            Mode = STATUS_SEEK_Second;
        } else {
            EditCurve[1] = onSketchPos;
            Mode = STATUS_Close;
        }
        return true;
    }

    virtual bool releaseButton(Base::Vector2D onSketchPos)
    {
        if (Mode==STATUS_Close) {
            float rx = EditCurve[1].fX - EditCurve[0].fX;
            float ry = EditCurve[1].fY - EditCurve[0].fY;
            unsetCursor();
            resetPositionText();
            Gui::Command::openCommand("Add sketch circle");
            Gui::Command::doCommand(Gui::Command::Doc,
                "App.ActiveDocument.%s.addGeometry(Part.Circle"
                "(App.Vector(%f,%f,0),App.Vector(0,0,1),%f))",
                      sketchgui->getObject()->getNameInDocument(),
                      EditCurve[0].fX, EditCurve[0].fY,
                      sqrt(rx*rx + ry*ry));

            Gui::Command::commitCommand();
            Gui::Command::updateActive();

            // add auto constraints for the center point
            if (sugConstr1.size() > 0) {
                createAutoConstraints(sugConstr1, getHighestCurveIndex(), Sketcher::mid);
                sugConstr1.clear();
            }

            // add suggested constraints for circumference
            if (sugConstr2.size() > 0) {
                createAutoConstraints(sugConstr2, getHighestCurveIndex(), Sketcher::none);
                sugConstr2.clear();
            }

            EditCurve.clear();
            sketchgui->drawEdit(EditCurve);
            sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider
        }
        return true;
    }
protected:
    SelectMode Mode;
    std::vector<Base::Vector2D> EditCurve;
    std::vector<AutoConstraint> sugConstr1, sugConstr2;

};

DEF_STD_CMD_A(CmdSketcherCreateCircle);

CmdSketcherCreateCircle::CmdSketcherCreateCircle()
  : Command("Sketcher_CreateCircle")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Create circle");
    sToolTipText    = QT_TR_NOOP("Create a circle in the sketch");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_CreateCircle";
    eType           = ForEdit;
}

void CmdSketcherCreateCircle::activated(int iMsg)
{
    ActivateHandler(getActiveGuiDocument(),new DrawSketchHandlerCircle() );
}

bool CmdSketcherCreateCircle::isActive(void)
{
    return isCreateGeoActive(getActiveGuiDocument());
}

// ======================================================================================

DEF_STD_CMD_A(CmdSketcherCreatePoint);

CmdSketcherCreatePoint::CmdSketcherCreatePoint()
  : Command("Sketcher_CreatePoint")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Create point");
    sToolTipText    = QT_TR_NOOP("Create a point in the sketch");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_CreatePoint";
    eType           = ForEdit;
}

void CmdSketcherCreatePoint::activated(int iMsg)
{
}

bool CmdSketcherCreatePoint::isActive(void)
{
    return false;
}

// ======================================================================================

DEF_STD_CMD_A(CmdSketcherCreateText);

CmdSketcherCreateText::CmdSketcherCreateText()
  : Command("Sketcher_CreateText")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Create text");
    sToolTipText    = QT_TR_NOOP("Create text in the sketch");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_CreateText";
    eType           = ForEdit;
}

void CmdSketcherCreateText::activated(int iMsg)
{
}

bool CmdSketcherCreateText::isActive(void)
{
    return false;
}

// ======================================================================================

DEF_STD_CMD_A(CmdSketcherCreateDraftLine);

CmdSketcherCreateDraftLine::CmdSketcherCreateDraftLine()
  : Command("Sketcher_CreateDraftLine")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Create draft line");
    sToolTipText    = QT_TR_NOOP("Create a draft line in the sketch");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_DraftLine";
    eType           = ForEdit;
}

void CmdSketcherCreateDraftLine::activated(int iMsg)
{
}

bool CmdSketcherCreateDraftLine::isActive(void)
{
    return false;
}

// ======================================================================================

namespace SketcherGui {
    class FilletSelection : public Gui::SelectionFilterGate
    {
        App::DocumentObject* object;
    public:
        FilletSelection(App::DocumentObject* obj)
            : Gui::SelectionFilterGate((Gui::SelectionFilter*)0), object(obj)
        {}

        bool allow(App::Document *pDoc, App::DocumentObject *pObj, const char *sSubName)
        {
            if (pObj != this->object)
                return false;
            if (!sSubName || sSubName[0] == '\0')
                return false;
            std::string element(sSubName);
            if (element.substr(0,4) == "Edge") {
                int index=std::atoi(element.substr(4,4000).c_str());
                Sketcher::SketchObject *Sketch = static_cast<Sketcher::SketchObject*>(object);
                const Part::Geometry *geom = Sketch->getGeometry(index);
                if (geom->getTypeId() == Part::GeomLineSegment::getClassTypeId())
                    return true;
            }
            if (element.substr(0,6) == "Vertex") {
                int index=std::atoi(element.substr(6,4000).c_str());
                Sketcher::SketchObject *Sketch = static_cast<Sketcher::SketchObject*>(object);
                std::vector<int> GeoIdList;
                std::vector<Sketcher::PointPos> PosIdList;
                Sketch->getCoincidentPoints(index, GeoIdList, PosIdList);
                if (GeoIdList.size() == 2 && GeoIdList[0] >= 0  && GeoIdList[1] >= 0) {
                    const Part::Geometry *geom1 = Sketch->getGeometry(GeoIdList[0]);
                    const Part::Geometry *geom2 = Sketch->getGeometry(GeoIdList[1]);
                    if (geom1->getTypeId() == Part::GeomLineSegment::getClassTypeId() &&
                        geom2->getTypeId() == Part::GeomLineSegment::getClassTypeId())
                        return true;
                }
            }
            return  false;
        }
    };
};

/* XPM */
static const char *cursor_createfillet[]={
"32 32 3 1",
"+ c white",
"* c red",
". c None",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"................................",
"+++++...+++++...................",
"................................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+..*......................",
".........*......................",
".........*......................",
".........*......................",
".........*......................",
".........*......................",
".........*.........***..........",
".........*.........*.*..........",
".........*.........***..........",
".........*......................",
".........*......................",
"..........*.....................",
"..........*.....................",
"...........*....................",
"............*...................",
".............*..................",
"..............*.................",
"...............**...............",
".................**************.",
"................................"};

class DrawSketchHandlerFillet: public DrawSketchHandler
{
public:
    DrawSketchHandlerFillet() : Mode(STATUS_SEEK_First) {}
    virtual ~DrawSketchHandlerFillet()
    {
        Gui::Selection().rmvSelectionGate();
    }
    enum SelectMode{
        STATUS_SEEK_First,
        STATUS_SEEK_Second
    };

    virtual void activated(ViewProviderSketch *sketchgui)
    {
        Gui::Selection().rmvSelectionGate();
        Gui::Selection().addSelectionGate(new FilletSelection(sketchgui->getObject()));
        setCursor(QPixmap(cursor_createfillet),7,7);
    }

    virtual void mouseMove(Base::Vector2D onSketchPos)
    {
    }

    virtual bool pressButton(Base::Vector2D onSketchPos)
    {
        return true;
    }

    virtual bool releaseButton(Base::Vector2D onSketchPos)
    {
        int VtId = sketchgui->getPreselectPoint();
        if (Mode == STATUS_SEEK_First && VtId != -1) {
            int GeoId;
            Sketcher::PointPos PosId=Sketcher::none;
            sketchgui->getSketchObject()->getGeoVertexIndex(VtId,GeoId,PosId);
            const Part::Geometry *geom = sketchgui->getSketchObject()->getGeometry(GeoId);
            if (geom->getTypeId() == Part::GeomLineSegment::getClassTypeId() &&
                (PosId == Sketcher::start || PosId == Sketcher::end)) {

                // guess fillet radius
                double radius=-1;
                std::vector<int> GeoIdList;
                std::vector<Sketcher::PointPos> PosIdList;
                sketchgui->getSketchObject()->getCoincidentPoints(GeoId, PosId, GeoIdList, PosIdList);
                if (GeoIdList.size() == 2 && GeoIdList[0] >= 0  && GeoIdList[1] >= 0) {
                    const Part::Geometry *geom1 = sketchgui->getSketchObject()->getGeometry(GeoIdList[0]);
                    const Part::Geometry *geom2 = sketchgui->getSketchObject()->getGeometry(GeoIdList[1]);
                    if (geom1->getTypeId() == Part::GeomLineSegment::getClassTypeId() &&
                        geom2->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
                        const Part::GeomLineSegment *lineSeg1 = dynamic_cast<const Part::GeomLineSegment *>(geom1);
                        const Part::GeomLineSegment *lineSeg2 = dynamic_cast<const Part::GeomLineSegment *>(geom2);
                        Base::Vector3d dir1 = lineSeg1->getEndPoint() - lineSeg1->getStartPoint();
                        Base::Vector3d dir2 = lineSeg2->getEndPoint() - lineSeg2->getStartPoint();
                        if (PosIdList[0] == Sketcher::end)
                            dir1 *= -1;
                        if (PosIdList[1] == Sketcher::end)
                            dir2 *= -1;
                        double l1 = dir1.Length();
                        double l2 = dir2.Length();
                        double angle = dir1.GetAngle(dir2);
                        radius = (l1 < l2 ? l1 : l2) * 0.2 * sin(angle/2);
                    }
                }
                if (radius < 0)
                    return false;

                // create fillet at point
                Gui::Command::openCommand("Create fillet");
                Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.fillet(%d,%d,%f)",
                          sketchgui->getObject()->getNameInDocument(),
                          GeoId, PosId, radius);
                Gui::Command::commitCommand();
                Gui::Command::updateActive();
            }
            return true;
        }

        int GeoId = sketchgui->getPreselectCurve();
        if (GeoId > -1) {
            const Part::Geometry *geom = sketchgui->getSketchObject()->getGeometry(GeoId);
            if (geom->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
                if (Mode==STATUS_SEEK_First) {
                    firstCurve = GeoId;
                    firstPos = onSketchPos;
                    Mode = STATUS_SEEK_Second;
                    // add the line to the selection
                    std::stringstream ss;
                    ss << "Edge" << firstCurve;
                    Gui::Selection().addSelection(sketchgui->getSketchObject()->getDocument()->getName()
                                                 ,sketchgui->getSketchObject()->getNameInDocument()
                                                 ,ss.str().c_str()
                                                 ,onSketchPos.fX
                                                 ,onSketchPos.fY
                                                 ,0.f);
                }
                else if (Mode==STATUS_SEEK_Second) {
                    int secondCurve = GeoId;
                    Base::Vector2D secondPos = onSketchPos;

                    // guess fillet radius
                    const Part::GeomLineSegment *lineSeg1 = dynamic_cast<const Part::GeomLineSegment *>
                                                            (sketchgui->getSketchObject()->getGeometry(firstCurve));
                    const Part::GeomLineSegment *lineSeg2 = dynamic_cast<const Part::GeomLineSegment *>
                                                            (sketchgui->getSketchObject()->getGeometry(secondCurve));
                    Base::Vector3d refPnt1(firstPos.fX, firstPos.fY, 0.f);
                    Base::Vector3d refPnt2(secondPos.fX, secondPos.fY, 0.f);
                    double radius = Part::suggestFilletRadius(lineSeg1, lineSeg2, refPnt1, refPnt2);
                    if (radius < 0)
                        return false;

                    // create fillet between lines
                    Gui::Command::openCommand("Create fillet");
                    Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.fillet(%d,%d,App.Vector(%f,%f,0),App.Vector(%f,%f,0),%f)",
                              sketchgui->getObject()->getNameInDocument(),
                              firstCurve, secondCurve,
                              firstPos.fX, firstPos.fY,
                              secondPos.fX, secondPos.fY, radius);
                    Gui::Command::commitCommand();
                    Gui::Command::updateActive();

                    Gui::Selection().clearSelection();
                    Mode = STATUS_SEEK_First;
                }
            }
        }

        if (VtId < 0 && GeoId < 0) // exit the fillet tool if the user clicked on empty space
            sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider

        return true;
    }

protected:
    SelectMode Mode;
    int firstCurve;
    Base::Vector2D firstPos;
};

DEF_STD_CMD_A(CmdSketcherCreateFillet);

CmdSketcherCreateFillet::CmdSketcherCreateFillet()
  : Command("Sketcher_CreateFillet")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Create fillet");
    sToolTipText    = QT_TR_NOOP("Create a fillet between two lines or at a coincidental point");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_CreateFillet";
    sAccel          = "F";
    eType           = ForEdit;
}

void CmdSketcherCreateFillet::activated(int iMsg)
{
    ActivateHandler(getActiveGuiDocument(), new DrawSketchHandlerFillet());
}

bool CmdSketcherCreateFillet::isActive(void)
{
    return isCreateGeoActive(getActiveGuiDocument());
}
// ======================================================================================

namespace SketcherGui {
    class TrimmingSelection : public Gui::SelectionFilterGate
    {
        App::DocumentObject* object;
    public:
        TrimmingSelection(App::DocumentObject* obj)
            : Gui::SelectionFilterGate((Gui::SelectionFilter*)0), object(obj)
        {}

        bool allow(App::Document *pDoc, App::DocumentObject *pObj, const char *sSubName)
        {
            if (pObj != this->object)
                return false;
            if (!sSubName || sSubName[0] == '\0')
                return false;
            std::string element(sSubName);
            if (element.substr(0,4) == "Edge") {
                int index=std::atoi(element.substr(4,4000).c_str());
                Sketcher::SketchObject *Sketch = static_cast<Sketcher::SketchObject*>(object);
                const Part::Geometry *geom = Sketch->getGeometry(index);
                if (geom->getTypeId() == Part::GeomLineSegment::getClassTypeId() ||
                    geom->getTypeId() == Part::GeomCircle::getClassTypeId()||
                    geom->getTypeId() == Part::GeomArcOfCircle::getClassTypeId())
                    return true;
            }
            return  false;
        }
    };
};

/* XPM */
static const char *cursor_trimming[]={
"32 32 3 1",
"+ c white",
"* c red",
". c None",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"................................",
"+++++...+++++...................",
"................................",
"......+.........................",
"......+.........................",
"......+......................*..",
"......+....................**...",
"......+...................**....",
".*..............................",
"..*.....................*.......",
"...*..................**........",
".....*...............**.........",
"......*.........................",
".......*..........*.............",
".........*......**..............",
"..........*....**...............",
"...........****.................",
"............*.*.................",
"............***.................",
"..........*....*................",
".........*.......*..............",
".......*..........*.............",
"......*............*............",
"....*................*..........",
"...*..................*.........",
".*.....................*........",
".........................*......"};

class DrawSketchHandlerTrimming: public DrawSketchHandler
{
public:
    DrawSketchHandlerTrimming() {}
    virtual ~DrawSketchHandlerTrimming()
    {
        Gui::Selection().rmvSelectionGate();
    }

    virtual void activated(ViewProviderSketch *sketchgui)
    {
        Gui::Selection().clearSelection();
        Gui::Selection().rmvSelectionGate();
        Gui::Selection().addSelectionGate(new TrimmingSelection(sketchgui->getObject()));
        setCursor(QPixmap(cursor_trimming),7,7);
    }

    virtual void mouseMove(Base::Vector2D onSketchPos)
    {
    }

    virtual bool pressButton(Base::Vector2D onSketchPos)
    {
        return true;
    }

    virtual bool releaseButton(Base::Vector2D onSketchPos)
    {
        int GeoId = sketchgui->getPreselectCurve();
        if (GeoId > -1) {
            const Part::Geometry *geom = sketchgui->getSketchObject()->getGeometry(GeoId);
            if (geom->getTypeId() == Part::GeomLineSegment::getClassTypeId() ||
                geom->getTypeId() == Part::GeomArcOfCircle::getClassTypeId() ||
                geom->getTypeId() == Part::GeomCircle::getClassTypeId()
            ) {
                try {
                    Gui::Command::openCommand("Trim edge");
                    Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.trim(%d,App.Vector(%f,%f,0))",
                              sketchgui->getObject()->getNameInDocument(),
                              GeoId, onSketchPos.fX, onSketchPos.fY);
                    Gui::Command::commitCommand();
                    Gui::Command::updateActive();
                }
                catch (const Base::Exception& e) {
                    Base::Console().Error("%s\n", e.what());
                }
            }
        }
        else // exit the trimming tool if the user clicked on empty space
            sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider

        return true;
    }
};

DEF_STD_CMD_A(CmdSketcherTrimming);

CmdSketcherTrimming::CmdSketcherTrimming()
  : Command("Sketcher_Trimming")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("Trim edge");
    sToolTipText    = QT_TR_NOOP("Trim an edge with respect to the picked position");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_Trimming";
    sAccel          = "T";
    eType           = ForEdit;
}

void CmdSketcherTrimming::activated(int iMsg)
{
    ActivateHandler(getActiveGuiDocument(), new DrawSketchHandlerTrimming());
}

bool CmdSketcherTrimming::isActive(void)
{
    return isCreateGeoActive(getActiveGuiDocument());
}

// ======================================================================================

namespace SketcherGui {
    class ExternalSelection : public Gui::SelectionFilterGate
    {
        App::DocumentObject* object;
    public:
        ExternalSelection(App::DocumentObject* obj)
            : Gui::SelectionFilterGate((Gui::SelectionFilter*)0), object(obj)
        {}

        bool allow(App::Document *pDoc, App::DocumentObject *pObj, const char *sSubName)
        {
            Sketcher::SketchObject *sketch = static_cast<Sketcher::SketchObject*>(object);
            App::DocumentObject *support = sketch->Support.getValue();
            // for the moment we allow external constraints only from the support
            if (pObj != support)
                return false;
            if (!sSubName || sSubName[0] == '\0')
                return false;
            std::string element(sSubName);
            // for the moment we allow only edges
            if (element.substr(0,4) == "Edge") {
                return true;
            }
            return  false;
        }
    };
};

/* XPM */
static const char *cursor_external[]={
"32 32 3 1",
"+ c white",
"* c red",
". c None",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"................................",
"+++++...+++++...................",
"................................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+.........................",
"......+....***************......",
".........**...............***...",
"........**................***...",
".......**................**.*...",
"......*.................*...*...",
"....**................**....*...",
"...**................**.....*...",
"..**................**......*...",
"..******************........*...",
"..*................*........*...",
"..*................*........*...",
"..*................*........*...",
"..*................*............",
"..*................*............",
"..*................*............",
"..*................*............",
"..*................*............",
"..*................*............",
"................................",
"................................"};

class DrawSketchHandlerExternal: public DrawSketchHandler
{
public:
    DrawSketchHandlerExternal() {}
    virtual ~DrawSketchHandlerExternal()
    {
        Gui::Selection().rmvSelectionGate();
    }

    virtual void activated(ViewProviderSketch *sketchgui)
    {
        Gui::MDIView *mdi = Gui::Application::Instance->activeDocument()->getActiveView();
        Gui::View3DInventorViewer *viewer;
        viewer = static_cast<Gui::View3DInventor *>(mdi)->getViewer();

        SoNode* root = viewer->getSceneGraph();
        static_cast<Gui::SoFCUnifiedSelection*>(root)->selectionRole.setValue(TRUE);

        Gui::Selection().clearSelection();
        Gui::Selection().rmvSelectionGate();
        Gui::Selection().addSelectionGate(new ExternalSelection(sketchgui->getObject()));
        setCursor(QPixmap(cursor_external),7,7);
    }

    virtual void mouseMove(Base::Vector2D onSketchPos)
    {
        applyCursor();
    }

    virtual bool pressButton(Base::Vector2D onSketchPos)
    {
        return true;
    }

    virtual bool releaseButton(Base::Vector2D onSketchPos)
    {
        sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider
        return true;
    }

    virtual bool onSelectionChanged(const Gui::SelectionChanges& msg)
    {
        if (msg.Type == Gui::SelectionChanges::AddSelection) {
            std::string subName(msg.pSubName);
            if (subName.size() > 4 && subName.substr(0,4) == "Edge") {
                try {
                    Gui::Command::openCommand("Add external geometry");
                    Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.addExternal(\"%s\",\"%s\")",
                              sketchgui->getObject()->getNameInDocument(),
                              msg.pObjectName, msg.pSubName);
                    Gui::Command::commitCommand();
                    Gui::Command::updateActive();
                    Gui::Selection().clearSelection();
                    sketchgui->purgeHandler(); // no code after this line, Handler get deleted in ViewProvider
                }
                catch (const Base::Exception& e) {
                    Base::Console().Error("%s\n", e.what());
                }
                return true;
            }
        }
        return false;
    }
};

DEF_STD_CMD_A(CmdSketcherExternal);

CmdSketcherExternal::CmdSketcherExternal()
  : Command("Sketcher_External")
{
    sAppModule      = "Sketcher";
    sGroup          = QT_TR_NOOP("Sketcher");
    sMenuText       = QT_TR_NOOP("External geometry");
    sToolTipText    = QT_TR_NOOP("Create an edge linked to an external geometry");
    sWhatsThis      = sToolTipText;
    sStatusTip      = sToolTipText;
    sPixmap         = "Sketcher_External";
    sAccel          = "E";
    eType           = ForEdit;
}

void CmdSketcherExternal::activated(int iMsg)
{
    ActivateHandler(getActiveGuiDocument(), new DrawSketchHandlerExternal());
}

bool CmdSketcherExternal::isActive(void)
{
    return isCreateGeoActive(getActiveGuiDocument());
}


void CreateSketcherCommandsCreateGeo(void)
{
    Gui::CommandManager &rcCmdMgr = Gui::Application::Instance->commandManager();

    //rcCmdMgr.addCommand(new CmdSketcherCreatePoint());
    rcCmdMgr.addCommand(new CmdSketcherCreateArc());
    rcCmdMgr.addCommand(new CmdSketcherCreateCircle());
    rcCmdMgr.addCommand(new CmdSketcherCreateLine());
    rcCmdMgr.addCommand(new CmdSketcherCreatePolyline());
    rcCmdMgr.addCommand(new CmdSketcherCreateRectangle());
    rcCmdMgr.addCommand(new CmdSketcherCreateFillet());
    //rcCmdMgr.addCommand(new CmdSketcherCreateText());
    //rcCmdMgr.addCommand(new CmdSketcherCreateDraftLine());
    rcCmdMgr.addCommand(new CmdSketcherTrimming());
    rcCmdMgr.addCommand(new CmdSketcherExternal());
}
