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


#ifndef APP_MEASURE_H
#define APP_MEASURE_H

#include "DocumentObject.h"
#include "PropertyContainer.h"
#include "Application.h"
#include <Base/Quantity.h>

namespace App
{

class AppExport MeasurementBase : public DocumentObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(App::MeasurementBase);

public:

    boost::signals2::signal<void (const MeasurementBase*)> signalGuiUpdate;

    // Initalize measurement properties from selection
    virtual void parseSelection(const MeasureSelection&) = 0;

    virtual Base::Quantity result() = 0;
};




template <typename T>
class AppExport MeasurementBaseExtendable : public MeasurementBase
{

    using GeometryHandler = std::function<T (std::string*, std::string*)>;
    using HandlerMap = std::map<std::string, GeometryHandler>;


public: 

    static void addGeometryHandler(const std::string& module, GeometryHandler callback) {
        _mGeometryHandlers[module] = callback;
    }

    static GeometryHandler getGeometryHandler(const std::string& module) {

        if (!hasGeometryHandler(module)) {
            return {};
        }

        return _mGeometryHandlers[module];
    }

    static void addGeometryHandlers(const std::vector<std::string>& modules, GeometryHandler callback){
        // TODO: this will replace a callback with a later one.  Should we check that there isn't already a
        // handler defined for this module?
        for (auto& mod : modules) {
            _mGeometryHandlers[mod] = callback;
        }
    }


    static bool hasGeometryHandler(const std::string& module) {
        return (_mGeometryHandlers.count(module) > 0);
    }

private:
    static HandlerMap _mGeometryHandlers;
};

template <typename T>
typename MeasurementBaseExtendable<T>::HandlerMap MeasurementBaseExtendable<T>::_mGeometryHandlers = MeasurementBaseExtendable<T>::HandlerMap();


// What is this?
class AppExport Measure {

public:
    static void initialize();
};

} //namespace App


#endif // APP_MEASURE_H
