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
#include "Application.h"
#include "App/Document.h"


using namespace Gui;


TaskMeasure::TaskMeasure(){


    measureModule = "";

    this->setButtonPosition(TaskMeasure::South);
    Gui::TaskView::TaskBox* taskbox = new Gui::TaskView::TaskBox(QPixmap(), QString(), true, nullptr);

    labelMeasureType = new QLabel();
    labelResult = new QLabel();

    labelType = new QLabel();
    labelPosition = new QLabel();
    labelLength = new QLabel();
    labelArea = new QLabel();

    QBoxLayout *layout = taskbox->groupLayout();
    layout->addWidget(labelMeasureType);
    layout->addWidget(labelResult);

    
    layout->addWidget(labelType);
    layout->addWidget(labelPosition);
    layout->addWidget(labelLength);
    layout->addWidget(labelArea);

    labelMeasureType->setText(QString::asprintf("Measure Type: -"));
    labelResult->setText(QString::asprintf("Result: -"));

    Content.push_back(taskbox);

    gatherSelection();
    attachSelection();
}

TaskMeasure::~TaskMeasure(){
    detachSelection();
}

void TaskMeasure::updateInfo() {
    labelType->hide();
    labelPosition->hide();
    labelLength->hide();
    labelArea->hide();

    if (!elementInfo || elementInfo->type.empty()) {
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


void TaskMeasure::update(){
    labelMeasureType->setText(QString::asprintf("Measure Type: -"));
    labelResult->setText(QString::asprintf("Result: -"));

    // Update element info display
    updateInfo();

    // Report selection stack
    Base::Console().Message("Selection: ");
    for (std::tuple<std::string, std::string> elem : selection) {
        Base::Console().Message("%s ", std::get<1>(elem));
    }
    Base::Console().Message("\n");


    // Get valid measure type
    bool isValid = false;
    App::MeasureType *measureType;

    for (App::MeasureType* mType : App::GetApplication().getMeasureTypes()){
        if (!mType->validatorCb(selection)) {
            continue;
        }
        isValid = true;
        measureType = mType;
        break;
    }

    if (!isValid) {

        // Note: If there's no valid measure type we might just restart the selection,
        // however this requiers enogh coverage of measuretypes that we can access all of them
        
        // std::tuple<std::string, std::string> sel = selection.back();
        // clearSelection();
        // addElement(measureModule.c_str(), get<0>(sel).c_str(), get<1>(sel).c_str());
        return;
    }


    if (!_mMeasureObject || measureType->measureObject != _mMeasureObject->getTypeId().getName()) {

        // Remove existing measurement object
        removeObject();

        // Create measure object
        App::Document *doc = App::GetApplication().getActiveDocument();
        _mMeasureObject = (App::MeasurementBase*)doc->addObject(measureType->measureObject.c_str());
    }
    
    // Set type label
    labelMeasureType->setText(QString::asprintf("Measure Type: %s", measureType->measureObject.c_str()));

    // Fill measure object's properties from selection
    _mMeasureObject->parseSelection(selection);

    // Get result
    labelResult->setText(QString::asprintf("Result: %f", _mMeasureObject->result()));

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
    removeObject();

    close();
    return false;
}


void TaskMeasure::addElement(const char* mod, const char* obName, const char* subName) {

    // Note: Currently only a selection of elements that belong to the same module is allowed
    if (strcmp(mod, measureModule.c_str())){
        clearSelection();
    }

    if (!App::GetApplication().hasMeasureHandler(mod)) {
        Base::Console().Message("No measure handler available for geometry of module: %s\n", mod);
        return;
    }

    measureModule = mod;
    selection.push_back(std::make_tuple((std::string)obName, (std::string)subName));

    // Update element info
    App::MeasureHandler handler = App::GetApplication().getMeasureHandler(mod);
    auto info = handler.infoCb(obName, subName);
    elementInfo = &info;

    update();
}

void TaskMeasure::gatherSelection() {
    // Fills the selection stack from the global selection and triggers an update

    if (!Gui::Selection().hasSelection()) {
        return;
    }

    App::Document* doc = App::GetApplication().getActiveDocument();

    for (auto sel : Gui::Selection().getSelection()) {
        const char* obName = sel.pObject->getNameInDocument();
        App::DocumentObject* ob = doc->getObject(obName);
        auto sub = ob->getSubObject(sel.SubName);
        std::string mod = sub->getClassTypeId().getModuleName(sub->getTypeId().getName());

        if (mod != measureModule){
            clearSelection();
        }
        measureModule = mod;
        selection.push_back(std::tuple<std::string, std::string>(obName, sel.SubName));
    }

    update();
}

void TaskMeasure::removeObject() {
    if (_mMeasureObject == nullptr) {
        return;
    }
    if (_mMeasureObject->isRemoving() ) {
        return;
    }
    _mMeasureObject->getDocument()->removeObject (_mMeasureObject->getNameInDocument());
    _mMeasureObject = nullptr;
}

bool TaskMeasure::hasSelection(){
    return selection.size() > 0;
}

void TaskMeasure::clearSelection(){
    elementInfo = nullptr;
    selection.clear();
}

void TaskMeasure::onSelectionChanged(const Gui::SelectionChanges& msg)
{

    if (msg.Type != SelectionChanges::AddSelection && msg.Type != SelectionChanges::RmvSelection
        && msg.Type != SelectionChanges::SetSelection && msg.Type != SelectionChanges::ClrSelection)
        return;


    // Add Element 
    if (msg.Type == SelectionChanges::AddSelection || msg.Type == SelectionChanges::SetSelection) {

        App::Document* doc = App::GetApplication().getActiveDocument();
        App::DocumentObject* ob = doc->getObject(msg.pObjectName);
        App::DocumentObject* sub = ob->getSubObject(msg.pSubName);
        std::string mod = sub->getClassTypeId().getModuleName(sub->getTypeId().getName());

        addElement(mod.c_str(), msg.pObjectName, msg.pSubName);
    }

}

#endif //GUI_TASKMEASURE_H