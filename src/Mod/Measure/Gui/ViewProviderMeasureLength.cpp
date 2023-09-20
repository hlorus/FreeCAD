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

#include <Gui/Inventor/MarkerBitmaps.h>

#include <App/Document.h>
#include <Base/Console.h>
#include <Base/Quantity.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/ViewParams.h>

#include <Mod/Measure/App/MeasureLength.h>
#include <Mod/Measure/App/Preferences.h>

#include "ViewProviderMeasureLength.h"

using namespace Gui;
using namespace MeasureGui;
using namespace Measure;

PROPERTY_SOURCE(MeasureGui::ViewProviderMeasureLength, MeasureGui::ViewProviderMeasurementBase)


ViewProviderMeasureLength::ViewProviderMeasureLength()
{
    static const char *agroup = "Appearance";
    ADD_PROPERTY_TYPE(DistFactor,(Preferences::defaultDistFactor()), agroup, App::Prop_None, "Adjusts the distance between measurement text and geometry");
    ADD_PROPERTY_TYPE(Mirror,(Preferences::defaultMirror()), agroup, App::Prop_None, "Reverses measurement text if true");


    const size_t vertexCount(4);
    const size_t lineCount(9);

    static const SbVec3f verts[vertexCount] =
    {
        SbVec3f(0,0,0), SbVec3f(0,0,0),
        SbVec3f(0,0,0), SbVec3f(0,0,0)
    };

    // indexes used to create the edges
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

ViewProviderMeasureLength::~ViewProviderMeasureLength()
{
    pCoords->unref();
    pLines->unref();
}

void ViewProviderMeasureLength::onChanged(const App::Property* prop)
{
    if (prop == &Mirror || prop == &DistFactor) {
        updateData(prop);
    }
    else {
        ViewProviderMeasurementBase::onChanged(prop);
    }
}


void ViewProviderMeasureLength::attach(App::DocumentObject* pcObject)
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


void ViewProviderMeasureLength::updateData(const App::Property* prop)
{
    if (pcObject == nullptr) {
        return;
    }

    if (prop->getTypeId() == App::PropertyVector::getClassTypeId() ||
        prop == &Mirror || prop == &DistFactor) {

        auto ob = static_cast<Measure::MeasureLength*>(getObject());

//        if (strcmp(prop->getName(),"Position1") == 0) {
//            auto vec1 = ob->Position1.getValue();
//            pCoords->point.set1Value(0, SbVec3f(vec1.x, vec1.y, vec1.z));
//        }
//        else if (strcmp(prop->getName(),"Position2") == 0) {
//            auto vec2 = ob->Position2.getValue();
//            pCoords->point.set1Value(1, SbVec3f(vec2.x, vec2.y, vec2.z));
//        }

        pCoords->point.set1Value(0, SbVec3f(0.0, 0.0, 0.0));
        pCoords->point.set1Value(1, SbVec3f(100.0, 100.0, 100.0));

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

    ViewProviderMeasurementBase::updateData(prop);
}


