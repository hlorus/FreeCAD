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


#ifndef APP_MEASUREPOSITION_H
#define APP_MEASUREPOSITION_H

#include <App/DocumentObject.h>
#include <App/PropertyGeo.h>
#include <App/PropertyUnits.h>
#include <tuple>
#include <App/Measure.h>
#include <functional>
#include <string.h>
#include <map>

#include <Mod/Measure/MeasureGlobal.h>


namespace Measure
{


class MeasureExport MeasurePosition : public App::MeasurementBaseExtendable<float>
{
    PROPERTY_HEADER_WITH_OVERRIDE(Measure::MeasurePosition);

public:
    /// Constructor
    MeasurePosition();
    ~MeasurePosition() override;

    App::PropertyLinkSubList Elements;
    App::PropertyVector Position;

    App::DocumentObjectExecReturn *execute() override;
    void recalculatePosition();

    // const char* getViewProviderName() const override {
    //     return "Gui::ViewProviderMeasurePosition";
    // }

    static bool isValidSelection(const App::MeasureSelection& selection);
    void parseSelection(const App::MeasureSelection& selection);
    
    // Note: How should we display a vector?
    // Allow multiple result values along with labels? X, Y, Z
    // Just return a string?
    Base::Quantity result() {return Base::Quantity();}

private:

    void onChanged(const App::Property* prop) override;
};

} //namespace Measure


#endif // APP_MEASUREPOSITION_H
