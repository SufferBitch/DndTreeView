/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "treemodel.h"

#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QCoreApplication>

void MainWindow::setupUi()
{
    this->resize(600, 450);
    this->setWindowTitle("Tree Model");

    exitAction = new QAction(this);
    exitAction->setText("E&xit");
    exitAction->setShortcut(tr("Ctrl+Q","Ctrl+Q"));

    insertRowAction = new QAction(this);
    insertRowAction->setText("Insert Row");
    removeRowAction = new QAction(this);
    removeRowAction->setText("Remove Row");
    removeRowAction->setShortcut(tr("Del", "Delete"));

    insertChildAction = new QAction(this);
    insertChildAction->setText("Insert Child");

    openAction = new QAction(this);
    openAction->setText("Open");

    saveAction = new QAction(this);
    saveAction->setText("Save");

    centralwidget = new QWidget(this);
    this->setCentralWidget(centralwidget);

    vboxLayout = new QVBoxLayout(centralwidget);
    vboxLayout->setSpacing(0);
    vboxLayout->setContentsMargins(0, 0, 0, 0);

    view = new QTreeView(centralwidget);
    view->setAlternatingRowColors(true);
    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->setAnimated(false);
    view->setAllColumnsShowFocus(true);

    vboxLayout->addWidget(view);


    menubar = new QMenuBar(this);
    menubar->setGeometry(QRect(0, 0, this->width(), 20));

    fileMenu = new QMenu(menubar);
    fileMenu->setTitle("&File");

    actionsMenu = new QMenu(menubar);
    actionsMenu->setTitle("&Actions");

    this->setMenuBar(menubar);
    menubar->addAction(fileMenu->menuAction());
    menubar->addAction(actionsMenu->menuAction());

    fileMenu->addAction(exitAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);

    actionsMenu->addAction(insertRowAction);
    actionsMenu->addSeparator();
    actionsMenu->addAction(removeRowAction);
    actionsMenu->addSeparator();
    actionsMenu->addAction(insertChildAction);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    QFile file(":/default.txt");
    file.open(QIODevice::ReadOnly);
    model = new TreeModel(file.readAll(), this);
    file.close();
    view->setModel(model);
    view->setDragEnabled(true);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setDragDropMode(QAbstractItemView::InternalMove);
    view->setAcceptDrops(true);
    view->setDropIndicatorShown(true);
    view->setAlternatingRowColors(true);

    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged,this, &MainWindow::updateActions);
    connect(actionsMenu, &QMenu::aboutToShow, this, &MainWindow::updateActions);
    connect(insertRowAction, &QAction::triggered, this, &MainWindow::insertRow);
    connect(openAction, &QAction::triggered, this, &MainWindow::readDataFromFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveDataToFile);
    connect(removeRowAction, &QAction::triggered, this, &MainWindow::removeRow);
    connect(insertChildAction, &QAction::triggered, this, &MainWindow::insertChild);

    updateActions();
}


bool MainWindow::saveDataToFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,"Save File",QDir::currentPath(),tr("Text Files(*.txt)"));

    if (fileName.isEmpty())
        return  false;

    QFile f(fileName+".txt");
    f.open(QIODevice::WriteOnly);
    f.write(model->getModelData());
    f.close();
    return true;
}


bool MainWindow::readDataFromFile()
{
    QString filename = QFileDialog::getOpenFileName(this,"Open File",QDir::currentPath(),tr("Text Files(*.txt)"));
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray ar = file.readAll();
        file.close();
        if (ar.isEmpty())
            return false;
        else
        {
            delete model;
            model = new TreeModel(ar, this);
            view->setModel(model);
        }
        return true;
    }
    return false;
}


void MainWindow::insertChild()
{
    const QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    for (int column = 0; column < model->columnCount(index); ++column) {
        const QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant(tr("[No data]")), Qt::EditRole);
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant(tr("[No header]")), Qt::EditRole);
    }

    view->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);
    updateActions();
}

bool MainWindow::insertColumn()
{
    QAbstractItemModel *model = view->model();
    int column = view->selectionModel()->currentIndex().column();

    bool changed = model->insertColumn(column + 1);
    if (changed)
        model->setHeaderData(column + 1, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    updateActions();
    return changed;
}

void MainWindow::insertRow()
{
    const QModelIndex index = view->selectionModel()->currentIndex();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    updateActions();
    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
        const QModelIndex child = model->index(index.row() + 1, column, index.parent());
        model->setData(child, QVariant(tr("[No data]")), Qt::EditRole);
    }
}

void MainWindow::removeRow()
{
    const QModelIndex index = view->selectionModel()->currentIndex();
    qDebug() << index.row() << index.parent().data();
    if (model->removeRows(index.row(),1,index.parent()))
        updateActions();
}
void MainWindow::updateActions()
{
    const bool hasSelection = !view->selectionModel()->selection().isEmpty();
    removeRowAction->setEnabled(hasSelection);
    const bool hasCurrent = view->selectionModel()->currentIndex().isValid();
    insertRowAction->setEnabled(hasCurrent);

}

