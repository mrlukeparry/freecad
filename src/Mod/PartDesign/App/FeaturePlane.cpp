/***************************************************************************
 *   Copyright (c) 2010 Juergen Riegel <FreeCAD@juergen-riegel.net>        *
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
# include <BRep_Builder.hxx>
# include <BRepGProp.hxx>
# include <BRepBndLib.hxx>
# include <BRepBuilderAPI_MakeFace.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <BRep_Tool.hxx>
# include <Geom_Plane.hxx>
# include <GProp_GProps.hxx>
# include <TopoDS.hxx>
# include <TopoDS_Face.hxx>
# include <TopoDS_Vertex.hxx>
# include <TopoDS_Edge.hxx>
# include <gce_MakePln.hxx>
# include <gp_Ax1.hxx>
# include <gp_Pnt.hxx>
# include <gp_Dir.hxx>
# include <GeomAPI_ProjectPointOnSurf.hxx>
# include <Geom_Plane.hxx>
# include <Geom2d_Curve.hxx>
# include <Geom2dAPI_ProjectPointOnCurve.hxx>
# include <GeomAPI.hxx>
# include <BRepAdaptor_Surface.hxx>
#endif


#include "FeaturePlane.h"
#include <Mod/Part/App/Part2DObject.h>

using namespace PartDesign;

const char* Plane::TypeEnums[]= {"Points","Face",NULL};

namespace PartDesign {

PROPERTY_SOURCE(PartDesign::Plane, PartDesign::Feature)

Plane::Plane()
{
    ADD_PROPERTY(Type,((long)0));
    Type.setEnums(TypeEnums);
    ADD_PROPERTY(OffsetX,(0.0));
    ADD_PROPERTY(OffsetY,(0.0));
    ADD_PROPERTY(OffsetZ,(0.0));
    ADD_PROPERTY(Rotation,(0.0));
    ADD_PROPERTY(Reversed,(0));
    ADD_PROPERTY(Entity1,(0));
    ADD_PROPERTY(Entity2,(0));
    ADD_PROPERTY(Entity3,(0));

    numFaces = 0;
    numEdges = 0;
    numVertices = 0;
}

short Plane::mustExecute() const
{
    if (Placement.isTouched() ||
        OffsetX.isTouched()   ||
        OffsetY.isTouched()   ||
        OffsetZ.isTouched()   ||
        Rotation.isTouched()  ||
        Reversed.isTouched()  ||
        Entity1.isTouched()   ||
        Entity2.isTouched()   ||
        Entity3.isTouched() )
        return 1;
    return 0;
}


void Plane::checkRefTypes(void)
{

  std::vector<App::PropertyLinkSub *> refs;
  std::vector<TopoDS_Shape> shapes;
  refs.push_back(&Entity1);
  refs.push_back(&Entity2);
  refs.push_back(&Entity3);

  // Reset Counters
  numVertices = 0;
  numFaces = 0;
  numEdges = 0;

  int i = 0;
  for (std::vector<App::PropertyLinkSub *>::const_iterator it=refs.begin(); it!=refs.end();++it, i++) {

      if((*it)->getValue()) {
          const Part::Feature *feat = static_cast<Part::Feature*>((*it)->getValue());
          if(!feat || !feat->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId()))
            continue;

          const std::vector<std::string> &sub = (*it)->getSubValues();

          // Get the shape
          const Part::TopoShape &shape = feat->Shape.getShape();
          if (shape._Shape.IsNull())
              continue;

          // Get the SubShape
          const TopoDS_Shape sh = shape.getSubShape(sub[0].c_str());

          bool isSameGeom = false;
          for (std::vector<TopoDS_Shape>::const_iterator shape=shapes.begin(); shape!=shapes.end();++shape)
          {
              if(sh.IsSame( (*shape) )) {
                isSameGeom = true;
                break;
              }
          }

          shapes.push_back(sh);

          if(isSameGeom)
            continue;

          // Check the shape type
          switch (sh.ShapeType()) {
            case TopAbs_FACE:
              numFaces++; break;
            case TopAbs_VERTEX:
              numVertices++; break;
            case TopAbs_EDGE:
              numEdges++; break;
            default: break;
          }
      }
  }
}

App::DocumentObjectExecReturn *Plane::execute(void)
{
    gp_Pln plane;
    gp_Ax1 Normal;
    gp_Pnt ObjOrg;
    Base::Placement Place;

    // Calculate the number of references
    checkRefTypes();
    try {
    if((std::string(Type.getValueAsString()) == "Face")) {
        if(numFaces != 1)
            return new App::DocumentObjectExecReturn("A valid face must be provided");

        // Find a valid face to use for reference
        std::vector<App::PropertyLinkSub *> refs;
        const Part::Feature *refFaceFeat;

        bool originFnd = false;

        refs.push_back(&Entity1);
        refs.push_back(&Entity2);
        refs.push_back(&Entity3);

        bool Reverse = false;

        for (std::vector<App::PropertyLinkSub *>::const_iterator it=refs.begin(); it!=refs.end();++it) {
            if((*it)->getValue()) {
                Part::Feature *feat = static_cast<Part::Feature*>((*it)->getValue());
                if(!feat || !feat->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId()))
                  continue;

                const std::vector<std::string> &sub = (*it)->getSubValues();

                // Get the shape
                const Part::TopoShape &shape = feat->Shape.getShape();
                if (shape._Shape.IsNull())
                    continue;

                // Get the SubShape
                const TopoDS_Shape sh = shape.getSubShape(sub[0].c_str());

                if(sh.ShapeType() == TopAbs_VERTEX)
                {
                    TopoDS_Vertex vert = TopoDS::Vertex(sh);
                    if(vert.IsNull())
                        throw Base::Exception("The selected point is invalid");
                    ObjOrg = BRep_Tool::Pnt(vert);
                    originFnd = true;
                    continue;
                }
  

                if(sh.ShapeType() != TopAbs_FACE)
                  continue;

                // Try different geometry casts
                const TopoDS_Face &face = TopoDS::Face(sh);

                if(face.IsNull())
                    throw Base::Exception("An invalid face was generated");

                BRepAdaptor_Surface adapt( face );
                if (adapt.GetType() != GeomAbs_Plane)
                    throw Base::Exception("Only Planar Faces Can be Handled");

                // ------ Successfuly Found a Plane we can use ------- //
                if ((face).Orientation() == TopAbs_REVERSED)
                    Reverse = true;

                // Store the created plane from face and its supporting feature
                plane = adapt.Plane();
                refFaceFeat = feat;

                // Find the Center of the Face and use this
                if(!originFnd)
                {
                    GProp_GProps massProps;
                    BRepGProp::SurfaceProperties(sh, massProps);
                    ObjOrg = massProps.CentreOfMass();
                }
            }
        }

        if (!plane.Direct()) {
            // toggle if plane has a left-handed coordinate system
            plane.UReverse();
            Reverse = !Reverse;
        }

        Normal = plane.Axis();
        if (Reverse)
            Normal.Reverse();

        Place = refFaceFeat->Placement.getValue();

    } else if ((std::string(Type.getValueAsString()) == "Points")) {

        // All three points must be defined
        if(numVertices != 3)
          return new App::DocumentObjectExecReturn("Three valid vertices must be provded");

        // Find a valid face to use for reference
        std::vector<App::PropertyLinkSub *> refs;
        std::vector<Part::Feature *> feats;
        std::vector<TopoDS_Vertex> vertices;
        refs.push_back(&Entity1);
        refs.push_back(&Entity2);
        refs.push_back(&Entity3);

        for (std::vector<App::PropertyLinkSub *>::const_iterator it=refs.begin(); it!=refs.end();++it) {
            if((*it)->getValue()) {
                Part::Feature *feat = static_cast<Part::Feature*>((*it)->getValue());
                if(!feat || !feat->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId()))
                  return new App::DocumentObjectExecReturn("Valid features must be provided");

                const std::vector<std::string> &sub = (*it)->getSubValues();

                // Get the shape
                const Part::TopoShape &shape = feat->Shape.getShape();
                if (shape._Shape.IsNull())
                    continue;

                // Get the SubShape
                const TopoDS_Shape sh = shape.getSubShape(sub[0].c_str());

                if(sh.ShapeType() != TopAbs_VERTEX)
                    continue;

                TopoDS_Vertex vert = TopoDS::Vertex(sh);
                if(!vert.IsNull()) {
                    vertices.push_back(vert);
                    feats.push_back(feat);
                }
            }
        }

        if (vertices.size() != 3 || feats.size() != 3)
            throw Base::Exception("Three Valid Points were not provided");

        gp_Pnt pnt1 = BRep_Tool::Pnt(vertices[0]);
        gp_Pnt pnt2 = BRep_Tool::Pnt(vertices[1]);
        gp_Pnt pnt3 = BRep_Tool::Pnt(vertices[2]);

        plane = gce_MakePln(pnt1, pnt2, pnt3);
        Normal = plane.Axis();

        // Calcaulate Average ObjOrigin
        Base::Placement Place1 = feats[0]->Placement.getValue();
        Base::Placement Place2 = feats[1]->Placement.getValue();
        Base::Placement Place3 = feats[2]->Placement.getValue();

        // TODO need to figure a way to calculate the placement. 
        Place = Place1;
        float x = (Place1.getPosition().x + Place2.getPosition().x + Place3.getPosition().x) / 3.;
        float y = (Place1.getPosition().y + Place2.getPosition().y + Place3.getPosition().y) / 3.;
        float z = (Place1.getPosition().z + Place2.getPosition().z + Place3.getPosition().z) / 3.;

        ObjOrg = gp_Pnt(x,y,z);

    } else {
      return new App::DocumentObjectExecReturn("Invalid Mode was provided");
    }

    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        return new App::DocumentObjectExecReturn(e->GetMessageString());
    }

    // TODO Make a general function for this? //
    Handle (Geom_Plane) gPlane = new Geom_Plane(plane);
    GeomAPI_ProjectPointOnSurf projector(ObjOrg,gPlane);
    gp_Pnt BasePoint = projector.NearestPoint();

    gp_Dir dir = Normal.Direction();


    double offsetZ = OffsetZ.getValue();
    bool reversed = Reversed.getValue();
    if (offsetZ > Precision::Confusion()) {
        gp_Vec vec(dir);
        vec.Normalize();
        gp_Vec vec2(gp_Pnt(0.f, 0.f, 0.f), BasePoint);

        if (Reversed.getValue())
            offsetZ *= -1.f;

        vec2 = vec2 + vec * offsetZ;
        BasePoint = gp_Pnt(vec2.X(), vec2.Y(), vec2.Z());
    }
    gp_Ax3 BasePos;
    Base::Vector3d dX,dY,dZ;
    Place.getRotation().multVec(Base::Vector3d(1,0,0),dX);
    Place.getRotation().multVec(Base::Vector3d(0,1,0),dY);
    Place.getRotation().multVec(Base::Vector3d(0,0,1),dZ);
    gp_Dir dirX(dX.x, dX.y, dX.z);
    gp_Dir dirY(dY.x, dY.y, dY.z);
    gp_Dir dirZ(dZ.x, dZ.y, dZ.z);
    double cosNX = dir.Dot(dirX);
    double cosNY = dir.Dot(dirY);
    double cosNZ = dir.Dot(dirZ);
    std::vector<double> cosXYZ;
    cosXYZ.push_back(fabs(cosNX));
    cosXYZ.push_back(fabs(cosNY));
    cosXYZ.push_back(fabs(cosNZ));

    int pos = std::max_element(cosXYZ.begin(), cosXYZ.end()) - cosXYZ.begin();

    // +X/-X
    if (pos == 0) {
        if (cosNX > 0)
            BasePos = gp_Ax3(BasePoint, dir, dirY);
        else
            BasePos = gp_Ax3(BasePoint, dir, -dirY);
    }
    // +Y/-Y
    else if (pos == 1) {
        if (cosNY > 0)
            BasePos = gp_Ax3(BasePoint, dir, -dirX);
        else
            BasePos = gp_Ax3(BasePoint, dir, dirX);
    }
    // +Z/-Z
    else {
        BasePos = gp_Ax3(BasePoint, dir, dirX);
    }

    // Add the rotation
    double rotation = Rotation.getValue();
    if (fabs(rotation) > Precision::Confusion()) {
        rotation *= M_PI / 180.0;
        BasePos.Rotate(BasePos.Axis(),rotation);
    }
    // Add the offsets
    gp_Vec xDir(BasePos.XDirection());
    gp_Vec yDir(BasePos.YDirection());

    double offsetX = OffsetX.getValue();
    double offsetY = OffsetY.getValue();

    if (fabs(offsetX) > Precision::Confusion()) {
        BasePos.Translate(xDir * offsetX);
    }

    if (fabs(offsetY) > Precision::Confusion()) {
        BasePos.Translate(yDir * offsetY);
    }

    gp_Trsf Trf;
    Trf.SetTransformation(BasePos);
    Trf.Invert();

    Base::Matrix4D mtrx;

    gp_Mat m = Trf._CSFDB_Getgp_Trsfmatrix();
    gp_XYZ p = Trf._CSFDB_Getgp_Trsfloc();
    Standard_Real scale = 1.0;

    // set Rotation matrix
    mtrx[0][0] = scale * m._CSFDB_Getgp_Matmatrix(0,0);
    mtrx[0][1] = scale * m._CSFDB_Getgp_Matmatrix(0,1);
    mtrx[0][2] = scale * m._CSFDB_Getgp_Matmatrix(0,2);

    mtrx[1][0] = scale * m._CSFDB_Getgp_Matmatrix(1,0);
    mtrx[1][1] = scale * m._CSFDB_Getgp_Matmatrix(1,1);
    mtrx[1][2] = scale * m._CSFDB_Getgp_Matmatrix(1,2);

    mtrx[2][0] = scale * m._CSFDB_Getgp_Matmatrix(2,0);
    mtrx[2][1] = scale * m._CSFDB_Getgp_Matmatrix(2,1);
    mtrx[2][2] = scale * m._CSFDB_Getgp_Matmatrix(2,2);

    // set pos vector
    mtrx[0][3] = p._CSFDB_Getgp_XYZx();
    mtrx[1][3] = p._CSFDB_Getgp_XYZy();
    mtrx[2][3] = p._CSFDB_Getgp_XYZz();

    // check the angle against the Z Axis
    //Standard_Real a = Normal.Angle(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,0,1)));

    Placement.setValue(Base::Placement(mtrx));
    return App::DocumentObject::StdReturn;
}

void Plane::onChanged(const App::Property* prop)
{
//     if (prop == &Sketch) {
//         // if attached to a sketch then mark it as read-only
//         this->Placement.StatusBits.set(2, Sketch.getValue() != 0);
//     }

    Feature::onChanged(prop);
}
}
