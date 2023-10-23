/***************************************************************************
 *   Copyright (c) 2023 Wanderer Fan <wandererfan@gmail.com>               *
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

#ifndef _PreComp_
#endif

#include <App/Document.h>
#include <App/Property.h>
#include <Base/Console.h>

#include <Mod/Measure/App/MeasureArea.h>

#include "ViewProviderMeasureArea.h"

using namespace Gui;
using namespace MeasureGui;
using namespace Measure;

PROPERTY_SOURCE(MeasureGui::ViewProviderMeasureArea, MeasureGui::ViewProviderMeasurePropertyBase)


//! handle changes to the feature's properties
void ViewProviderMeasureArea::updateData(const App::Property* prop)
{
    if (pcObject == nullptr) {
        return;
    }

    auto obj = dynamic_cast<Measure::MeasureArea*>(getMeasureObject());

    if (obj &&
        prop == &(obj->Elements) ) {
        connectToSubject(obj->getSubject());
        redrawAnnotation();
    }

    if (obj &&
        prop == &(obj->Area)){
        redrawAnnotation();
    }

    ViewProviderMeasurePropertyBase::updateData(prop);
}

