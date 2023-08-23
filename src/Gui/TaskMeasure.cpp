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

#ifndef GUI_TASKMEASURE_H
#define GUI_TASKMEASURE_H

#include "PreCompiled.h"

#ifndef _PreComp_
# include <QApplication>
# include <QKeyEvent>
#endif


#include "TaskMeasure.h"

#include "Control.h"
#include "MainWindow.h"
#include "Application.h"
#include "App/Document.h"

#include <QFormLayout>
#include <QPushButton>

using namespace Gui;


TaskMeasure::TaskMeasure(){


    qApp->installEventFilter(this);

    measureModule = "";

    this->setButtonPosition(TaskMeasure::South);
    Gui::TaskView::TaskBox* taskbox = new Gui::TaskView::TaskBox(QPixmap(), QString(), true, nullptr);



    labelType = new QLabel();
    labelPosition = new QLabel();
    labelLength = new QLabel();
    labelArea = new QLabel();

    // Create mode dropdown and add all registered measuretypes
    modeSwitch = new QComboBox();
    modeSwitch->addItem(QString::fromLatin1("Auto"));

    for (App::MeasureType* mType : App::GetApplication().getMeasureTypes()){
        modeSwitch->addItem(QString::fromLatin1(mType->label.c_str()));
    }

    // Connect dropdown's change signal to our onModeChange slot
    connect(modeSwitch, qOverload<int>(&QComboBox::currentIndexChanged), this, &TaskMeasure::onModeChanged);


    // Element info layout
    QVBoxLayout* layoutElement = new QVBoxLayout();
    layoutElement->addWidget(labelType);
    layoutElement->addWidget(labelPosition);
    layoutElement->addWidget(labelLength);
    layoutElement->addWidget(labelArea);


    // Result widget
    valueResult = new QLineEdit();
    valueResult->setReadOnly(true);

    // Main layout
    QBoxLayout *layout = taskbox->groupLayout();

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setHorizontalSpacing(10);
    // Note: How can the split between columns be kept in the middle?
    // formLayout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::ExpandingFieldsGrow);
    formLayout->setFormAlignment(Qt::AlignCenter);

    formLayout->addRow(QString::fromLatin1("Mode:"), modeSwitch);
    formLayout->addRow(QString::fromLatin1("Result:"), valueResult);
    layout->addLayout(formLayout);

    layout->addSpacing(10);
    layout->addLayout(layoutElement);


    Content.push_back(taskbox);
    gatherSelection();
    attachSelection();
}

TaskMeasure::~TaskMeasure(){
    detachSelection();
    qApp->removeEventFilter(this);
}


void TaskMeasure::modifyStandardButtons(QDialogButtonBox* box) {

    QPushButton* btn = box->button(QDialogButtonBox::Ok);
    btn->setText(tr("Annotate"));
}

void TaskMeasure::updateInfo() {
    labelType->hide();
    labelPosition->hide();
    labelLength->hide();
    labelArea->hide();

    if (elementInfo.type.empty()) {
        return;
    }


    labelType->setText(QString::fromUtf8(elementInfo.type.c_str()));
    labelType->show();

    if (!elementInfo.pos.IsNull()) {
        labelPosition->setText(
            QString::asprintf("Position X: %.3lf Y: %.3lf Z: %.3lf", elementInfo.pos.x, elementInfo.pos.y, elementInfo.pos.z)
        );
        labelPosition->show();
    }

    if (elementInfo.length > 0.0) {
        labelLength->setText(QString::asprintf("Length: %.3f", elementInfo.length));
        labelLength->show();
    }

    if (elementInfo.area > 0.0) {
        labelArea->setText(QString::asprintf("Area: %.3f", elementInfo.area));
        labelArea->show();
    }
}


