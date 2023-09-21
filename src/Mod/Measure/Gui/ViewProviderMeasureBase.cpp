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
#include <Base/Console.h>

#include "ViewProviderMeasureBase.h"


using namespace MeasureGui;
namespace bp = boost::placeholders;


PROPERTY_SOURCE(MeasureGui::ViewProviderMeasureBase, Gui::ViewProviderDocumentObject)

ViewProviderMeasureBase::ViewProviderMeasureBase()
{
    static const char *agroup = "Appearance";
    ADD_PROPERTY_TYPE(TextColor, (defaultTextColor()), agroup, App::Prop_None, "Color for the measurement text");
    ADD_PROPERTY_TYPE(LineColor, (defaultLineColor()), agroup, App::Prop_None, "Color for the measurement lines");
    ADD_PROPERTY_TYPE(FontSize, (defaultFontSize()), agroup, App::Prop_None, "Size of measurement text");

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

ViewProviderMeasureBase::~ViewProviderMeasureBase()
{
    pFont->unref();
    pLabel->unref();
    pColor->unref();
    pTextColor->unref();
    pTranslation->unref();
}

std::vector<std::string> ViewProviderMeasureBase::getDisplayModes() const
{
    // add modes
    std::vector<std::string> StrList;
    StrList.emplace_back("Base");
    return StrList;
}

void ViewProviderMeasureBase::setDisplayMode(const char* ModeName)
{
    if (strcmp(ModeName, "Base") == 0) {
        setDisplayMaskMode("Base");
    }
    ViewProviderDocumentObject::setDisplayMode(ModeName);
}


void ViewProviderMeasureBase::onChanged(const App::Property* prop)
{
    if (prop == &TextColor) {
        const App::Color& color = TextColor.getValue();
        pTextColor->rgb.setValue(color.r, color.g, color.b);
    }
    else if (prop == &LineColor) {
        const App::Color& color = LineColor.getValue();
        pColor->rgb.setValue(color.r, color.g, color.b);
    }
    else if (prop == &FontSize) {
        pFont->size = FontSize.getValue();
    }
}


void ViewProviderMeasureBase::setLabelValue(const Base::Quantity& value) {
    pLabel->string.setValue(value.getUserString().toUtf8().constData());
}

void ViewProviderMeasureBase::setLabelTranslation(const SbVec3f& position) {
    pTranslation->translation.setValue(position);
}


SoPickStyle* ViewProviderMeasureBase::getSoPickStyle() {
    auto ps = new SoPickStyle();
    ps->style = SoPickStyle::UNPICKABLE;
    return ps;
}

SoDrawStyle* ViewProviderMeasureBase::getSoLineStylePrimary() {
    auto style = new SoDrawStyle();
    style->lineWidth = 2.0f;
    return style;
}

SoSeparator* ViewProviderMeasureBase::getSoSeparatorText() {
    auto textsep = new SoSeparator();
    textsep->addChild(pTranslation);
    textsep->addChild(pTextColor);
    textsep->addChild(pFont);
    textsep->addChild(pLabel);
    return textsep;
}


void ViewProviderMeasureBase::onGuiUpdate(const Measure::MeasureBase* measureObject) {
    (void) measureObject;
    updateView();
}

void ViewProviderMeasureBase::attach(App::DocumentObject *pcObj)
{
    ViewProviderDocumentObject::attach(pcObj);

    auto bnd = boost::bind(&ViewProviderMeasureBase::onGuiUpdate, this, bp::_1);

    Measure::MeasureBase* feature = dynamic_cast<Measure::MeasureBase*>(pcObject);
    if (feature) {
        feature->signalGuiUpdate.connect(bnd);
    }
}

// TODO: migrate these routines to Mod/Measure
//! Returns the Measure preference group
Base::Reference<ParameterGrp> ViewProviderMeasureBase::getPreferenceGroup(const char* Name)
{
    return App::GetApplication().GetUserParameter().GetGroup("BaseApp/Preferences/Mod/Measure")->GetGroup(Name);
}

App::Color ViewProviderMeasureBase::defaultLineColor()
{
    App::Color fcColor;
    fcColor.setPackedValue(getPreferenceGroup("Appearance")->GetUnsigned("DefaultLineColor", 0xFFFFFFFF));
    return fcColor;
}

App::Color ViewProviderMeasureBase::defaultTextColor()
{
    App::Color fcColor;
    fcColor.setPackedValue(getPreferenceGroup("Appearance")->GetUnsigned("DefaultTextColor", 0xFFFFFFFF));
    return fcColor;
}

int ViewProviderMeasureBase::defaultFontSize()
{
    return getPreferenceGroup("Appearance")->GetInt("DefaultFontSize", 18);
}
