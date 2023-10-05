/***************************************************************************
 *   Copyright (c) 2023 WandererFan <wandererfan@gmail.com>                *
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

#include <tuple>

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepLProp_CLProps.hxx>

#include <App/Application.h>
#include <App/Document.h>

#include <Mod/Part/App/PartFeature.h>

#include "MeasureRadius.h"


using namespace Measure;

PROPERTY_SOURCE(Measure::MeasureRadius, Measure::MeasureBase)



MeasureRadius::MeasureRadius()
{
    ADD_PROPERTY_TYPE(Elements,(nullptr), "Measurement", App::Prop_None, "Elements to get the radius from");
    Elements.setScope(App::LinkScope::Global);
    Elements.setAllowExternal(true);

    ADD_PROPERTY_TYPE(Radius,(0.0)       ,"Measurement",App::PropertyType(App::Prop_ReadOnly|App::Prop_Output),
                                            "Radius of selection");

}

MeasureRadius::~MeasureRadius() = default;

//! validate all the object+subelement pairs in the selection. Must be circle or arc
//! and have a geometry handler available.
//?? should we validate every entry?  Any entry?
//?? start with just checking the first entry
bool MeasureRadius::isValidSelection(const App::MeasureSelection& selection){

    if (selection.empty()) {
        return false;
    }

    App::Document* doc = App::GetApplication().getActiveDocument();

    for (const std::tuple<std::string, std::string>& element : selection) {
        const std::string& obName = get<0>(element);
        App::DocumentObject* ob = doc->getObject(obName.c_str());
        
        const std::string& subName = get<1>(element);
        const char* className = ob->getSubObject(subName.c_str())->getTypeId().getName();
        std::string mod = ob->getClassTypeId().getModuleName(className);

        if (!hasGeometryHandler(mod)) {
            return false;
        }

        App::MeasureHandler handler = App::GetApplication().getMeasureHandler(mod.c_str());
        App::MeasureElementType type = handler.typeCb(obName.c_str(), subName.c_str());

        if (type == App::MeasureElementType::INVALID) {
            return false;
        }

        if (type != App::MeasureElementType::CIRCLE
              && type != App::MeasureElementType::ARC) {
            return false;
        }
    }
    return true;
}

//! return true if the selection is particularly interesting to MeasureRadius.
//! In this case we claim circles and arcs.
bool MeasureRadius::isPrioritizedSelection(const App::MeasureSelection& selection) {
    if (selection.empty()) {
        return false;
    }
    App::Document* doc = App::GetApplication().getActiveDocument();
    std::string firstObjectName = get<0>(selection.front());
    App::DocumentObject* firstObject = doc->getObject(firstObjectName.c_str());
    std::string firstSubName = get<1>(selection.front());
    if (!firstObject) {
        return false;
    }
    TopoDS_Shape firstShape = Part::Feature::getTopoShape(firstObject).getSubShape(firstSubName.c_str());
    TopoDS_Edge firstEdge;
    if (firstShape.ShapeType() == TopAbs_EDGE) {
        firstEdge = TopoDS::Edge(firstShape);
    } else if (firstShape.ShapeType() == TopAbs_WIRE) {
        TopoDS_Wire firstWire = TopoDS::Wire(firstShape);
        TopExp_Explorer edges(firstWire, TopAbs_EDGE);
        if (edges.More()) {
            firstEdge = TopoDS::Edge(edges.Current());
        }
    } else {
        // TODO: only edge or wire can have radius?
        return false;
    }

    BRepAdaptor_Curve adapt(firstEdge);

    if (selection.size() == 1  &&
        adapt.GetType() == GeomAbs_Circle) {
        return true;
    }

    return false;
}


//! Set properties from selection. assumes a valid selection.
void MeasureRadius::parseSelection(const App::MeasureSelection& selection) {
    App::Document* doc = App::GetApplication().getActiveDocument();

    std::vector<App::DocumentObject*> objects;
    std::vector<const char*> subElements;

    for (const std::tuple<std::string, std::string>& element : selection) {
       App::DocumentObject* ob = doc->getObject(get<0>(element).c_str());
       objects.push_back(ob);
       subElements.push_back(get<1>(element).c_str());
    }

    Elements.setValues(objects, subElements);
}


App::DocumentObjectExecReturn *MeasureRadius::execute()
{
    recalculateRadius();
    return DocumentObject::StdReturn;
}


// TODO: does it make sense to sum radii here? to average them? to only take the first circular
// edge in the selection?
void MeasureRadius::recalculateRadius()
{
    const std::vector<App::DocumentObject*>& objects = Elements.getValues();
    const std::vector<std::string>& subElements = Elements.getSubValues();

    App::DocumentObject *object = objects.front();
    std::string subElement = subElements.front();

    // Get the Geometry handler based on the module an this type of measurement (radius)
    const char* className = object->getSubObject(subElement.c_str())->getTypeId().getName();
    const std::string& mod = object->getClassTypeId().getModuleName(className);
    auto handler = getGeometryHandler(mod);
    if (!handler) {
       throw Base::RuntimeError("No geometry handler available for submitted element type");
    }

    std::string objectName = object->getNameInDocument();
    double result = handler(&objectName, &subElement).radius;

    Radius.setValue(result);
}

void MeasureRadius::onChanged(const App::Property* prop)
{
    if (isRestoring() || isRemoving()) {
        return;
    }

    if (prop == &Elements) {
        recalculateRadius();
    }
    
    MeasureBase::onChanged(prop);
}

//! return a placement (location + orientation) for the first element
Base::Placement MeasureRadius::getPlacement() {
    const std::vector<App::DocumentObject*>& objects = Elements.getValues();
    const std::vector<std::string>& subElements = Elements.getSubValues();

    if (objects.empty() || subElements.empty()) {
        return Base::Placement();
    }

    App::DocumentObject* object = objects.front();
    std::string subElement = subElements.front();
    const char* className = object->getSubObject(subElement.c_str())->getTypeId().getName();
    const std::string& mod = object->getClassTypeId().getModuleName(className);

    auto handler = getGeometryHandler(mod);
    if (!handler) {
        throw Base::RuntimeError("No geometry handler available for submitted element type");
    }

    std::string obName = object->getNameInDocument();
    return handler(&obName, &subElement).placement;
}

//! return the pointOnCurve element in MeasureRadiusInfo for the first element
Base::Vector3d MeasureRadius::getPointOnCurve() const
{
    // TODO:: GeometryHandler* MeasureRadius::getFirstElementInfoFromHandler() {
    const std::vector<App::DocumentObject*>& objects = Elements.getValues();
    const std::vector<std::string>& subElements = Elements.getSubValues();

    if (objects.empty() || subElements.empty()) {
        return Base::Vector3d();
    }

    App::DocumentObject* object = objects.front();
    std::string subElement = subElements.front();
    const char* className = object->getSubObject(subElement.c_str())->getTypeId().getName();
    const std::string& mod = object->getClassTypeId().getModuleName(className);

    auto handler = getGeometryHandler(mod);
    if (!handler) {
        throw Base::RuntimeError("No geometry handler available for submitted element type");
    }
    // return handler;
    // end of getFirstFromHandler

    std::string obName = object->getNameInDocument();
    return handler(&obName, &subElement).pointOnCurve;
}
