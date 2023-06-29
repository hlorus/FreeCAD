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


#ifndef APP_MEASURELENGTH_H
#define APP_MEASURELENGTH_H

#include <App/DocumentObject.h>
#include <App/PropertyGeo.h>
#include <App/PropertyUnits.h>
#include <tuple>
#include "Measure.h"
#include <functional>
#include <string.h>
#include <map>

namespace App
{

using MeasureLengthGeometryHandler = std::function<float (std::string*, std::string*)>;
using HandlerMap = std::map<std::string, MeasureLengthGeometryHandler>;

typedef struct GeometryHandler {
    std::string module;
    MeasureLengthGeometryHandler callback;
} GeometryHandler;

class AppExport MeasureLength : public App::MeasurementBase
{
    PROPERTY_HEADER_WITH_OVERRIDE(App::MeasureLength);

public:
    /// Constructor
    MeasureLength();
    ~MeasureLength() override;

    App::PropertyLinkSubList Elements;
    App::PropertyDistance Distance;

    App::DocumentObjectExecReturn *execute() override;

    // const char* getViewProviderName() const override {
    //     return "Gui::ViewProviderMeasureDistance";
    // }

    static bool isValidSelection(const App::MeasureSelection& selection);
    void parseSelection(const App::MeasureSelection& selection);
    float result() {return Distance.getValue();}

    // Register geometry handler
    static void addGeometryHandler(const std::string& module, MeasureLengthGeometryHandler callback);
    static MeasureLengthGeometryHandler getGeometryHandler(const std::string& module);
    static bool hasGeometryHandler(const std::string& module);

private:
    static HandlerMap _mGeometryHandlers;

    void onChanged(const App::Property* prop) override;
};

} //namespace App


#endif // APP_MEASURELENGTH_H
