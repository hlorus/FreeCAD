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
# include <boost_signals2.hpp>
# include <boost/signals2/connection.hpp>

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




#include <App/DocumentObject.h>
#include "ViewProviderMeasurementBase.h"

#include "Base/Console.h"


using namespace Gui;
namespace bp = boost::placeholders;


PROPERTY_SOURCE(ViewProviderMeasurementBase, ViewProviderDocumentObject)

ViewProviderMeasurementBase::ViewProviderMeasurementBase()
{
    ADD_PROPERTY(TextColor,(1.0f,1.0f,1.0f));
    ADD_PROPERTY(LineColor,(1.0f,1.0f,1.0f));
    ADD_PROPERTY(FontSize,(18));

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
}

ViewProviderMeasurementBase::~ViewProviderMeasurementBase()
{
    pFont->unref();
    pLabel->unref();
    pColor->unref();
    pTextColor->unref();
    pTranslation->unref();
}

std::vector<std::string> ViewProviderMeasurementBase::getDisplayModes() const
{
    // add modes
    std::vector<std::string> StrList;
    StrList.emplace_back("Base");
    return StrList;
}

void ViewProviderMeasurementBase::setDisplayMode(const char* ModeName)
{
    if (strcmp(ModeName, "Base") == 0) {
        setDisplayMaskMode("Base");
    }
    ViewProviderDocumentObject::setDisplayMode(ModeName);
}


void ViewProviderMeasurementBase::onChanged(const App::Property* prop)
{
    if (prop == &TextColor) {
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
}


void ViewProviderMeasurementBase::setLabelValue(const Base::Quantity& value) {
    pLabel->string.setValue(value.getUserString().toUtf8().constData());
}

void ViewProviderMeasurementBase::setLabelTranslation(const SbVec3f& position) {
    pTranslation->translation.setValue(position);
}


SoPickStyle* ViewProviderMeasurementBase::getSoPickStyle() {
    auto ps = new SoPickStyle();
    ps->style = SoPickStyle::UNPICKABLE;
    return ps;
}

SoDrawStyle* ViewProviderMeasurementBase::getSoLineStylePrimary() {
    auto style = new SoDrawStyle();
    style->lineWidth = 2.0f;
    return style;
}

SoSeparator* ViewProviderMeasurementBase::getSoSeparatorText() {
    auto textsep = new SoSeparator();
    textsep->addChild(pTranslation);
    textsep->addChild(pTextColor);
    textsep->addChild(pFont);
    textsep->addChild(pLabel);
    return textsep;
}


void ViewProviderMeasurementBase::onGuiUpdate(const App::MeasurementBase* measureObject) {
    updateView();
}

void ViewProviderMeasurementBase::attach(App::DocumentObject *pcObj)
{
    ViewProviderDocumentObject::attach(pcObj);

    auto bnd = boost::bind(&ViewProviderMeasurementBase::onGuiUpdate, this, bp::_1);

    App::MeasurementBase* feature = dynamic_cast<App::MeasurementBase*>(pcObject);
    if (feature) {
        feature->signalGuiUpdate.connect(bnd);
    }
}

