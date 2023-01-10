/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2017-2021 Syping
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

#include "JsonEditorDialog.h"
#include "ui_JsonEditorDialog.h"
#include "SnapmaticEditor.h"
#include "AppEnv.h"
#include "config.h"
#include <QStringBuilder>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#if QT_VERSION >= 0x050200
#include <QFontDatabase>
#endif

#ifdef GTA5SYNC_TELEMETRY
#include "TelemetryClass.h"
#endif

JsonEditorDialog::JsonEditorDialog(SnapmaticPicture *picture, QWidget *parent) :
    QDialog(parent), smpic(picture),
    ui(new Ui::JsonEditorDialog)
{
    // Set Window Flags
#if QT_VERSION >= 0x050900
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setWindowFlag(Qt::WindowMinMaxButtonsHint, true);
#else
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint^Qt::WindowMinMaxButtonsHint);
#endif

    ui->setupUi(this);
    ui->cmdClose->setDefault(true);
    ui->cmdClose->setFocus();

    // Set Icon for Close Button
    if (QIcon::hasThemeIcon("dialog-close")) {
        ui->cmdClose->setIcon(QIcon::fromTheme("dialog-close"));
    }
    else if (QIcon::hasThemeIcon("gtk-close")) {
        ui->cmdClose->setIcon(QIcon::fromTheme("gtk-close"));
    }

    // Set Icon for Save Button
    if (QIcon::hasThemeIcon("document-save")) {
        ui->cmdSave->setIcon(QIcon::fromTheme("document-save"));
    }
    else if (QIcon::hasThemeIcon("gtk-save")) {
        ui->cmdSave->setIcon(QIcon::fromTheme("gtk-save"));
    }

    jsonCode = picture->getJsonStr();

#if QT_VERSION >= 0x050200
    ui->txtJSON->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
#else
    QFont jsonFont = ui->txtJSON->font();
    jsonFont.setStyleHint(QFont::Monospace);
    jsonFont.setFixedPitch(true);
    ui->txtJSON->setFont(jsonFont);
#endif
    QFontMetrics fontMetrics(ui->txtJSON->font());
#if QT_VERSION >= 0x050B00
    ui->txtJSON->setTabStopDistance(fontMetrics.horizontalAdvance("    "));
#else
    ui->txtJSON->setTabStopWidth(fontMetrics.width("    "));
#endif

    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonCode.toUtf8());
    ui->txtJSON->setStyleSheet("QPlainTextEdit{background-color: rgb(46, 47, 48); color: rgb(238, 231, 172);}");
    ui->txtJSON->setPlainText(QString::fromUtf8(jsonDocument.toJson(QJsonDocument::Indented)).trimmed());
    jsonHl = new JSHighlighter(ui->txtJSON->document());

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
#ifndef Q_OS_MAC
    ui->hlButtons->setSpacing(6 * screenRatio);
    ui->hlButtons->setContentsMargins(9 * screenRatio, 0, 9 * screenRatio, 0);
    ui->vlInterface->setContentsMargins(0, 0, 0, 9 * screenRatio);
#else
    ui->hlButtons->setSpacing(6 * screenRatio);
    ui->hlButtons->setContentsMargins(9 * screenRatio, 0, 9 * screenRatio, 0);
    ui->vlInterface->setContentsMargins(0, 0, 0, 9 * screenRatio);
#endif
    if (screenRatio > 1) {
        ui->lineJSON->setMinimumHeight(qRound(1 * screenRatio));
        ui->lineJSON->setMaximumHeight(qRound(1 * screenRatio));
        ui->lineJSON->setLineWidth(qRound(1 * screenRatio));
    }
    resize(450 * screenRatio, 550 * screenRatio);
}

JsonEditorDialog::~JsonEditorDialog()
{
    delete jsonHl;
    delete ui;
}

