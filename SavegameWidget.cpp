/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016-2017 Syping
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
#include "SavegameDialog.h"
#include "StandardPaths.h"
#include "SavegameData.h"
#include "SavegameCopy.h"
#include "AppEnv.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QPalette>
#include <QColor>
#include <QBrush>
#include <QTimer>
#include <QDebug>
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

    qreal screenRatio = AppEnv::screenRatio();
    ui->labSavegamePic->setFixedSize(48 * screenRatio, 27 * screenRatio);

    QPixmap savegamePixmap(":/img/savegame.png");
    if (screenRatio != 1) savegamePixmap = savegamePixmap.scaledToHeight(ui->labSavegamePic->height(), Qt::SmoothTransformation);
    ui->labSavegamePic->setPixmap(savegamePixmap);

    QString exportSavegameStr = tr("Export Savegame...");
    Q_UNUSED(exportSavegameStr)

    labelAutosaveStr = tr("AUTOSAVE - %1\n%2");
    labelSaveStr = tr("SAVE %3 - %1\n%2");

    ui->SavegameFrame->setMouseTracking(true);
    ui->labSavegamePic->setMouseTracking(true);
    ui->labSavegameStr->setMouseTracking(true);
    ui->cbSelected->setMouseTracking(true);
    sgdata = nullptr;
}

SavegameWidget::~SavegameWidget()
{
    delete ui;
}

void SavegameWidget::setSavegameData(SavegameData *savegame, QString savegamePath)
{
    // BETA CODE
    QString savegameString = savegame->getSavegameStr();
    QString fileName = QFileInfo(savegame->getSavegameFileName()).fileName();
    renderString(savegameString, fileName);
    sgdStr = savegameString;
    sgdPath = savegamePath;
    sgdata = savegame;
}

void SavegameWidget::renderString(const QString &savegameString, const QString &fileName)
{
    bool validNumber;
    QString savegameName = tr("WRONG FORMAT");
    QString savegameDate = tr("WRONG FORMAT");
    QStringList savegameNDL = QString(savegameString).split(" - ");
    if (savegameNDL.length() >= 2)
    {
        savegameDate = savegameNDL.at(savegameNDL.length() - 1);
        savegameName = QString(savegameString).remove(savegameString.length() - savegameDate.length() - 3, savegameDate.length() + 3);
    }
    int savegameNumber = QString(fileName).remove(0,5).toInt(&validNumber) + 1;
    if (validNumber)
    {
        if (savegameNumber == 16)
        {
            ui->labSavegameStr->setText(labelAutosaveStr.arg(savegameDate, savegameName));
        }
        else
        {
            ui->labSavegameStr->setText(labelSaveStr.arg(savegameDate, savegameName, QString::number(savegameNumber)));
        }
    }
    else
    {
        ui->labSavegameStr->setText(labelSaveStr.arg(savegameDate, savegameName, tr("UNKNOWN")));
    }
}

void SavegameWidget::retranslate()
{
    labelAutosaveStr = tr("AUTOSAVE - %1\n%2");
    labelSaveStr = tr("SAVE %3 - %1\n%2");

    QString fileName = QFileInfo(sgdata->getSavegameFileName()).fileName();
    renderString(sgdStr, fileName);
}

void SavegameWidget::on_cmdCopy_clicked()
{
    SavegameCopy::copySavegame(this, sgdPath);
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
    savegameDialog->setSavegameData(sgdata, sgdPath, true);
    savegameDialog->setModal(true);
#ifdef Q_OS_ANDROID
    // Android ...
    savegameDialog->showMaximized();
#else
    savegameDialog->show();
#endif
    savegameDialog->exec();
    delete savegameDialog;
}

void SavegameWidget::mousePressEvent(QMouseEvent *ev)
{
    ProfileWidget::mousePressEvent(ev);
}

void SavegameWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    ProfileWidget::mouseReleaseEvent(ev);
    if (ui->cbSelected->isVisible())
    {
        if (rect().contains(ev->pos()) && ev->button() == Qt::LeftButton)
        {
            ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
        }
    }
    else
    {
        if (getContentMode() == 0 && rect().contains(ev->pos()) && ev->button() == Qt::LeftButton)
        {
            if (ev->modifiers().testFlag(Qt::ShiftModifier))
            {
                ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
            }
            else
            {
                on_cmdView_clicked();
            }
        }
        else if (!ui->cbSelected->isVisible() && getContentMode() == 1 && ev->button() == Qt::LeftButton && ev->modifiers().testFlag(Qt::ShiftModifier))
        {
            ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
        }
    }
}

void SavegameWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    ProfileWidget::mouseDoubleClickEvent(ev);

    if (!ui->cbSelected->isVisible() && getContentMode() == 1 && ev->button() == Qt::LeftButton)
    {
        on_cmdView_clicked();
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
    emit contextMenuTriggered(ev);
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

QString SavegameWidget::getWidgetType()
{
    return "SavegameWidget";
}
