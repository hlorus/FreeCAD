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

#include "Gui/Application.h"
#include "Gui/MDIView.h"
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
#include <Gui/Document.h>
#include <Gui/ViewParams.h>
#include <Gui/Inventor/MarkerBitmaps.h>
#include <Gui/View3DInventor.h>
#include <Gui/View3DInventorViewer.h>


#include <Mod/Measure/App/Preferences.h>

#include "ViewProviderMeasureBase.h"


using namespace MeasureGui;
using namespace Measure;
namespace bp = boost::placeholders;

//NOLINTBEGIN
PROPERTY_SOURCE(MeasureGui::ViewProviderMeasureBase, Gui::ViewProviderDocumentObject)
//NOLINTEND

ViewProviderMeasureBase::ViewProviderMeasureBase()
{
    static const char *agroup = "Appearance";
//NOLINTBEGIN
    ADD_PROPERTY_TYPE(TextColor, (Preferences::defaultTextColor()), agroup, App::Prop_None, "Color for the measurement text");
    ADD_PROPERTY_TYPE(TextBackgroundColor, (Preferences::defaultTextBackgroundColor()), agroup, App::Prop_None, "Color for the measurement text background");
    ADD_PROPERTY_TYPE(LineColor, (Preferences::defaultLineColor()), agroup, App::Prop_None, "Color for the measurement lines");
    ADD_PROPERTY_TYPE(FontSize, (Preferences::defaultFontSize()), agroup, App::Prop_None, "Size of measurement text");
    ADD_PROPERTY_TYPE(DistFactor,(Preferences::defaultDistFactor()), agroup, App::Prop_None, "Adjusts the distance between measurement text and geometry");
    ADD_PROPERTY_TYPE(Mirror,(Preferences::defaultMirror()), agroup, App::Prop_None, "Reverses measurement text position if true");
//NOLINTEND

    pFont = new SoFontStyle();
    pFont->ref();
    pLabel = new Gui::SoTextLabel();
    pLabel->ref();
    pColor = new SoBaseColor();
    pColor->ref();
    pTextColor = new SoBaseColor();
    pTextColor->ref();
    pTranslation = new SoTranslation();
    pTranslation->ref();

    TextColor.touch();
    TextBackgroundColor.touch();
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


void ViewProviderMeasureBase::finishRestoring() {

    // Force measurment visibility when loading a document
    show();
}


void ViewProviderMeasureBase::onChanged(const App::Property* prop)
{
    if (prop == &TextColor) {
        const App::Color& color = TextColor.getValue();
        pTextColor->rgb.setValue(color.r, color.g, color.b);
    }
    else if (prop == &TextBackgroundColor) {
        const App::Color& color = TextBackgroundColor.getValue();
        pLabel->backgroundColor.setValue(color.r, color.g, color.b);
    }
    else if (prop == &LineColor) {
        const App::Color& color = LineColor.getValue();
        pColor->rgb.setValue(color.r, color.g, color.b);
    }
    else if (prop == &FontSize) {
        pFont->size = FontSize.getValue();
    } else if (prop == &DistFactor || prop == &Mirror) {
        redrawAnnotation();
    }
}


void ViewProviderMeasureBase::setLabelValue(const Base::Quantity& value) {
    pLabel->string.setValue(value.getUserString().toUtf8().constData());
}

void ViewProviderMeasureBase::setLabelValue(const QString& value) {
    pLabel->string.setValue(value.toUtf8().constData());
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


//! the App side has requested a redraw
void ViewProviderMeasureBase::onGuiUpdate(const Measure::MeasureBase* measureObject) {
    (void) measureObject;
    updateView();
}


void ViewProviderMeasureBase::attach(App::DocumentObject *pcObj)
{
    ViewProviderDocumentObject::attach(pcObj);

//NOLINTBEGIN
    auto bnd = boost::bind(&ViewProviderMeasureBase::onGuiUpdate, this, bp::_1);
//NOLINTEND

    auto feature = dynamic_cast<Measure::MeasureBase*>(pcObject);
    if (feature) {
        feature->signalGuiUpdate.connect(bnd);
    }
}


// TODO: should this be pure virtual in vpMeasureBase?
void ViewProviderMeasureBase::redrawAnnotation()
{
//    Base::Console().Message("VPMB::redrawAnnotation()\n");
}

//! connect to the subject to receive visibility updates
void ViewProviderMeasureBase::connectToSubject(App::DocumentObject* subject)
{
    if (!subject) {
        return;
    }

    //NOLINTBEGIN
    auto bndVisibility = boost::bind(&ViewProviderMeasureBase::onSubjectVisibilityChanged, this, bp::_1, bp::_2);
    //NOLINTEND
    subject->signalChanged.connect(bndVisibility);
}


//! retrive the feature
Measure::MeasureBase* ViewProviderMeasureBase::getMeasureObject()
{
    // Note: Cast to MeasurePropertyBase once we use it to provide the needed values e.g. basePosition textPosition etc.
    auto feature = dynamic_cast<Measure::MeasureBase*>(pcObject);
    if (!feature) {
        throw Base::RuntimeError("Feature not found for ViewProviderMeasureBase");
    }
    return feature;
}


//! calculate a good direction from the elements being measured to the annotation text based on the layout
//! of the elements and relationship with the cardinal axes and the view direction.  elementDirection
//! is expected to be a normalized vector.
//! an example of an elementDirection would be the vector from the start of a line to the end.
Base::Vector3d ViewProviderMeasureBase::getTextDirection(Base::Vector3d elementDirection, double tolerance)
{
    auto view = dynamic_cast<Gui::View3DInventor*>(Gui::Application::Instance->activeDocument()->getActiveView());
    // if (!view) {
    //     // Measure doesn't work with this kind of active view.  Might be dependency graph, might be TechDraw, or ????
    //     // i don't know if this can even happen.
    //     throw Base::RuntimeError("Measure doesn't work with this kind of active view.");
    // }
    Gui::View3DInventorViewer* viewer = view->getViewer();
    Base::Vector3d viewDirection = toVector3d(viewer->getViewDirection()).Normalize();
    Base::Vector3d textDirection = elementDirection.Cross(viewDirection);
    if (textDirection.Length() < tolerance)  {
        // either elementDirection and viewDirection are parallel or one of them is null.
        viewDirection = toVector3d(viewer->getUpDirection()).Normalize();
        textDirection = elementDirection.Cross(viewDirection);
    }

    return textDirection.Normalize();
}

//! true if the subject of this measurement is visible
bool ViewProviderMeasureBase::isSubjectVisible()
{
    // we need these things to proceed
    if (!getMeasureObject() ||
        !getMeasureObject()->getSubject() ||
        !Gui::Application::Instance->getDocument(getMeasureObject()->getDocument()) ) {
        return false;
    }

    auto guiDoc = Gui::Application::Instance->getDocument(getMeasureObject()->getDocument());
    Gui::ViewProvider* vp = guiDoc->getViewProvider(getMeasureObject()->getSubject());
    if (vp) {
        return vp->isVisible();
    }

    return false;
}


//! gets called when the subject object issues a signalChanged (ie a property change).  We are only interested in the subject's
//! Visibility property
void ViewProviderMeasureBase::onSubjectVisibilityChanged(const App::DocumentObject& docObj, const App::Property& prop)
{
    std::string propName = prop.getName();
    if (propName == "Visibility") {
        if (docObj.Visibility.getValue()) {
            // show only if subject is visible
            setVisible(true);
        } else {
            setVisible(false);
        }
    }
}


//NOLINTBEGIN
PROPERTY_SOURCE_ABSTRACT(MeasureGui::ViewProviderMeasurePropertyBase, MeasureGui::ViewProviderMeasureBase)
//NOLINTEND


ViewProviderMeasurePropertyBase::ViewProviderMeasurePropertyBase()
{

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

ViewProviderMeasurePropertyBase::~ViewProviderMeasurePropertyBase()
{
    pCoords->unref();
    pLines->unref();
}

void ViewProviderMeasurePropertyBase::onChanged(const App::Property* prop)
{
    if (pcObject == nullptr) {
        return;
    }

    ViewProviderMeasureBase::onChanged(prop);
}


//! handle changes to the feature's properties
void ViewProviderMeasurePropertyBase::updateData(const App::Property* prop)
{
    if (pcObject == nullptr) {
        return;
    }

    ViewProviderMeasureBase::updateData(prop);
}


void ViewProviderMeasurePropertyBase::attach(App::DocumentObject* pcObject)
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
            Gui::ViewParams::instance()->getMarkerSize());
    points->numPoints=1;
    lineSep->addChild(points);

    auto textsep = getSoSeparatorText();

    auto sep = new SoAnnotation();
    sep->addChild(lineSep);
    sep->addChild(textsep);
    addDisplayMaskMode(sep, "Base");
}


//! repaint the anotation
void ViewProviderMeasurePropertyBase::redrawAnnotation()
{
    // point on element
    Base::Vector3d basePos = getBasePosition();
    pCoords->point.set1Value(0, SbVec3f(basePos.x, basePos.y, basePos.z));
    
    // text position
    Base::Vector3d textPos = getTextPosition();
    pCoords->point.set1Value(1, SbVec3f(textPos.x, textPos.y, textPos.z));

    setLabelTranslation(pCoords->point[1]);
    setLabelValue(getMeasureObject()->getResultString());

    ViewProviderMeasureBase::redrawAnnotation();

    ViewProviderDocumentObject::updateView();
}


Base::Vector3d ViewProviderMeasurePropertyBase::getBasePosition(){
    auto measureObject = getMeasureObject();
    Base::Placement placement = measureObject->getPlacement();
    return placement.getPosition();
}


Base::Vector3d ViewProviderMeasurePropertyBase::getTextPosition(){
    auto basePoint = getBasePosition();
    Base::Vector3d textDirection(1.0, 1.0, 1.0);
    textDirection.Normalize();
    constexpr double defaultLength = 10.0;
    double length = defaultLength;
    if (Mirror.getValue()) {
        length = -length;
    }

    return basePoint + textDirection * length * DistFactor.getValue();
}

//! called by the system when it is time to display this measure
void ViewProviderMeasureBase::show()
{
    if (isSubjectVisible()) {
        // only show the annotation if the subject is visible.
        // this avoids disconnected annotations floating in space.
        ViewProviderDocumentObject::show();
    }
}