void JsonEditorDialog::closeEvent(QCloseEvent *ev)
{
    QString jsonPatched = QString(ui->txtJSON->toPlainText()).replace("\t", "    ");
    QJsonDocument jsonNew = QJsonDocument::fromJson(jsonPatched.toUtf8());
    QJsonDocument jsonOriginal = QJsonDocument::fromJson(jsonCode.toUtf8());
    QString originalCode = QString::fromUtf8(jsonOriginal.toJson(QJsonDocument::Compact));
    QString newCode = QString::fromUtf8(jsonNew.toJson(QJsonDocument::Compact));
    if (newCode != originalCode) {
        QMessageBox::StandardButton button = QMessageBox::warning(this, SnapmaticEditor::tr("Snapmatic Properties"), SnapmaticEditor::tr("<h4>Unsaved changes detected</h4>You want to save the JSON content before you quit?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
        if (button == QMessageBox::Yes) {
            if (saveJsonContent()) {
                ev->accept();
            }
            else {
                ev->ignore();
            }
            return;
        }
        else if (button == QMessageBox::No) {
            ev->accept();
            return;
        }
        else {
            ev->ignore();
            return;
        }
    }
}

bool JsonEditorDialog::saveJsonContent()
{
    QString jsonPatched = QString(ui->txtJSON->toPlainText()).replace("\t", "    ");
    QJsonDocument jsonNew = QJsonDocument::fromJson(jsonPatched.toUtf8());
    if (!jsonNew.isEmpty()) {
        QJsonDocument jsonOriginal = QJsonDocument::fromJson(jsonCode.toUtf8());
        QString originalCode = QString::fromUtf8(jsonOriginal.toJson(QJsonDocument::Compact));
        QString newCode = QString::fromUtf8(jsonNew.toJson(QJsonDocument::Compact));
        if (newCode != originalCode) {
            QString currentFilePath = smpic->getPictureFilePath();
            QString originalFilePath = smpic->getOriginalPictureFilePath();
            QString backupFileName = originalFilePath % ".bak";
            if (!QFile::exists(backupFileName)) {
                QFile::copy(currentFilePath, backupFileName);
            }
            smpic->setJsonStr(newCode, true);
            if (!smpic->isJsonOk()) {
                QString lastStep = smpic->getLastStep(false);
                QString readableError;
                if (lastStep.contains("JSONINCOMPLETE") && lastStep.contains("JSONERROR")) {
                    readableError = SnapmaticPicture::tr("JSON is incomplete and malformed");
                }
                else if (lastStep.contains("JSONINCOMPLETE")) {
                    readableError = SnapmaticPicture::tr("JSON is incomplete");
                }
                else if (lastStep.contains("JSONERROR")) {
                    readableError = SnapmaticPicture::tr("JSON is malformed");
                }
                else {
                    readableError = tr("JSON Error");
                }
                QMessageBox::warning(this, SnapmaticEditor::tr("Snapmatic Properties"), SnapmaticEditor::tr("Patching of Snapmatic Properties failed because of %1").arg(readableError));
                smpic->setJsonStr(originalCode, true);
                return false;
            }
            if (!smpic->exportPicture(currentFilePath)) {
                QMessageBox::warning(this, SnapmaticEditor::tr("Snapmatic Properties"), SnapmaticEditor::tr("Patching of Snapmatic Properties failed because of I/O Error"));
                smpic->setJsonStr(originalCode, true);
                return false;
            }
            jsonCode = newCode;
            smpic->updateStrings();
            smpic->emitUpdate();
#ifdef GTA5SYNC_TELEMETRY
            QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            telemetrySettings.beginGroup("Telemetry");
            bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
            telemetrySettings.endGroup();
            if (pushUsageData && Telemetry->canPush()) {
                QJsonDocument jsonDocument;
                QJsonObject jsonObject;
                jsonObject["Type"] = "JSONEdited";
                jsonObject["EditedSize"] = QString::number(smpic->getContentMaxLength());
#if QT_VERSION >= 0x060000
                jsonObject["EditedTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                jsonObject["EditedTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                jsonDocument.setObject(jsonObject);
                Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
            }
#endif
            return true;
        }
        return true;
    }
    else {
        QMessageBox::warning(this, SnapmaticEditor::tr("Snapmatic Properties"), SnapmaticEditor::tr("Patching of Snapmatic Properties failed because of JSON Error"));
        return false;
    }
}

void JsonEditorDialog::on_cmdClose_clicked()
{
    close();
}

void JsonEditorDialog::on_cmdSave_clicked()
{
    if (saveJsonContent())
        close();
}
