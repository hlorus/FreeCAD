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

#include "PreCompiled.h"

#ifndef _PreComp_
# include <boost_signals2.hpp>
# include <boost/signals2/connection.hpp>
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
}

ViewProviderMeasurementBase::~ViewProviderMeasurementBase()
{
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

