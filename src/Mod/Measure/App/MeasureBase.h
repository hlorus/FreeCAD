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


#ifndef MEASURE_MEASUREBASE_H
#define MEASURE_MEASUREBASE_H

#include <Mod/Measure/MeasureGlobal.h>

#include <App/Application.h>
#include <App/DocumentObject.h>
#include <App/PropertyStandard.h>
#include <App/PropertyUnits.h>
#include <Base/Quantity.h>
#include <Base/Placement.h>
#include <QString>


// TODO: this is the base for the MeasureXXXXX classes.  It should be renamed to MeasureBase and moved to Mod/Measure/App
namespace Measure
{

class AppExport MeasureBase : public App::DocumentObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(Measure::MeasureBase);

public:
    MeasureBase() = default;
    ~MeasureBase() override = default;

    boost::signals2::signal<void (const MeasureBase*)> signalGuiUpdate;

    // Initalize measurement properties from selection
    virtual void parseSelection(const App::MeasureSelection&) = 0;  // pure abstract, implement in derived classes

    virtual App::Property* getResultProp() = 0;
    virtual QString getResultString();
    virtual Base::Placement getPlacement() {return Base::Placement();}

protected:
    void onDocumentRestored() override;
    virtual App::DocumentObject* getSubject() const = 0;
};


template <typename T>
class AppExport MeasureBaseExtendable : public MeasureBase
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
typename MeasureBaseExtendable<T>::HandlerMap MeasureBaseExtendable<T>::_mGeometryHandlers = MeasureBaseExtendable<T>::HandlerMap();


} //namespace Measure


#endif // MEASURE_MEASUREBASE_H
