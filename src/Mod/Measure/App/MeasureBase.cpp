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

#include "PreCompiled.h"
#include <App/PropertyGeo.h>
#include <Base/PlacementPy.h>

#include "MeasureBase.h"
// Generated from MeasureBasePy.xml
#include "MeasureBasePy.h"

using namespace Measure;


PROPERTY_SOURCE(Measure::MeasureBase, App::DocumentObject)

MeasureBase::MeasureBase() {
    ADD_PROPERTY_TYPE(Placement, (Base::Placement()), nullptr, App::PropertyType(App::Prop_ReadOnly|App::Prop_Output|App::Prop_NoRecompute), "Visual placement of the measurement");

}

void MeasureBase::onDocumentRestored() {
    // Refresh the viewprovider
    signalGuiUpdate(this);
}


PyObject *MeasureBase::getPyObject(void)
{
    if (PythonObject.is(Py::_None())) {
        // ref counter is set to 1
        PythonObject = Py::Object(new MeasureBasePy(this), true);
    }
    return Py::new_reference_to(PythonObject);
}

Py::Object MeasureBase::getProxyObject() {
    Base::PyGILStateLocker lock;
    App::Property* prop = this->getPropertyByName("Proxy");
    if (!prop) {
        return Py::None();
    }
    return dynamic_cast<App::PropertyPythonObject*>(prop)->getValue();
};


void MeasureBase::parseSelection(const App::MeasureSelection& selection) {
    Base::PyGILStateLocker lock;
    
    Py::Object proxy = getProxyObject();

    // Convert selection to python list
    Py::Tuple selectionPy(selection.size());
    int i = 0;
    for (auto it : selection) {
        Py::Tuple sel(2);
        sel.setItem(0, Py::String(std::get<0>(it)));
        sel.setItem(1, Py::String(std::get<1>(it)));

        // selectionPy.append(sel);
        selectionPy.setItem(i, sel);
        i++;
    }

    // Call the parseSelection method of the proxy object
    Py::Tuple args(1);
    args.setItem(0, selectionPy);
    proxy.callMemberFunction("parseSelection", args);
    return;
}

QString MeasureBase::getResultString() {
    Py::Object proxy = getProxyObject();
    Base::PyGILStateLocker lock;
 
    if (!proxy.isNone()) {
        auto ret = proxy.callMemberFunction("getResultString");
        return QString::fromStdString(ret.as_string());
    }

    App::Property* prop = getResultProp();
        if (prop == nullptr) {
            return QString();
        }

    if (prop->isDerivedFrom(App::PropertyQuantity::getClassTypeId())) {
        return static_cast<App::PropertyQuantity*>(prop)->getQuantityValue().getUserString();
    }


    return QString();
}
Base::Placement MeasureBase::getPlacement() {
    return this->Placement.getValue();
}
// Python Drawing feature ---------------------------------------------------------


#include <App/FeaturePythonPyImp.h>

namespace App {
/// @cond DOXERR
PROPERTY_SOURCE_TEMPLATE(Measure::MeasurePython, Measure::MeasureBase)
template<> const char* Measure::MeasurePython::getViewProviderName(void) const {
    return "MeasureGui::ViewProviderMeasurePropertyBase";
}
template<> PyObject* Measure::MeasurePython::getPyObject() {
    if (PythonObject.is(Py::_None())) {
        // ref counter is set to 1
        PythonObject = Py::Object(new FeaturePythonPyT<Measure::MeasureBasePy>(this),true);
    }
    return Py::new_reference_to(PythonObject);
}
/// @endcond

// explicit template instantiation
template class MeasureExport FeaturePythonT<Measure::MeasureBase>;
}
