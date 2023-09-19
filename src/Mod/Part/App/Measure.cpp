/***************************************************************************
 *   Copyright (c) 2023 David Friedli <david[at]friedli-be.ch>             *
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

#include <Mod/Part/PartGlobal.h>

#include <string>

#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopAbs.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <TopExp.hxx>
#include <GProp_GProps.hxx>

#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Base/Console.h>
#include <Base/Vector3D.h>

//#include "App/MeasureLength.h"
#include <Mod/Measure/App/MeasureAngle.h>
#include <Mod/Measure/App/MeasureDistance.h>
#include <Mod/Measure/App/MeasureLength.h>

#include "VectorAdapter.h"
//#include "PrimitiveFeature.h"
#include "PartFeature.h"

#include "Measure.h"

// From: https://github.com/Celemation/FreeCAD/blob/joel_selection_summary_demo/src/Gui/SelectionSummary.cpp

// Should work with edges and wires
static float getLength(TopoDS_Shape& wire){
    GProp_GProps gprops;
    BRepGProp::LinearProperties(wire, gprops);
    return gprops.Mass();
}

static float getFaceArea(TopoDS_Shape& face){
    GProp_GProps gprops;
    BRepGProp::SurfaceProperties(face, gprops);
    return gprops.Mass();
}


static App::MeasureElementInfo PartMeasureCb(const char* objectName, const char* subName) {

    App::DocumentObject* ob = App::GetApplication().getActiveDocument()->getObject(objectName);
//    auto sub = ob->getSubObject(subName);

    TopoDS_Shape shape = Part::Feature::getShape(ob, subName, true);
    if (shape.IsNull()) {
        // failure here on loading document with existing measurement.
        Base::Console().Message("Part::VectorAdapter did not retrieve shape for %s, %s\n", objectName, subName);
        return App::MeasureElementInfo();
    }

    TopAbs_ShapeEnum sType = shape.ShapeType();

    std::string type;
    Base::Vector3d pos;
    float length = 0.0;
    float area = 0.0;

    if(sType == TopAbs_VERTEX) {
        type = "Point";

        TopoDS_Vertex vertex = TopoDS::Vertex(shape);    
        auto point = BRep_Tool::Pnt(vertex);
        pos.Set(point.X(), point.Y(), point.Z());
    
    }else if (sType == TopAbs_EDGE) {

        type = "Edge";
        length = getLength(shape);
    }else if (sType == TopAbs_FACE) {
        type = "Face";
        area = getFaceArea(shape);
    }
     

    App::MeasureElementInfo info = {
        type,
        pos,
        length,
        area
    };
    
    return info;
}

App::MeasureElementType PartMeasureTypeCb(const char* objectName, const char* subName) {
    App::DocumentObject* ob = App::GetApplication().getActiveDocument()->getObject(objectName);
//    auto sub = ob->getSubObject(subName);

    TopoDS_Shape shape = Part::Feature::getShape(ob, subName, true);
    if (shape.IsNull()) {
        // failure here on loading document with existing measurement.
        Base::Console().Message("Part::VectorAdapter did not retrieve shape for %s, %s\n", objectName, subName);
        return App::MeasureElementType();
    }
    TopAbs_ShapeEnum shapeType = shape.ShapeType();

    switch (shapeType) {
        case TopAbs_VERTEX: {
            return App::MeasureElementType::POINT;
        }
        case TopAbs_EDGE: {
            const TopoDS_Edge& edge = TopoDS::Edge(shape);
            BRepAdaptor_Curve curve(edge);

            switch (curve.GetType()) {
                case GeomAbs_Line: { return App::MeasureElementType::LINE; }
                case GeomAbs_Circle: { return App::MeasureElementType::CIRCLE; }
                case GeomAbs_BezierCurve:
                case GeomAbs_BSplineCurve: {
                    return App::MeasureElementType::CURVE;
                }
                default: { return App::MeasureElementType::INVALID; }
            }
        }
        case TopAbs_FACE: {
            const TopoDS_Face& face = TopoDS::Face(shape);
            BRepAdaptor_Surface surface(face);

            switch (surface.GetType()) {
                case GeomAbs_Cylinder: { return App::MeasureElementType::CYLINDER; }
                case GeomAbs_Plane: { return App::MeasureElementType::PLANE; }
                default: { return App::MeasureElementType::INVALID; }
            }
        }
        case TopAbs_SOLID: {
            return App::MeasureElementType::Volume;
        }
        default: {
            return App::MeasureElementType::INVALID;
        }
    }
}



bool getShapeFromStrings(TopoDS_Shape &shapeOut, const std::string &doc, const std::string &object, const std::string &sub, Base::Matrix4D *mat)
{
  App::Document *docPointer = App::GetApplication().getDocument(doc.c_str());
  if (!docPointer) {
    return false;
  }
  App::DocumentObject *objectPointer = docPointer->getObject(object.c_str());
  if (!objectPointer) {
    return false;
  }
  shapeOut = Part::Feature::getShape(objectPointer,sub.c_str(),true,mat);
  return !shapeOut.IsNull();
}



Part::VectorAdapter buildAdapter(const App::DocumentObject* ob, std::string* objectName, const std::string* subName)
{
    (void) objectName;
    Base::Matrix4D mat;
    TopoDS_Shape shape = Part::Feature::getShape(ob, subName->c_str(), true);
    if (shape.IsNull()) {
        // failure here on loading document with existing measurement.
        Base::Console().Message("Part::VectorAdapter did not retrieve shape for %s, %s\n", objectName->c_str(), subName->c_str());
        return Part::VectorAdapter();
    }

    TopAbs_ShapeEnum shapeType = shape.ShapeType();


    if (shapeType == TopAbs_EDGE)
    {
      TopoDS_Shape edgeShape;
      if (!getShapeFromStrings(edgeShape, ob->getDocument()->getName(), ob->getNameInDocument(), *subName, &mat)) {
        return {};
      }
      TopoDS_Edge edge = TopoDS::Edge(edgeShape);
      // make edge orientation so that end of edge closest to pick is head of vector.
      TopoDS_Vertex firstVertex = TopExp::FirstVertex(edge, Standard_True);
      TopoDS_Vertex lastVertex = TopExp::LastVertex(edge, Standard_True);
      if (firstVertex.IsNull() || lastVertex.IsNull()) {
        return {};
      }
      gp_Vec firstPoint = Part::VectorAdapter::convert(firstVertex);
      gp_Vec lastPoint = Part::VectorAdapter::convert(lastVertex);
      Base::Vector3d v(0.0, 0.0, 0.0); //v(current.x,current.y,current.z);
      v = mat*v;
      gp_Vec pickPoint(v.x, v.y, v.z);
      double firstDistance = (firstPoint - pickPoint).Magnitude();
      double lastDistance = (lastPoint - pickPoint).Magnitude();
      if (lastDistance > firstDistance)
      {
        if (edge.Orientation() == TopAbs_FORWARD) {
          edge.Orientation(TopAbs_REVERSED);
        }
        else {
          edge.Orientation(TopAbs_FORWARD);
        }
      }
      return {edge, pickPoint};
    }
    if (shapeType == TopAbs_FACE)
    {
      TopoDS_Shape faceShape;
      if (!getShapeFromStrings(faceShape, ob->getDocument()->getName(), ob->getNameInDocument(), *subName, &mat)) {
        return {};
      }

      TopoDS_Face face = TopoDS::Face(faceShape);
      Base::Vector3d vTemp(0.0, 0.0, 0.0); //v(current.x, current.y, current.z);
      vTemp = mat*vTemp;
      gp_Vec pickPoint(vTemp.x, vTemp.y, vTemp.z);
      return {face, pickPoint};
    }

    return {};
}


float MeasureLengthHandler(std::string* objectName, std::string* subName){
    App::DocumentObject* ob = App::GetApplication().getActiveDocument()->getObject(objectName->c_str());

    TopoDS_Shape shape = Part::Feature::getShape(ob, subName->c_str(), true);
    if (shape.IsNull()) {
        // failure here on loading document with existing measurement.
        Base::Console().Message("MeasureLengthHandler did not retrieve shape for %s, %s\n", objectName->c_str(), subName->c_str());
        return 0.0;
    }
    TopAbs_ShapeEnum sType = shape.ShapeType();

    if (sType != TopAbs_EDGE) {
        return 0.0;
    }

    return getLength(shape);
}


Measure::MeasureAngleInfo MeasureAngleHandler(std::string* objectName, std::string* subName) {
    App::DocumentObject* ob = App::GetApplication().getActiveDocument()->getObject(objectName->c_str());
    TopoDS_Shape shape = Part::Feature::getShape(ob, subName->c_str(), true);
    if (shape.IsNull()) {
        // failure here on loading document with existing measurement.
        Base::Console().Message("MeasureAngleHandler did not retrieve shape for %s, %s\n", objectName->c_str(), subName->c_str());
        return Measure::MeasureAngleInfo();
    }

    TopAbs_ShapeEnum sType = shape.ShapeType();

    Part::VectorAdapter vAdapt = buildAdapter(ob, objectName, subName);

    gp_Pnt vec;
    Base::Vector3d position;
    if (sType == TopAbs_FACE) {
        TopoDS_Face face = TopoDS::Face(shape);
        
        GProp_GProps gprops;
        BRepGProp::SurfaceProperties(face, gprops);
        vec = gprops.CentreOfMass();
        
    } else if (sType == TopAbs_EDGE) {
        TopoDS_Edge edge = TopoDS::Edge(shape);

        GProp_GProps gprops;
        BRepGProp::LinearProperties(edge, gprops);
        vec = gprops.CentreOfMass();
    }

    position.Set(vec.X(), vec.Y(), vec.Z());

    Measure::MeasureAngleInfo info = {vAdapt.isValid(), (Base::Vector3d)vAdapt, position};
    return info;
}


Measure::MeasureDistanceInfo MeasureDistanceHandler(std::string* objectName, std::string* subName) {
    App::DocumentObject* ob = App::GetApplication().getActiveDocument()->getObject(objectName->c_str());
    TopoDS_Shape shape = Part::Feature::getShape(ob, subName->c_str(), true);
    if (shape.IsNull()) {
        // failure here on loading document with existing measurement.
        Base::Console().Message("MeasureDistanceHandler did not retrieve shape for %s, %s\n", objectName->c_str(), subName->c_str());
        return Measure::MeasureDistanceInfo();
    }

    TopAbs_ShapeEnum sType = shape.ShapeType();

    // Just return the TopoDS_Shape here
    return {true, shape};
}


using namespace Measure;

void Part::Measure::initialize() {

    App::Application& app = App::GetApplication();
    app.addMeasureHandler("Part", PartMeasureCb, PartMeasureTypeCb);


    std::vector<std::string> proxyList(  { "Part", "PartDesign" } );
    // Extend MeasureLength
    MeasureLength::addGeometryHandlers(proxyList, MeasureLengthHandler);

    // Extend MeasureAngle
    MeasureAngle::addGeometryHandlers(proxyList, MeasureAngleHandler);

    // Extend MeasureDistance
    MeasureDistance::addGeometryHandlers(proxyList, MeasureDistanceHandler);
}
