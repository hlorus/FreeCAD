/***************************************************************************
 *   Copyright (c) 2008 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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
#include "Mod/Part/App/MeasureDistancePoints.h"

#include "ViewProviderMeasureDistancePoints.h"
#include "Gui/Application.h"
#include <Gui/Command.h>
#include "Gui/Document.h"
#include "Gui/ViewParams.h"


using namespace Gui;
using namespace PartGui;

PROPERTY_SOURCE(PartGui::ViewProviderMeasureDistancePoints, Gui::ViewProviderDocumentObject)


ViewProviderMeasureDistancePoints::ViewProviderMeasureDistancePoints()
{
    ADD_PROPERTY(DistFactor,(1.0));
    ADD_PROPERTY(Mirror,(false));

    pFont = new SoFontStyle();
    pFont->ref();
    pLabel = new SoText2();
    pLabel->ref();
    pColor = new SoBaseColor();
    pColor->ref();
    pTextColor = new SoBaseColor();
    pTextColor->ref();
    pTranslation = new SoTranslation();
    pTranslation->ref();

    TextColor.touch();
    FontSize.touch();
    LineColor.touch();

    static const SbVec3f verts[4] =
    {
        SbVec3f(0,0,0), SbVec3f(0,0,0),
        SbVec3f(0,0,0), SbVec3f(0,0,0)
    };

    // indexes used to create the edges
    static const int32_t lines[9] =
    {
        0,2,-1,
        1,3,-1,
        2,3,-1
    };

    pCoords = new SoCoordinate3();
    pCoords->ref();
    pCoords->point.setNum(4);
    pCoords->point.setValues(0, 4, verts);

    pLines  = new SoIndexedLineSet();
    pLines->ref();
    pLines->coordIndex.setNum(9);
    pLines->coordIndex.setValues(0, 9, lines);
    sPixmap = "view-measurement";
}

ViewProviderMeasureDistancePoints::~ViewProviderMeasureDistancePoints()
{
    pFont->unref();
    pLabel->unref();
    pColor->unref();
    pTextColor->unref();
    pTranslation->unref();
    pCoords->unref();
    pLines->unref();
}

void ViewProviderMeasureDistancePoints::onChanged(const App::Property* prop)
{
    if (prop == &Mirror || prop == &DistFactor) {
        updateData(prop);
    }
    else if (prop == &TextColor) {
        const App::Color& c = TextColor.getValue();
        pTextColor->rgb.setValue(c.r,c.g,c.b);
    }
    else if (prop == &LineColor) {
        const App::Color& c = LineColor.getValue();
        pColor->rgb.setValue(c.r,c.g,c.b);
    }
    else if (prop == &FontSize) {
        pFont->size = FontSize.getValue();
    }
    else {
        ViewProviderDocumentObject::onChanged(prop);
    }
}

std::vector<std::string> ViewProviderMeasureDistancePoints::getDisplayModes() const
{
    // add modes
    std::vector<std::string> StrList;
    StrList.emplace_back("Base");
    return StrList;
}

void ViewProviderMeasureDistancePoints::setDisplayMode(const char* ModeName)
{
    if (strcmp(ModeName, "Base") == 0)
        setDisplayMaskMode("Base");
    ViewProviderDocumentObject::setDisplayMode(ModeName);
}

void ViewProviderMeasureDistancePoints::attach(App::DocumentObject* pcObject)
{
    ViewProviderDocumentObject::attach(pcObject);

    auto ps = new SoPickStyle();
    ps->style = SoPickStyle::UNPICKABLE;

    auto lineSep = new SoSeparator();
    auto style = new SoDrawStyle();
    style->lineWidth = 2.0f;
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

    auto textsep = new SoSeparator();
    textsep->addChild(pTranslation);
    textsep->addChild(pTextColor);
    textsep->addChild(pFont);
    textsep->addChild(pLabel);

    auto sep = new SoAnnotation();
    sep->addChild(lineSep);
    sep->addChild(textsep);
    addDisplayMaskMode(sep, "Base");
}

void ViewProviderMeasureDistancePoints::updateData(const App::Property* prop)
{
    if (prop->getTypeId() == App::PropertyLinkSub::getClassTypeId() ||
        prop == &Mirror || prop == &DistFactor) {
        if (strcmp(prop->getName(),"P1") == 0) {
            Base::Vector3f v = static_cast<Part::MeasureDistancePoints*>(getObject())->getP1();
            pCoords->point.set1Value(0, SbVec3f(v.x,v.y,v.z));
        }
        else if (strcmp(prop->getName(),"P2") == 0) {
            Base::Vector3f v = static_cast<Part::MeasureDistancePoints*>(getObject())->getP2();
            pCoords->point.set1Value(1, SbVec3f(v.x,v.y,v.z));
        }

        SbVec3f pt1 = pCoords->point[0];
        SbVec3f pt2 = pCoords->point[1];
        SbVec3f dif = pt1-pt2;

        float length = fabs(dif.length())*DistFactor.getValue();
        if (Mirror.getValue())
            length = -length;


        if (dif.sqrLength() < 10.0e-6f) {
            pCoords->point.set1Value(2, pt1+SbVec3f(0.0f,0.0f,length));
            pCoords->point.set1Value(3, pt2+SbVec3f(0.0f,0.0f,length));
        }
        else {
            SbVec3f dir = dif.cross(SbVec3f(1.0f,0.0f,0.0f));
            if (dir.sqrLength() < 10.0e-6f)
                dir = dif.cross(SbVec3f(0.0f,1.0f,0.0f));
            if (dir.sqrLength() < 10.0e-6f)
                dir = dif.cross(SbVec3f(0.0f,0.0f,1.0f));
            dir.normalize();
            if (dir.dot(SbVec3f(0.0f,0.0f,1.0f)) < 0.0f)
                length = -length;
            pCoords->point.set1Value(2, pt1 + length*dir);
            pCoords->point.set1Value(3, pt2 + length*dir);
        }

        SbVec3f pos = (pCoords->point[2]+pCoords->point[3])/2.0f;
        pTranslation->translation.setValue(pos);

        pLabel->string.setValue((Base::Quantity(dif.length(), Base::Unit::Length)).getUserString().toUtf8().constData());
    }

    ViewProviderDocumentObject::updateData(prop);
}

