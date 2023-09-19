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
#include "Base/Console.h"
#include "Measure.h"
#include <App/MeasureLength.h>
#include <Mod/Measure/App/MeasureAngle.h>
#include <Mod/Measure/App/MeasureDistance.h>


using namespace Measure;

void PartDesign::Measure::initialize() {
    App::Application& app = App::GetApplication();
    const App::MeasureHandler& handler = app.getMeasureHandler("Part");

    // Note: This is not ideal, avoid having to pass along all Part geomerty handlers, PartDesign geometry should be the same

    app.addMeasureHandler("PartDesign", handler.infoCb, handler.typeCb);

//    App::MeasureLength::addGeometryHandler("PartDesign",
//                                           App::MeasureLength::getGeometryHandler("Part"));

//    MeasureAngle::addGeometryHandler("PartDesign",
//                                           MeasureAngle::getGeometryHandler("Part"));

//    MeasureDistance::addGeometryHandler("PartDesign", MeasureDistance::getGeometryHandler("Part"));
}

