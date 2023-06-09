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

#ifndef GUI_TASKMEASURE_H
#define GUI_TASKMEASURE_H

#include "PreCompiled.h"
#ifndef _PreComp_
#endif

#include <Inventor/events/SoEvent.h>

#include "TaskMeasure.h"

#include "Control.h"
#include "MainWindow.h"
#include "View3DInventor.h"
#include "View3DInventorViewer.h"


using namespace Gui;


TaskMeasure::TaskMeasure(){

    this->setButtonPosition(TaskMeasure::South);
    Gui::TaskView::TaskBox* taskbox = new Gui::TaskView::TaskBox(QPixmap(), QString(), true, nullptr);

    labelType = new QLabel();
    labelPosition = new QLabel();
    labelLength = new QLabel();
    labelArea = new QLabel();

    taskbox->groupLayout()->addWidget(labelType);
    taskbox->groupLayout()->addWidget(labelPosition);
    taskbox->groupLayout()->addWidget(labelLength);
    taskbox->groupLayout()->addWidget(labelArea);

    Content.push_back(taskbox);
}

TaskMeasure::~TaskMeasure(){
    // automatically deleted in the sub-class
}

void TaskMeasure::update(){
    labelType->hide();
    labelPosition->hide();
    labelLength->hide();
    labelArea->hide();

    if (elementInfo && elementInfo->type.empty()) {
        return;
    }

    labelType->setText(QString::fromUtf8(elementInfo->type.c_str()));
    labelType->show();

    if (!elementInfo->pos.IsNull()) {
        labelPosition->setText(
            QString::asprintf("Position X: %.3lf Y: %.3lf Z: %.3lf", elementInfo->pos.x, elementInfo->pos.y, elementInfo->pos.z)
        );
        labelPosition->show();
    }

    if (elementInfo->length > 0.0) {
        labelLength->setText(QString::asprintf("Length: %.3f", elementInfo->length));
        labelLength->show();
    }

    if (elementInfo->area > 0.0) {
        labelArea->setText(QString::asprintf("Area: %.3f", elementInfo->area));
        labelArea->show();
    }

}

void TaskMeasure::close(){

    MDIView* view = getMainWindow()->activeWindow();
    Gui::View3DInventorViewer *viewer = static_cast<Gui::View3DInventor*>(view)->getViewer();
    
    viewer->setEditing(false);
    viewer->removeEventCallback(SoEvent::getClassTypeId(), eventCallback);
    
    Control().closeDialog();
}

bool TaskMeasure::accept(){
    close();
    return false;
}

bool TaskMeasure::reject(){
    close();
    return false;
}


#endif //GUI_TASKMEASURE_H