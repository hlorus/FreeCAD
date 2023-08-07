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

#include <App/PropertyContainer.h>
#include <App/Application.h>
#include <App/Document.h>
#include <tuple>

#include "MeasureLength.h"


using namespace App;

PROPERTY_SOURCE(App::MeasureLength, App::MeasurementBase)



MeasureLength::MeasureLength()
{
    ADD_PROPERTY_TYPE(Elements,(nullptr), "Measurement", App::Prop_None, "Elements to get the length from");
    Elements.setScope(App::LinkScope::Global);
    Elements.setAllowExternal(true);

    ADD_PROPERTY_TYPE(Distance,(0.0)       ,"Measurement",App::PropertyType(App::Prop_ReadOnly|App::Prop_Output),
                                            "Distance between the points");

}

MeasureLength::~MeasureLength() = default;


bool MeasureLength::isValidSelection(const App::MeasureSelection& selection){

    if (selection.size() != 1) {
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

        if (!(type == App::MeasureElementType::LINE || type == App::MeasureElementType::CIRCLE
              || type == App::MeasureElementType::ARC || type == App::MeasureElementType::CURVE)) {
            return false;
        }
    }
    return true;
}

void MeasureLength::parseSelection(const App::MeasureSelection& selection) {
    // Set properties from selection, method is only invoked when isValid Selection returns true

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


App::DocumentObjectExecReturn *MeasureLength::execute()
{
    const std::vector<App::DocumentObject*>& objects = Elements.getValues();
    const std::vector<std::string>& subElements = Elements.getSubValues();

    float result;

    // Loop through Elements and call the valid geometry handler
    for (std::vector<App::DocumentObject*>::size_type i=0; i<objects.size(); i++) {
        App::DocumentObject *object = objects.at(i);
        std::string subElement = subElements.at(i);

        // Get the Geometry handler based on the module
        const char* className = object->getSubObject(subElement.c_str())->getTypeId().getName();
        const std::string& mod = object->getClassTypeId().getModuleName(className);
        auto handler = getGeometryHandler(mod);
        if (!handler) {
            return new App::DocumentObjectExecReturn("No geometry handler available for submitted element type");
        }

        std::string obName = object->getNameInDocument();
        result += handler(&obName, &subElement);
    }

    Distance.setValue(result);

    signalGuiUpdate(this);
    return DocumentObject::StdReturn;
}

void MeasureLength::onChanged(const App::Property* prop)
{
    if (prop == &Elements) {
        if (!isRestoring()) {
            App::DocumentObjectExecReturn *ret = recompute();
            delete ret;
        }
    }
    DocumentObject::onChanged(prop);
}
