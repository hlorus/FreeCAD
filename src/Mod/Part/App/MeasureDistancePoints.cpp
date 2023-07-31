/***************************************************************************
 *   Copyright (c) 2008 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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


#include <BRep_Tool.hxx>
#include <Mod/Part/App/TopoShape.h>
#include <Mod/Part/App/PartFeature.h>
#include <App/PropertyContainer.h>
#include <App/Application.h>
#include <App/Document.h>
#include <tuple>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <ShapeAnalysis.hxx>
#include <TopExp_Explorer.hxx>

#include "MeasureDistancePoints.h"


using namespace Part;

PROPERTY_SOURCE(Part::MeasureDistancePoints, App::MeasurementBase)


App::DocumentObjectExecReturn* getShape(const App::PropertyLinkSub& link,
                                                      TopoDS_Shape& shape)
{
    App::DocumentObject* obj = link.getValue();
    const Part::TopoShape part = Part::Feature::getTopoShape(obj);
    if (part.isNull()) {
        return new App::DocumentObjectExecReturn("No shape linked.");
    }

    // if no explicit sub-shape is selected use the whole part
    const std::vector<std::string>& element = link.getSubValues();
    if (element.empty()) {
        shape = part.getShape();
        return nullptr;
    }
    else if (element.size() != 1) {
        return new App::DocumentObjectExecReturn("Not exactly one sub-shape linked.");
    }

    if (!part.getShape().IsNull()) {
        if (!element[0].empty()) {
            shape = part.getSubShape(element[0].c_str());
        }
        else {
            // the sub-element is an empty string, so use the whole part
            shape = part.getShape();
        }
    }

    return nullptr;
}


MeasureDistancePoints::MeasureDistancePoints()
{
    ADD_PROPERTY_TYPE(P1,(nullptr),"Measurement",App::Prop_None,"First point of measurement");
    P1.setScope(App::LinkScope::Global);
    P1.setAllowExternal(true);

    ADD_PROPERTY_TYPE(P2,(nullptr),"Measurement",App::Prop_None,"Second point of measurement");
    P2.setScope(App::LinkScope::Global);
    P2.setAllowExternal(true);

    ADD_PROPERTY_TYPE(Distance,(0.0)       ,"Measurement",App::PropertyType(App::Prop_ReadOnly|App::Prop_Output),
                                            "Distance between the points");

}

MeasureDistancePoints::~MeasureDistancePoints() = default;

bool MeasureDistancePoints::isValidSelection(const App::MeasureSelection& selection){

    if (selection.size() != 2) {
        return false;
    }

    App::Document* doc = App::GetApplication().getActiveDocument();
    App::DocumentObject* ob1 = doc->getObject(get<0>(selection.at(0)).c_str());
    Part::TopoShape sub1 = Part::Feature::getTopoShape(ob1).getSubShape(get<1>(selection.at(0)).c_str());
    
    App::DocumentObject* ob2 = doc->getObject(get<0>(selection.at(1)).c_str());
    Part::TopoShape sub2 = Part::Feature::getTopoShape(ob2).getSubShape(get<1>(selection.at(1)).c_str());
    

    if (sub1.shapeType() != TopAbs_VERTEX || sub2.shapeType() != TopAbs_VERTEX) {
        return false;
    }

    return true;
}

void MeasureDistancePoints::parseSelection(const App::MeasureSelection& selection) {
    // Set properties from selection, method is only invoked when isValid Selection return true

    App::Document* doc = App::GetApplication().getActiveDocument();
    App::DocumentObject* ob1 = doc->getObject(get<0>(selection.at(0)).c_str());
    App::DocumentObject* ob2 = doc->getObject(get<0>(selection.at(1)).c_str());

    P1.setValue(ob1, std::vector<std::string>(1, get<1>(selection.at(0))));
    P2.setValue(ob2, std::vector<std::string>(1, get<1>(selection.at(1))));
}


Base::Vector3f MeasureDistancePoints::getP1() {

    TopoDS_Shape shape;
    getShape(P1, shape);

    if (shape.IsNull() || shape.ShapeType() != TopAbs_VERTEX) {
        return Base::Vector3f();
    }

    gp_XYZ point = BRep_Tool::Pnt(TopoDS::Vertex(shape)).XYZ();

    return Base::Vector3f(point.X(), point.Y(), point.Z());
}


// TODO: Avoid redundancy
Base::Vector3f MeasureDistancePoints::getP2() {
    TopoDS_Shape shape;
    getShape(P2, shape);

    if (shape.IsNull() || shape.ShapeType() != TopAbs_VERTEX) {
        return Base::Vector3f();
    }

    gp_XYZ point = BRep_Tool::Pnt(TopoDS::Vertex(shape)).XYZ();

    return Base::Vector3f(point.X(), point.Y(), point.Z());
}


App::DocumentObjectExecReturn *MeasureDistancePoints::execute()
{
    App::DocumentObjectExecReturn* ret;

    TopoDS_Shape S1;
    ret = getShape(P1, S1);
        if (ret)
            return ret;

    TopoDS_Shape S2;
    ret = getShape(P2, S2);
        if (ret)
            return ret;

    // check for expected type
    if (S1.IsNull() || S2.IsNull())
        return new App::DocumentObjectExecReturn("Linked shapes are empty.");

    if (S1.ShapeType() != TopAbs_VERTEX || S2.ShapeType() != TopAbs_VERTEX)
        return new App::DocumentObjectExecReturn("Linked shape is not a vertex.");


    gp_Pnt p1 = BRep_Tool::Pnt(TopoDS::Vertex(S1));
    gp_Pnt p2 = BRep_Tool::Pnt(TopoDS::Vertex(S2));

    Distance.setValue(p1.Distance(p2));

    signalGuiUpdate(this);
    return DocumentObject::StdReturn;
}

void MeasureDistancePoints::onChanged(const App::Property* prop)
{
    if (prop == &P1 || prop == &P2) {
        if (!isRestoring()) {
            App::DocumentObjectExecReturn *ret = recompute();
            delete ret;
        }
    }
    DocumentObject::onChanged(prop);
}
