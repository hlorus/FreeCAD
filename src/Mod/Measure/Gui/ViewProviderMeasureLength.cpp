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

PROPERTY_SOURCE(MeasureGui::ViewProviderMeasureLength, MeasureGui::ViewProviderMeasureBase)


ViewProviderMeasureLength::ViewProviderMeasureLength()
{
    static const char *agroup = "Appearance";
    ADD_PROPERTY_TYPE(DistFactor,(Preferences::defaultDistFactor()), agroup, App::Prop_None, "Adjusts the distance between measurement text and geometry");
    ADD_PROPERTY_TYPE(Mirror,(Preferences::defaultMirror()), agroup, App::Prop_None, "Reverses measurement text if true");


    const size_t vertexCount(2);
    const size_t lineCount(3);

    // the vertices that define the extension and dimension lines
    static const SbVec3f verts[vertexCount] =
    {
        SbVec3f(0,0,0), SbVec3f(0,0,0)
    };

    // indexes used to create the edges
    // this makes a line from verts[0] to verts[1] above
    static const int32_t lines[lineCount] =
    {
        0,1,-1
    };

    pCoords = new SoCoordinate3();
    pCoords->ref();
    pCoords->point.setNum(vertexCount);
    pCoords->point.setValues(0, vertexCount, verts);

    pLines  = new SoIndexedLineSet();
    pLines->ref();
    pLines->coordIndex.setNum(lineCount);
    pLines->coordIndex.setValues(0, lineCount, lines);

    sPixmap = "umf-measurement";
}

ViewProviderMeasureLength::~ViewProviderMeasureLength()
{
    pCoords->unref();
    pLines->unref();
}

void ViewProviderMeasureLength::onChanged(const App::Property* prop)
{
    if (prop == &Mirror || prop == &DistFactor) {
        redrawAnnotation();
        return;
    }

    ViewProviderMeasureBase::onChanged(prop);
}


void ViewProviderMeasureLength::attach(App::DocumentObject* pcObject)
{
    ViewProviderMeasureBase::attach(pcObject);

    auto ps = getSoPickStyle();

    auto lineSep = new SoSeparator();
    auto style = getSoLineStylePrimary();
    lineSep->addChild(ps);
    lineSep->addChild(style);
    lineSep->addChild(pColor);
    lineSep->addChild(pCoords);
    lineSep->addChild(pLines);
    auto points = new SoMarkerSet();
    points->markerIndex = Gui::Inventor::MarkerBitmaps::getMarkerIndex("CROSS",
            ViewParams::instance()->getMarkerSize());
    points->numPoints=1;
    lineSep->addChild(points);

    auto textsep = getSoSeparatorText();

    auto sep = new SoAnnotation();
    sep->addChild(lineSep);
    sep->addChild(textsep);
    addDisplayMaskMode(sep, "Base");
}


//! handle changes to the feature's properties
void ViewProviderMeasureLength::updateData(const App::Property* prop)
{
    if (pcObject == nullptr) {
        return;
    }


     if (prop == &(getMeasureLength()->Elements) ||
        prop == &(getMeasureLength()->Length) ) {
        redrawAnnotation();
    }


    ViewProviderMeasureBase::updateData(prop);
}

//! repaint the anotation
void ViewProviderMeasureLength::redrawAnnotation()
{
    auto positions = getPositionPoints();
    // point on element
    pCoords->point.set1Value(0, SbVec3f(positions.first.x, positions.first.y, positions.first.z));
    // text position
    pCoords->point.set1Value(1, SbVec3f(positions.second.x, positions.second.y, positions.second.z));

    setLabelTranslation(pCoords->point[1]);
    setLabelValue(getMeasureLength()->Length.getQuantityValue());

    updateView();
}

//! retrive the feature
Measure::MeasureLength* ViewProviderMeasureLength::getMeasureLength()
{
    auto feature = dynamic_cast<Measure::MeasureLength*>(pcObject);
    if (!feature) {
        throw Base::RuntimeError("Feature not found for ViewProviderMeasureLength");
    }
    return feature;
}

//! retrieve the point on element and location of text
//! mostly based on code in vpMeasureDistance
std::pair<Base::Vector3d, Base::Vector3d> ViewProviderMeasureLength::getPositionPoints()
{
    std::pair<Base::Vector3d, Base::Vector3d> result;
    std::pair<Base::Vector3d, Base::Vector3d> ends = getMeasureLength()->getEndPoints();
    auto pointOnElements = (ends.first + ends.second) / 2.0;
    double distance = (ends.second - ends.first).Length();
    auto elementDirection = (ends.second - ends.first).Normalize();
    auto textDirection = getTextDirection(elementDirection);
    result.first = pointOnElements;

    // without the fudgeFactor, the text is too far away sometimes.  Not a big deal if we can drag the text. Could change the default for DistFactor too.
    double fudgeFactor(0.5);
    result.second = pointOnElements + textDirection * distance * DistFactor.getValue() * fudgeFactor;

    return result;
}

//! calculate a good direction for the text based on the layout of the elements and its
//! relationship with the cardinal axes.  elementDirection should be normalized.
//! original is in VPMeasureDistance.
Base::Vector3d ViewProviderMeasureLength::getTextDirection(Base::Vector3d elementDirection, double tolerance) const
{
    const Base::Vector3d stdX(1.0, 0.0, 0.0);
    const Base::Vector3d stdY(0.0, 1.0, 0.0);
    const Base::Vector3d stdZ(0.0, 0.0, 1.0);

    Base::Vector3d textDirection = elementDirection.Cross(stdX);
    if (textDirection.Length() < tolerance) {
        textDirection = elementDirection.Cross(stdY);
    }
    if (textDirection.Length() < tolerance) {
        textDirection = elementDirection.Cross(stdZ);
    }
    textDirection.Normalize();
    if (textDirection.Dot(stdZ) < 0.0) {
        textDirection = textDirection * -1.0;
    }

    return textDirection.Normalize();
}
