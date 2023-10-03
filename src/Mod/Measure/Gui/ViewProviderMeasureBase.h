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

#ifndef GUI_VIEWPROVIDER_MEASUREMENTBASE_H
#define GUI_VIEWPROVIDER_MEASUREMENTBASE_H

#include <Mod/Measure/MeasureGlobal.h>

#include <App/Application.h>
#include <App/PropertyStandard.h>
#include <Base/Parameter.h>
#include <Gui/ViewProviderDocumentObject.h>
#include <Gui/SoTextLabel.h>
#include <QString>

#include <Mod/Measure/App/MeasureBase.h>

class SbVec2s;
class SoFontStyle;
class SoBaseColor;
class SoText2;
class SoTranslation;
class SoPickStyle;
class SoCoordinate3;
class SoIndexedLineSet;



namespace MeasureGui {

class MeasureGuiExport ViewProviderMeasureBase :public Gui::ViewProviderDocumentObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(ViewProviderMeasureBase);

public:
    /// constructor.
    ViewProviderMeasureBase();

    /// destructor.
    ~ViewProviderMeasureBase() override;

    // Display properties
    App::PropertyColor          TextColor;
    App::PropertyColor          TextBackgroundColor;
    App::PropertyColor          LineColor;
    App::PropertyInteger        FontSize;

    /**
     * Attaches the document object to this view provider.
     */
    void attach(App::DocumentObject *pcObj) override;
    void onGuiUpdate(const Measure::MeasureBase* measureObject);

    bool useNewSelectionModel() const override {return true;}
    std::vector<std::string> getDisplayModes() const override;
    void setDisplayMode(const char* ModeName) override;

    virtual void redrawAnnotation();
    Measure::MeasureBase* getMeasureObject();

protected:
    void onChanged(const App::Property* prop) override;
    void setLabelValue(const Base::Quantity& value);
    void setLabelValue(const QString& value);
    void setLabelTranslation(const SbVec3f& position);

    SoPickStyle* getSoPickStyle();
    SoDrawStyle* getSoLineStylePrimary();
    SoSeparator* getSoSeparatorText();

    bool _mShowTree = true;

    Gui::SoTextLabel * pLabel;
    SoTranslation    * pTranslation;
    SoFontStyle      * pFont;
    SoBaseColor      * pColor;
    SoBaseColor      * pTextColor;

};


class MeasureGuiExport ViewProviderMeasurePropertyBase : public MeasureGui::ViewProviderMeasureBase
{
    PROPERTY_HEADER_WITH_OVERRIDE(MeasureGui::ViewProviderMeasurePropertyBase);

public:
    /// Constructor
    ViewProviderMeasurePropertyBase();
    ~ViewProviderMeasurePropertyBase() override;

    void attach(App::DocumentObject * feature) override;
    void updateData(const App::Property* prop) override;

    void redrawAnnotation() override;

protected:
    void onChanged(const App::Property* prop) override;

    virtual Base::Vector3d getBasePosition();
    virtual Base::Vector3d getTextPosition();
    Base::Vector3d getTextDirection(Base::Vector3d elementDirection, double tolerance = 10e-6) const;

private:
    SoCoordinate3    * pCoords;
    SoIndexedLineSet * pLines;

};

} // namespace Gui

#endif // GUI_VIEWPROVIDER_MEASUREMENTBASE_H

