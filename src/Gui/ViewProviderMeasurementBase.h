/***************************************************************************
 *   Copyright (c) 2004 Jürgen Riegel <juergen.riegel@web.de>              *
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

#include <App/Measure.h>
#include "ViewProviderDocumentObject.h"
#include "App/PropertyContainer.h"


class SbVec2s;
class SoFontStyle;
class SoBaseColor;

namespace Gui {

class View3DInventorViewer;

class GuiExport ViewProviderMeasurementBase:public ViewProviderDocumentObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(Gui::ViewProviderMeasurementBase);

public:
    /// constructor.
    ViewProviderMeasurementBase();

    /// destructor.
    ~ViewProviderMeasurementBase() override;

    // Display properties
    App::PropertyColor          TextColor;
    App::PropertyColor          LineColor;
    App::PropertyInteger        FontSize;

    /**
     * Attaches the document object to this view provider.
     */
    void attach(App::DocumentObject *pcObj) override;
    void onGuiUpdate(const App::MeasurementBase* measureObject);

protected:
    bool _mShowTree = true;

    SoFontStyle      * pFont;
    SoBaseColor      * pColor;
    SoBaseColor      * pTextColor;

};

} // namespace Gui

#endif // GUI_VIEWPROVIDER_MEASUREMENTBASE_H

