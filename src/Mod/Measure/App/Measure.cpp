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

#include <App/Application.h>
#include <Mod/Measure/MeasureGlobal.h>
#include "Measure.h"

#include "Base/Console.h"
#include <Base/Vector3D.h>
#include "App/Document.h"
#include "App/DocumentObject.h"

#include "MeasureAngle.h"
#include "MeasureDistance.h"
#include "MeasureLength.h"

#include <string>



namespace Measure {


void Measure::initialize() {

    App::Application& app = App::GetApplication();

    // Add Measure Types
    app.addMeasureType(
        new App::MeasureType {
            "ANGLE",
            "Angle",
            "Measure::MeasureAngle",
            MeasureAngle::isValidSelection,
            MeasureAngle::isPrioritizedSelection,
        });
        
    app.addMeasureType(
        new App::MeasureType {
            "DISTANCE",
            "Distance",
            "Measure::MeasureDistance",
            MeasureDistance::isValidSelection,
            MeasureDistance::isPrioritizedSelection,
    });

    app.addMeasureType(
        new App::MeasureType {
            "LENGTH",
            "Length",
            "Measure::MeasureLength",
            MeasureLength::isValidSelection,
            nullptr,
    });
}

}

