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


#ifndef APP_MEASUREDISTANCEPOINTS_H
#define APP_MEASUREDISTANCEPOINTS_H

#include <App/DocumentObject.h>
#include <App/PropertyGeo.h>
#include <App/PropertyUnits.h>
#include <tuple>
#include <App/Measure.h>

namespace Part
{


class PartExport MeasureDistancePoints : public App::MeasurementBase
{
    PROPERTY_HEADER_WITH_OVERRIDE(Part::MeasureDistancePoints);

public:
    /// Constructor
    MeasureDistancePoints();
    ~MeasureDistancePoints() override;

    App::PropertyLinkSub P1;
    App::PropertyLinkSub P2;

    App::PropertyDistance Distance;


    static bool isValidSelection(const App::MeasureSelection& selection);
    void parseSelection(const App::MeasureSelection& selection);

    App::DocumentObjectExecReturn *execute() override;


    float result() {return Distance.getValue();}

protected:
    void onChanged(const App::Property* prop) override;
};

} //namespace Part


#endif // APP_MEASUREDISTANCEPOINTS_H
