/***************************************************************************
 *   Copyright (c) 2002 Jürgen Riegel <juergen.riegel@web.de>              *
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

#include <Inventor/nodes/SoEventCallback.h>
#include <tuple>
#include <string.h>

#include "TaskView/TaskDialog.h"
#include "TaskView/TaskView.h"
#include "Application.h"
#include "App/Measure.h"

#include <qcolumnview.h>
#include <QString>


namespace Gui {

class TaskMeasure : public TaskView::TaskDialog {

public:


    QColumnView* dialog;
    void(*eventCallback)(void*, SoEventCallback*);
    App::MeasureElementInfo *elementInfo;

    TaskMeasure();
    ~TaskMeasure();

    void update();
    void close();
    bool accept();
    bool reject();

    void addElement(const char* mod, const char* obName, const char* subName);
    bool hasSelection();
    void clearSelection();    

protected:
    App::MeasurementBase *_mMeasureObject = nullptr;

    QLabel *labelMeasureType;
    QLabel *labelResult;
    QLabel *labelType;
    QLabel *labelPosition;
    QLabel *labelLength;
    QLabel *labelArea;

    void removeObject();
    void updateInfo();

    // Store the active measure module
    std::string measureModule;

    // Store a list of picked elements and subelements
    App::MeasureSelection selection;

    // List of measure types
    std::vector<App::DocumentObject> measureObjects;

};

} // namespace Gui