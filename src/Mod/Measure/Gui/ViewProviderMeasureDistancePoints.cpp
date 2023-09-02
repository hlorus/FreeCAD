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
#include "Mod/Measure/App/MeasureDistancePoints.h"

#include "ViewProviderMeasureDistancePoints.h"
#include "Gui/Application.h"
#include <Gui/Command.h>
#include "Gui/Document.h"
#include "Gui/ViewParams.h"


using namespace Gui;
using namespace MeasureGui;

PROPERTY_SOURCE(MeasureGui::ViewProviderMeasureDistancePoints, Gui::ViewProviderDocumentObject)


ViewProviderMeasureDistancePoints::ViewProviderMeasureDistancePoints()
{
    ADD_PROPERTY(DistFactor,(1.0));
    ADD_PROPERTY(Mirror,(false));

    const size_t vertexCount(4);
    const size_t lineCount(9);

    static const SbVec3f verts[vertexCount] =
    {
        SbVec3f(0,0,0), SbVec3f(0,0,0),
        SbVec3f(0,0,0), SbVec3f(0,0,0)
    };
//    static const std::array<SbVec3f, vertexCount> verts {
//        SbVec3f(0,0,0), SbVec3f(0,0,0),
//        SbVec3f(0,0,0), SbVec3f(0,0,0)
//    };

    // indexes used to create the edges
    static const int32_t lines[lineCount] =
    {
        0,2,-1,
        1,3,-1,
        2,3,-1
    };
//    static const std::array<int32_t, lineCount> lines {
//        0,2,-1,
//        1,3,-1,
//        2,3,-1
//    };

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

ViewProviderMeasureDistancePoints::~ViewProviderMeasureDistancePoints()
{
    pCoords->unref();
    pLines->unref();
}

void ViewProviderMeasureDistancePoints::onChanged(const App::Property* prop)
{
    if (prop == &Mirror || prop == &DistFactor) {
        updateData(prop);
    }
    else {
        Gui::ViewProviderMeasurementBase::onChanged(prop);
    }
}


void ViewProviderMeasureDistancePoints::attach(App::DocumentObject* pcObject)
{
    ViewProviderMeasurementBase::attach(pcObject);

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

void ViewProviderMeasureDistancePoints::updateData(const App::Property* prop)
{

    if (prop == &static_cast<Measure::MeasureDistancePoints*>(getObject())->Distance) {
        updateView();
    }
    else if (prop->getTypeId() == App::PropertyLinkSub::getClassTypeId() ||
        prop == &Mirror || prop == &DistFactor) {
        if (strcmp(prop->getName(),"P1") == 0) {
            Base::Vector3f vec = static_cast<Measure::MeasureDistancePoints*>(getObject())->getP1();
            pCoords->point.set1Value(0, SbVec3f(vec.x, vec.y, vec.z));
        }
        else if (strcmp(prop->getName(),"P2") == 0) {
            Base::Vector3f vec = static_cast<Measure::MeasureDistancePoints*>(getObject())->getP2();
            pCoords->point.set1Value(1, SbVec3f(vec.x, vec.y, vec.z));
        }

        SbVec3f pt1 = pCoords->point[0];
        SbVec3f pt2 = pCoords->point[1];
        SbVec3f dif = pt1-pt2;

        double length = fabs(dif.length())*DistFactor.getValue();
        if (Mirror.getValue()) {
            length = -length;
        }

        const double tolerance(10.0e-6);
        if (dif.sqrLength() < tolerance) {
            pCoords->point.set1Value(2, pt1+SbVec3f(0.0, 0.0, length));
            pCoords->point.set1Value(3, pt2+SbVec3f(0.0, 0.0, length));
        }
        else {
            SbVec3f dir = dif.cross(SbVec3f(1.0, 0.0, 0.0));
            if (dir.sqrLength() < tolerance) {
                dir = dif.cross(SbVec3f(0.0, 1.0, 0.0));
            }
            if (dir.sqrLength() < tolerance) {
                dir = dif.cross(SbVec3f(0.0, 0.0, 1.0));
            }
            dir.normalize();
            if (dir.dot(SbVec3f(0.0, 0.0, 1.0)) < 0.0) {
                length = -length;
            }
            pCoords->point.set1Value(2, pt1 + length*dir);
            pCoords->point.set1Value(3, pt2 + length*dir);
        }

        SbVec3f pos = (pCoords->point[2]+pCoords->point[3]) / 2.0;
        setLabelTranslation(pos);
        setLabelValue(Base::Quantity(dif.length(), Base::Unit::Length));

        updateView();
    }

    ViewProviderDocumentObject::updateData(prop);
}

