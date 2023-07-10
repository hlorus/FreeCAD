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


#ifndef GUI_VIEWPROVIDERMEASUREDISTANCEPOINTS_H
#define GUI_VIEWPROVIDERMEASUREDISTANCEPOINTS_H

#include <Gui/ViewProviderDocumentObject.h>
#include <Gui/ViewProviderMeasurementBase.h>
#include <Gui/ViewProviderMeasureDistance.h>

#include <QObject>

class SoText2;
class SoTranslation;
class SoCoordinate3;
class SoIndexedLineSet;

namespace PartGui
{


class PartGuiExport ViewProviderMeasureDistancePoints : public Gui::ViewProviderMeasurementBase
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartGui::ViewProviderMeasureDistancePoints);

public:
    /// Constructor
    ViewProviderMeasureDistancePoints();
    ~ViewProviderMeasureDistancePoints() override;

    // // Display properties
    App::PropertyFloat          DistFactor;
    App::PropertyBool           Mirror;

    void attach(App::DocumentObject *) override;
    void updateData(const App::Property*) override;
    bool useNewSelectionModel() const override {return true;}
    std::vector<std::string> getDisplayModes() const override;
    void setDisplayMode(const char* ModeName) override;

protected:
    void onChanged(const App::Property* prop) override;

private:
    SoText2          * pLabel;
    SoTranslation    * pTranslation;
    SoCoordinate3    * pCoords;
    SoIndexedLineSet * pLines;

};

} //namespace PartGui


#endif // GUI_VIEWPROVIDERMEASUREDISTANCEPOINTS_H