void TaskMeasure::update(){
    valueResult->setText(QString::asprintf("-"));

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
        // If the measure mode is explicitly set we only check matching measure types
        if (explicitMode && mType->label.c_str() != modeSwitch->currentText().toLatin1()) {
            continue;
        }
        
        if (!mType->validatorCb(selection)) {
            continue;
        }

        // Check if the measurement type prioritizes the given selection
        bool isPriority = (mType->prioritizeCb != nullptr && mType->prioritizeCb(selection));

        if (!isValid || isPriority) {
            isValid = true;
            measureType = mType;

        }


        // break;
    }

    if (!isValid) {

        // Note: If there's no valid measure type we might just restart the selection,
        // however this requiers enogh coverage of measuretypes that we can access all of them
        
        // std::tuple<std::string, std::string> sel = selection.back();
        // clearSelection();
        // addElement(measureModule.c_str(), get<0>(sel).c_str(), get<1>(sel).c_str());
        return;
    }

    // Update tool mode display
    setModeSilent(measureType);

    if (!_mMeasureObject || measureType->measureObject != _mMeasureObject->getTypeId().getName()) {

        // Remove existing measurement object
        removeObject();

        // Create measure object
        App::Document *doc = App::GetApplication().getActiveDocument();
        _mMeasureObject = (App::MeasurementBase*)doc->addObject(measureType->measureObject.c_str());
    }

    // Fill measure object's properties from selection
    _mMeasureObject->parseSelection(selection);

    // Get result
    valueResult->setText(_mMeasureObject->result().getUserString());

}

void TaskMeasure::close(){
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

void TaskMeasure::reset() {
    // Reset tool state
    this->clearSelection();

    // Should the explicit mode also be reset? 
    // setModeSilent(nullptr);
    // explicitMode = false;

    this->update();
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
    elementInfo.type = info.type;
    elementInfo.pos = info.pos;
    elementInfo.length = info.length;
    elementInfo.area = info.area;

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
    elementInfo.type.clear();
    selection.clear();
}

void TaskMeasure::onSelectionChanged(const Gui::SelectionChanges& msg)
{

    if (msg.Type != SelectionChanges::AddSelection && msg.Type != SelectionChanges::RmvSelection
        && msg.Type != SelectionChanges::SetSelection && msg.Type != SelectionChanges::ClrSelection) {

        return;
        }


    // Add Element 
    if (msg.Type == SelectionChanges::AddSelection || msg.Type == SelectionChanges::SetSelection) {

        App::Document* doc = App::GetApplication().getActiveDocument();
        App::DocumentObject* ob = doc->getObject(msg.pObjectName);
        App::DocumentObject* sub = ob->getSubObject(msg.pSubName);
        std::string mod = sub->getClassTypeId().getModuleName(sub->getTypeId().getName());

        addElement(mod.c_str(), msg.pObjectName, msg.pSubName);
    }

}

bool TaskMeasure::eventFilter(QObject* obj, QEvent* event) {

    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Escape) {

            if (this->hasSelection()) {
                this->reset();
            } else {
                this->reject();
            }

            return true;
        }
        else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            this->accept();
            return true;
        }
    }

    return TaskDialog::eventFilter(obj, event);
}

void TaskMeasure::onModeChanged(int index) {
    explicitMode = (index != 0);
    this->update();
}

void TaskMeasure::setModeSilent(App::MeasureType* mode) {
    modeSwitch->blockSignals(true);
    
    if (mode == nullptr) {
        modeSwitch->setCurrentIndex(0);
    }
    else {
        modeSwitch->setCurrentText(QString::fromLatin1(mode->label.c_str()));
    }
    modeSwitch->blockSignals(false);
}

App::MeasureType* TaskMeasure::getMeasureType() {
    for (App::MeasureType* mType : App::GetApplication().getMeasureTypes()) {
        if (mType->label.c_str() == modeSwitch->currentText().toLatin1()) {
            return mType;
        }
    }
    return nullptr;
}

#endif //GUI_TASKMEASURE_H