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
# include <sstream>
# include <QApplication>
# include <Inventor/nodes/SoAnnotation.h>
# include <Inventor/nodes/SoBaseColor.h>
# include <Inventor/nodes/SoCoordinate3.h>
# include <Inventor/nodes/SoDrawStyle.h>
# include <Inventor/nodes/SoFontStyle.h>
# include <Inventor/nodes/SoIndexedLineSet.h>
# include <Inventor/nodes/SoMarkerSet.h>
# include <Inventor/nodes/SoPickStyle.h>
# include <Inventor/nodes/SoText2.h>
# include <Inventor/nodes/SoTranslation.h>
#endif

#include <App/Document.h>
#include <Base/Console.h>
#include <Base/Quantity.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/ViewParams.h>
#include <Gui/Inventor/MarkerBitmaps.h>

#include <Mod/Measure/App/MeasureLength.h>
#include <Mod/Measure/App/Preferences.h>

#include "ViewProviderMeasureLength.h"

using namespace Gui;
using namespace MeasureGui;
using namespace Measure;

PROPERTY_SOURCE(MeasureGui::ViewProviderMeasureLength, MeasureGui::ViewProviderMeasurePropertyBase)


//! handle changes to the feature's properties
void ViewProviderMeasureLength::updateData(const App::Property* prop)
{
    if (pcObject == nullptr) {
        return;
    }

    auto obj = dynamic_cast<Measure::MeasureLength*>(getMeasureObject());
    if (prop == &(obj->Elements) || prop == &(obj->Length)){
        {
            redrawAnnotation();
        }
    }


    ViewProviderMeasurePropertyBase::updateData(prop);
}


Base::Vector3d ViewProviderMeasureLength::getTextPosition(){
    auto measureObject = dynamic_cast<Measure::MeasureLength*>(getMeasureObject());

    auto basePoint = getBasePosition();
    double length = measureObject->Length.getValue();
    if (Mirror.getValue()) {
        length = -length;
    }
    Base::Placement placement = measureObject->getPlacement();
    Base::Vector3d textDirection = getTextDirection(placement.getRotation().multVec(Base::Vector3d(0.0, 0.0, 1.0)));

    // without the fudgeFactor, the text is too far away sometimes.  Not a big deal if we can drag the text.
    double fudgeFactor(0.5);
    return basePoint + textDirection * length * fudgeFactor * DistFactor.getValue();
}

