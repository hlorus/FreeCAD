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

#include <Gui/Inventor/MarkerBitmaps.h>

#include <App/Document.h>
#include <App/MeasureDistance.h>
#include <Base/Console.h>
#include <Base/Quantity.h>
#include "Mod/Measure/App/MeasureDistance.h"
#include <Mod/Measure/App/Preferences.h>

#include "ViewProviderMeasureDistance.h"
#include "Gui/Application.h"
#include <Gui/Command.h>
#include "Gui/Document.h"
#include "Gui/ViewParams.h"


using namespace Gui;
using namespace MeasureGui;
using namespace Measure;

PROPERTY_SOURCE(MeasureGui::ViewProviderMeasureDistance, MeasureGui::ViewProviderMeasureBase)


ViewProviderMeasureDistance::ViewProviderMeasureDistance()
{
    const size_t vertexCount(4);
    const size_t lineCount(9);

    // vert[0], vert[1] points on shape (dimension points)
    // vert[2], vert[3] ends of extension lines/dimension line
    static const SbVec3f verts[vertexCount] =
    {
        SbVec3f(0,0,0), SbVec3f(0,0,0),
        SbVec3f(0,0,0), SbVec3f(0,0,0)
    };

    // vert indexes used to create the annotation lines
    // lines[0] extension line 1
    // lines[1] extension line 2
    // lines[2] dimension line
    static const int32_t lines[lineCount] =
    {
        0,2,-1,
        1,3,-1,
        2,3,-1
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

ViewProviderMeasureDistance::~ViewProviderMeasureDistance()
{
    pCoords->unref();
    pLines->unref();
}

void ViewProviderMeasureDistance::onChanged(const App::Property* prop)
{
    if (prop == &Mirror || prop == &DistFactor) {
        redrawAnnotation();
    }
    else {
        ViewProviderMeasureBase::onChanged(prop);
    }
}


void ViewProviderMeasureDistance::attach(App::DocumentObject* pcObject)
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
    points->numPoints=2;
    lineSep->addChild(points);

    auto textsep = getSoSeparatorText();

    auto sep = new SoAnnotation();
    sep->addChild(lineSep);
    sep->addChild(textsep);
    addDisplayMaskMode(sep, "Base");
}


//! handle changes to the feature's properties
void ViewProviderMeasureDistance::updateData(const App::Property* prop)
{
    if (pcObject == nullptr) {
        return;
    }

    auto obj = dynamic_cast<Measure::MeasureDistance*>(getObject());

    if (obj &&
        (prop == &(obj->Element1)  ||
         prop == &(obj->Element2)) ) {
        connectToSubject(obj->getSubject());
        redrawAnnotation();
    }

    if (obj &&
        (prop == &(obj->Distance) ||
         prop == &(obj->Position1) ||
         prop == &(obj->Position2)) ) {
        redrawAnnotation();
    }

    ViewProviderDocumentObject::updateData(prop);
}

Measure::MeasureDistance* ViewProviderMeasureDistance::getMeasureDistance()
{
    Measure::MeasureDistance* feature = dynamic_cast<Measure::MeasureDistance*>(pcObject);
    if (!feature) {
        throw Base::RuntimeError("Feature not found for ViewProviderMeasureDistance");
    }
    return feature;
}

//! repaint the anotation
void ViewProviderMeasureDistance::redrawAnnotation()
{
    auto object = dynamic_cast<Measure::MeasureDistance*>(getObject());
    // if (!object) {
    //      throw Base::RuntimeError("something really bad happened here");
    // }

    auto vec1 = object->Position1.getValue();
    pCoords->point.set1Value(0, toSbVec3f(vec1));
    auto vec2 = object->Position2.getValue();
    pCoords->point.set1Value(1, toSbVec3f(vec2));

    SbVec3f pt1 = pCoords->point[0];
    SbVec3f pt2 = pCoords->point[1];
    SbVec3f dif = pt1-pt2;


    double length = fabs(dif.length())*DistFactor.getValue();
    if (Mirror.getValue()) {
        length = -length;
    }

    // without the fudgeFactor, the text is too far away sometimes.  Not a big deal if we can drag the text.
    constexpr double fudgeFactor = 0.5;
    length = length * fudgeFactor;

    const double tolerance(10.0e-6);
    if (dif.sqrLength() < tolerance) {
        // if the Positionx points are too close together, move them both by the same amount? won't they still be the same distance apart?
        pCoords->point.set1Value(2, pt1+SbVec3f(0.0, 0.0, length));
        pCoords->point.set1Value(3, pt2+SbVec3f(0.0, 0.0, length));
    }
    else {
        Base::Vector3d vDif = toVector3d(dif).Normalize();
        Base::Vector3d textDirection = getTextDirection(vDif, tolerance);
        pCoords->point.set1Value(2, pt1 + length * toSbVec3f(textDirection));
        pCoords->point.set1Value(3, pt2 + length * toSbVec3f(textDirection));
    }

    // put the annotation at the middle of the dimension line
    SbVec3f pos = (pCoords->point[2]+pCoords->point[3]) / 2.0;
    setLabelTranslation(pos);
    setLabelValue(object->Distance.getQuantityValue().getUserString());

    ViewProviderMeasureBase::redrawAnnotation();
    updateView();
}

