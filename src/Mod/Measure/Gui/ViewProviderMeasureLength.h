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


#ifndef MEASUREGUI_VIEWPROVIDERMEASURELENGTH_H
#define MEASUREGUI_VIEWPROVIDERMEASURELENGTH_H

#include <Mod/Measure/MeasureGlobal.h>

#include "ViewProviderMeasurementBase.h"

class SoCoordinate3;
class SoIndexedLineSet;

namespace App
{
class DocumentObject;
class Property;
}

namespace MeasureGui
{

class MeasureGuiExport ViewProviderMeasureLength : public MeasureGui::ViewProviderMeasurementBase
{
    PROPERTY_HEADER_WITH_OVERRIDE(MeasureGui::ViewProviderMeasureLength);

public:
    /// Constructor
    ViewProviderMeasureLength();
    ~ViewProviderMeasureLength() override;

    // // Display properties
    App::PropertyFloat          DistFactor;
    App::PropertyBool           Mirror;

    void attach(App::DocumentObject *) override;
    void updateData(const App::Property*) override;

protected:
    void onChanged(const App::Property* prop) override;

private:
    SoCoordinate3    * pCoords;
    SoIndexedLineSet * pLines;

};

} //namespace MeasureGui


#endif // MEASUREGUI_VIEWPROVIDERMEASURELENGTH_H

