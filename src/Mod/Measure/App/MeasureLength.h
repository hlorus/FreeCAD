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


#ifndef MEASURE_MEASURELENGTH_H
#define MEASURE_MEASURELENGTH_H

#include <Mod/Measure/MeasureGlobal.h>

#include <functional>
#include <string.h>
#include <map>
#include <tuple>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pnt.hxx>
#include <TopExp.hxx>

#include <App/PropertyLinks.h>
#include <App/PropertyUnits.h>
#include <App/GeoFeature.h>

#include <Mod/Part/App/TopoShape.h>

#include "MeasureBase.h"

namespace Measure
{


class MeasureExport MeasureLength : public Measure::MeasureBaseExtendable<float>
{
    PROPERTY_HEADER_WITH_OVERRIDE(Measure::MeasureLength);

public:
    /// Constructor
    MeasureLength();
    ~MeasureLength() override;

    App::PropertyLinkSubList Elements;
    App::PropertyDistance Length;

    App::DocumentObjectExecReturn *execute() override;
    void recalculateLength();

    const char* getViewProviderName() const override {
        return "MeasureGui::ViewProviderMeasureLength";
    }

    static bool isValidSelection(const App::MeasureSelection& selection);
    void parseSelection(const App::MeasureSelection& selection) override;

    App::PropertyQuantity* getResultProp() override {return &this->Length;}
    Base::Quantity result() override {return Length.getQuantityValue();}

    std::pair<Base::Vector3d, Base::Vector3d> getEndPoints() const;

private:
    void onChanged(const App::Property* prop) override;

    TopoDS_Shape getSubShape(App::DocumentObject* object, std::string subName) const;

};

} //namespace Measure


#endif // MEASURE_MEASURELENGTH_H

