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


#ifndef APP_MEASUREANGLE_H
#define APP_MEASUREANGLE_H

#include <App/DocumentObject.h>
#include <App/PropertyGeo.h>
#include <App/PropertyUnits.h>
#include <tuple>
#include <App/Measure.h>
#include <functional>
#include <string.h>
#include <map>

#include <Base/Vector3D.h>

#include <Mod/Measure/MeasureGlobal.h>


namespace Measure
{


struct MeasureAngleInfo {
    bool valid;
    Base::Vector3d orientation;
    Base::Vector3d position;
};


class MeasureExport MeasureAngle : public App::MeasurementBaseExtendable<MeasureAngleInfo>
{
    PROPERTY_HEADER_WITH_OVERRIDE(Measure::MeasureAngle);

public:
    /// Constructor
    MeasureAngle();
    ~MeasureAngle() override;

    App::PropertyLinkSub Element1;
    App::PropertyLinkSub Element2;
    App::PropertyAngle Angle;

    App::DocumentObjectExecReturn *execute() override;

    const char* getViewProviderName() const override {
        return "Gui::ViewProviderMeasureAngle";
    }

    static bool isValidSelection(const App::MeasureSelection& selection);
    static bool isPrioritizedSelection(const App::MeasureSelection& selection);
    void parseSelection(const App::MeasureSelection& selection);
    Base::Quantity result() {return Angle.getQuantityValue();}

    bool getVec(App::DocumentObject& ob, std::string& subName, Base::Vector3d& vecOut);
    Base::Vector3d getLoc(App::DocumentObject& ob, std::string& subName);

    // Orientation Vectors
    Base::Vector3d vector1();
    Base::Vector3d vector2();

    // Location Vectors
    Base::Vector3d location1();
    Base::Vector3d location2();

private:

    void onChanged(const App::Property* prop) override;
};

} //namespace Measure


#endif // APP_MEASUREANGLE_H
