/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "SavegameWidget.h"
#include "ui_SavegameWidget.h"
#include "SidebarGenerator.h"
#include "ProfileInterface.h"
#include "SavegameDialog.h"
#include "StandardPaths.h"
#include "SavegameData.h"
#include "SavegameCopy.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QPalette>
#include <QColor>
#include <QBrush>
#include <QTimer>
#include <QFile>
#include <QMenu>
#include <QUrl>

SavegameWidget::SavegameWidget(QWidget *parent) :
    ProfileWidget(parent),
    ui(new Ui::SavegameWidget)
{
    ui->setupUi(this);
    ui->cmdCopy->setVisible(false);
    ui->cmdView->setVisible(false);
    ui->cmdDelete->setVisible(false);
    ui->cbSelected->setVisible(false);

    QPalette palette;
    QColor highlightBackColor = palette.highlight().color();
    QColor highlightTextColor = palette.highlightedText().color();
    setStyleSheet(QString("QFrame:hover#SavegameFrame{background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));

    clkIssued = 0;
    sgdPath = "";
    sgdStr = "";
    sgdata = 0;
}

SavegameWidget::~SavegameWidget()
{
    delete ui;
}

void SavegameWidget::setSavegameData(SavegameData *savegame, QString savegamePath)
{
    ui->labSavegameStr->setText(savegame->getSavegameStr());
    sgdStr = savegame->getSavegameStr();
    sgdPath = savegamePath;
    sgdata = savegame;
}

void SavegameWidget::on_cmdCopy_clicked()
{
    SavegameCopy::CopySavegame(this, sgdPath);
}

void SavegameWidget::on_cmdDelete_clicked()
{
    int uchoice = QMessageBox::question(this, tr("Delete savegame"), tr("Are you sure to delete %1 from your savegames?").arg("\""+sgdStr+"\""), QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
    if (uchoice == QMessageBox::Yes)
    {
        if (!QFile::exists(sgdPath))
        {
            emit savegameDeleted();
        }
        else if(QFile::remove(sgdPath))
        {
            emit savegameDeleted();
        }
        else
        {
            QMessageBox::warning(this, tr("Delete savegame"), tr("Failed at deleting %1 from your savegames").arg("\""+sgdStr+"\""));
        }
    }
}

void SavegameWidget::on_cmdView_clicked()
{
    SavegameDialog *savegameDialog = new SavegameDialog(this);
    savegameDialog->setWindowFlags(savegameDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
    savegameDialog->setSavegameData(sgdata, sgdPath, true);
    savegameDialog->setModal(true);
    savegameDialog->show();
    savegameDialog->exec();
    savegameDialog->deleteLater();
    delete savegameDialog;
}

void SavegameWidget::mousePressEvent(QMouseEvent *ev)
{
    ProfileWidget::mouseReleaseEvent(ev);
    clkIssued = true;
}

void SavegameWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    ProfileWidget::mouseReleaseEvent(ev);
    if (ui->cbSelected->isVisible())
    {
        if (rect().contains(ev->pos()) && ev->button() == Qt::LeftButton)
        {
            clkIssued = false;
            //QTimer::singleShot(QApplication::doubleClickInterval(), this, SLOT(changeCheckedState()));
            ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
        }
    }
    else
    {
        if (rect().contains(ev->pos()) && ev->button() == Qt::LeftButton)
        {
            on_cmdView_clicked();
        }
    }
}

void SavegameWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    QWidget::mouseDoubleClickEvent(ev);

//  if (ev->button() == Qt::LeftButton)
//  {
//      clkIssued = true;
//      on_cmdView_clicked();
//  }
}

void SavegameWidget::changeCheckedState()
{
    if (!clkIssued)
    {
        ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
    }
}

void SavegameWidget::setSelected(bool isSelected)
{
    ui->cbSelected->setChecked(isSelected);
}

void SavegameWidget::savegameSelected()
{
    setSelected(!ui->cbSelected->isChecked());
}

void SavegameWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu contextMenu(this);
    if (!ui->cbSelected->isVisible())
    contextMenu.addAction(tr("View"), this, SLOT(on_cmdView_clicked()));
    contextMenu.addAction(tr("Copy"), this, SLOT(on_cmdCopy_clicked()));
    contextMenu.addAction(tr("Delete"), this, SLOT(on_cmdDelete_clicked()));
    if (ui->cbSelected->isVisible())
    {
        contextMenu.addSeparator();
        if (!ui->cbSelected->isChecked()) { contextMenu.addAction(tr("Select"), this, SLOT(savegameSelected())); }
        if (ui->cbSelected->isChecked()) { contextMenu.addAction(tr("Deselect"), this, SLOT(savegameSelected())); }
        contextMenu.addAction(tr("Select all"), this, SLOT(selectAllWidgets()), QKeySequence::fromString(tr("Ctrl+S")));
        contextMenu.addAction(tr("Deselect all"), this, SLOT(deselectAllWidgets()), QKeySequence::fromString(tr("Shift+S")));
    }
    else
    {
        contextMenu.addSeparator();
        contextMenu.addAction(tr("Select"), this, SLOT(savegameSelected()));
        contextMenu.addAction(tr("Select all"), this, SLOT(selectAllWidgets()), QKeySequence::fromString(tr("Ctrl+S")));
    }
    contextMenu.exec(ev->globalPos());
    setStyleSheet(styleSheet()); // fix multi highlight bug
}

void SavegameWidget::on_cbSelected_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        emit widgetSelected();
    }
    else if (arg1 == Qt::Unchecked)
    {
        emit widgetDeselected();
    }
}

bool SavegameWidget::isSelected()
{
    return ui->cbSelected->isChecked();
}

void SavegameWidget::setSelectionMode(bool selectionMode)
{
    ui->cbSelected->setVisible(selectionMode);
}

void SavegameWidget::selectAllWidgets()
{
    emit allWidgetsSelected();
}

void SavegameWidget::deselectAllWidgets()
{
    emit allWidgetsDeselected();
}

SavegameData* SavegameWidget::getSavegame()
{
    return sgdata;
}
