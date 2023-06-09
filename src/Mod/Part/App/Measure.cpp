/***************************************************************************
 *   Copyright (c) 2022 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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

#include <App/Application.h>
#include <Mod/Part/PartGlobal.h>
#include "Measure.h"
#include "Base/Console.h"
#include "App/Document.h"
#include "App/DocumentObject.h"
#include "PrimitiveFeature.h"
#include "PartFeature.h"
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>


#include <TopAbs.hxx>
#include "Base/Vector3D.h"
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>

#include <string>


// From: https://github.com/Celemation/FreeCAD/blob/joel_selection_summary_demo/src/Gui/SelectionSummary.cpp

// Should work with edges and wires
static float getLength(TopoDS_Shape wire){
    GProp_GProps gprops;
    BRepGProp::LinearProperties(wire, gprops);
    return gprops.Mass();
}

static float getFaceArea(TopoDS_Shape face){
    GProp_GProps gprops;
    BRepGProp::SurfaceProperties(face, gprops);
    return gprops.Mass();
}


static App::MeasureElementInfo PartMeasureCb(const char* obName, const char* subName) {

    App::DocumentObject* ob = App::GetApplication().getActiveDocument()->getObject(obName);
    auto sub = ob->getSubObject(subName);

    TopoDS_Shape shape = Part::Feature::getShape(ob, subName, true);
    TopAbs_ShapeEnum sType = shape.ShapeType();

    std::string type = "";
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
        .type = type,
        .pos = pos,
        .length = length,
        .area = area
    };
    
    return info;
}

namespace Part {


void Measure::initialize() {

    App::GetApplication().addMeasureHandler("Part", PartMeasureCb);

}

}

