/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2021 Syping
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
#include "config.h"
#include <QStringBuilder>
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

#ifdef GTA5SYNC_TELEMETRY
#include "TelemetryClass.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#endif

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

    ui->labSavegamePic->setScaledContents(true);
    ui->labSavegamePic->setPixmap(QPixmap(AppEnv::getImagesFolder() % "/savegame.svgz"));

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
    int uchoice = QMessageBox::question(this, tr("Delete Savegame"), tr("Are you sure to delete %1 from your savegames?").arg("\""+sgdStr+"\""), QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
    if (uchoice == QMessageBox::Yes)
    {
        if (!QFile::exists(sgdPath))
        {
            emit savegameDeleted();
#ifdef GTA5SYNC_TELEMETRY
            QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            telemetrySettings.beginGroup("Telemetry");
            bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
            telemetrySettings.endGroup();
            if (pushUsageData && Telemetry->canPush())
            {
                QJsonDocument jsonDocument;
                QJsonObject jsonObject;
                jsonObject["Type"] = "DeleteSuccess";
                jsonObject["ExtraFlags"] = "Savegame";
#if QT_VERSION >= 0x060000
                jsonObject["DeletedTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                jsonObject["DeletedTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                jsonDocument.setObject(jsonObject);
                Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
            }
#endif
        }
        else if (QFile::remove(sgdPath))
        {
#ifdef GTA5SYNC_TELEMETRY
            QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            telemetrySettings.beginGroup("Telemetry");
            bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
            telemetrySettings.endGroup();
            if (pushUsageData && Telemetry->canPush())
            {
                QJsonDocument jsonDocument;
                QJsonObject jsonObject;
                jsonObject["Type"] = "DeleteSuccess";
                jsonObject["ExtraFlags"] = "Savegame";
#if QT_VERSION >= 0x060000
                jsonObject["DeletedTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                jsonObject["DeletedTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                jsonDocument.setObject(jsonObject);
                Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
            }
#endif
            emit savegameDeleted();
        }
        else
        {
            QMessageBox::warning(this, tr("Delete Savegame"), tr("Failed at deleting %1 from your savegames").arg("\""+sgdStr+"\""));
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
        const int contentMode = getContentMode();
        if ((contentMode == 0 || contentMode == 10 || contentMode == 20) && rect().contains(ev->pos()) && ev->button() == Qt::LeftButton)
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
        else if (!ui->cbSelected->isVisible() && (contentMode == 1 || contentMode == 11 || contentMode == 21) && ev->button() == Qt::LeftButton && ev->modifiers().testFlag(Qt::ShiftModifier))
        {
            ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
        }
    }
}

void SavegameWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    ProfileWidget::mouseDoubleClickEvent(ev);

    const int contentMode = getContentMode();
    if (!ui->cbSelected->isVisible() && (contentMode == 1 || contentMode == 11 || contentMode == 21) && ev->button() == Qt::LeftButton)
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
