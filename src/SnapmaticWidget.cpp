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

#include "SnapmaticWidget.h"
#include "ui_SnapmaticWidget.h"
#include "MapLocationDialog.h"
#include "JsonEditorDialog.h"
#include "SnapmaticPicture.h"
#include "SnapmaticEditor.h"
#include "DatabaseThread.h"
#include "PictureDialog.h"
#include "PictureExport.h"
#include "StringParser.h"
#include "ImportDialog.h"
#include "AppEnv.h"
#include "config.h"
#include <QStringBuilder>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QMenu>
#include <QFile>

#ifdef GTA5SYNC_TELEMETRY
#include "TelemetryClass.h"
#include <QJsonDocument>
#include <QJsonObject>
#endif

SnapmaticWidget::SnapmaticWidget(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QString profileName, QWidget *parent) :
    ProfileWidget(parent), profileDB(profileDB), crewDB(crewDB), threadDB(threadDB), profileName(profileName),
    ui(new Ui::SnapmaticWidget)
{
    ui->setupUi(this);
    ui->cmdView->setVisible(false);
    ui->cmdCopy->setVisible(false);
    ui->cmdExport->setVisible(false);
    ui->cmdDelete->setVisible(false);
    ui->cbSelected->setVisible(false);

    QPalette palette;
    palette.setCurrentColorGroup(QPalette::Disabled);
    highlightHiddenColor = palette.text().color();

    ui->SnapmaticFrame->setMouseTracking(true);
    ui->labPicture->setMouseTracking(true);
    ui->labPicStr->setMouseTracking(true);
    ui->cbSelected->setMouseTracking(true);
    smpic = nullptr;
}

SnapmaticWidget::~SnapmaticWidget()
{
    delete ui;
}

void SnapmaticWidget::setSnapmaticPicture(SnapmaticPicture *picture)
{
    smpic = picture;
    QObject::connect(picture, SIGNAL(updated()), this, SLOT(snapmaticUpdated()));
    QObject::connect(picture, SIGNAL(customSignal(QString)), this, SLOT(customSignal(QString)));

    const qreal screenRatio = AppEnv::screenRatio();
    const qreal screenRatioPR = AppEnv::screenRatioPR();
    const QSize renderResolution(48 * screenRatio * screenRatioPR, 27 * screenRatio * screenRatioPR);
    ui->labPicture->setFixedSize(48 * screenRatio, 27 * screenRatio);
    ui->labPicture->setScaledContents(true);

    QPixmap renderPixmap(renderResolution);
    renderPixmap.fill(Qt::transparent);
    QPainter renderPainter(&renderPixmap);
    const QImage originalImage = picture->getImage();
    const QImage renderImage = originalImage.scaled(renderResolution, Qt::KeepAspectRatio, Qt::SmoothTransformation); // Stack smash
    if (renderImage.width() < renderResolution.width()) {
        renderPainter.drawImage((renderResolution.width() - renderImage.width()) / 2, 0, renderImage, Qt::AutoColor);
    }
    else if (renderImage.height() < renderResolution.height()) {
        renderPainter.drawImage(0, (renderResolution.height() - renderImage.height()) / 2, renderImage, Qt::AutoColor);
    }
    else {
        renderPainter.drawImage(0, 0, renderImage, Qt::AutoColor);
    }
    renderPainter.end();
#if QT_VERSION >= 0x050600
    renderPixmap.setDevicePixelRatio(screenRatioPR);
#endif

    ui->labPicStr->setText(smpic->getPictureStr() % "\n" % smpic->getPictureTitl());
    ui->labPicture->setPixmap(renderPixmap);

    picture->clearCache();

    adjustTextColor();
}

void SnapmaticWidget::snapmaticUpdated()
{
    ui->labPicStr->setText(smpic->getPictureStr() % "\n" % smpic->getPictureTitl());
}

void SnapmaticWidget::customSignal(QString signal)
{
    if (signal == "PictureUpdated") {
        QPixmap SnapmaticPixmap = QPixmap::fromImage(smpic->getImage().scaled(ui->labPicture->width(), ui->labPicture->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), Qt::AutoColor);
        ui->labPicture->setPixmap(SnapmaticPixmap);
    }
}

void SnapmaticWidget::retranslate()
{
    smpic->updateStrings();
    ui->labPicStr->setText(smpic->getPictureStr() % "\n" % smpic->getPictureTitl());
}

void SnapmaticWidget::on_cmdView_clicked()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Interface");
    bool navigationBar = settings.value("NavigationBar", true).toBool();
    settings.endGroup();

    PictureDialog *picDialog = new PictureDialog(profileDB, crewDB, profileName, this);
    picDialog->setSnapmaticPicture(smpic, true);
    picDialog->setModal(true);

    // be ready for crewName and playerName updated
    QObject::connect(threadDB, SIGNAL(crewNameUpdated()), picDialog, SLOT(crewNameUpdated()));
    QObject::connect(threadDB, SIGNAL(playerNameUpdated()), picDialog, SLOT(playerNameUpdated()));
    QObject::connect(picDialog, SIGNAL(nextPictureRequested()), this, SLOT(dialogNextPictureRequested()));
    QObject::connect(picDialog, SIGNAL(previousPictureRequested()), this, SLOT(dialogPreviousPictureRequested()));

    // add previous next buttons
    if (navigationBar)
        picDialog->addPreviousNextButtons();

    // show picture dialog
#ifdef Q_OS_ANDROID
    // Android ...
    picDialog->showMaximized();
#else
    picDialog->show();
    if (navigationBar) picDialog->styliseDialog();
    //picDialog->adaptNewDialogSize();
    picDialog->setMinimumSize(picDialog->size());
    picDialog->setMaximumSize(picDialog->size());
#endif
    picDialog->exec();
    delete picDialog;
}

void SnapmaticWidget::on_cmdCopy_clicked()
{
    PictureExport::exportAsSnapmatic(this, smpic);
}

void SnapmaticWidget::on_cmdExport_clicked()
{
    PictureExport::exportAsPicture(this, smpic);
}

void SnapmaticWidget::on_cmdDelete_clicked()
{
    if (deletePicture())
        emit pictureDeleted();
}

bool SnapmaticWidget::deletePicture()
{
    int uchoice = QMessageBox::question(this, tr("Delete picture"), tr("Are you sure to delete %1 from your Snapmatic pictures?").arg("\""+smpic->getPictureTitle()+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (uchoice == QMessageBox::Yes) {
        if (smpic->deletePictureFile()) {
#ifdef GTA5SYNC_TELEMETRY
            QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            telemetrySettings.beginGroup("Telemetry");
            bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
            telemetrySettings.endGroup();
            if (pushUsageData && Telemetry->canPush()) {
                QJsonDocument jsonDocument;
                QJsonObject jsonObject;
                jsonObject["Type"] = "DeleteSuccess";
                jsonObject["ExtraFlags"] = "Snapmatic";
                jsonObject["DeletedSize"] = QString::number(smpic->getContentMaxLength());
#if QT_VERSION >= 0x060000
                jsonObject["DeletedTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                jsonObject["DeletedTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                jsonDocument.setObject(jsonObject);
                Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
            }
#endif
            return true;
        }
        else {
            QMessageBox::warning(this, tr("Delete picture"), tr("Failed at deleting %1 from your Snapmatic pictures").arg("\""+smpic->getPictureTitle()+"\""));
        }
    }
    return false;
}

void SnapmaticWidget::mousePressEvent(QMouseEvent *ev)
{
    ProfileWidget::mousePressEvent(ev);
}

void SnapmaticWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    ProfileWidget::mouseReleaseEvent(ev);
    if (ui->cbSelected->isVisible()) {
        if (rect().contains(ev->pos()) && ev->button() == Qt::LeftButton) {
            ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
        }
    }
    else {
        const int contentMode = getContentMode();
        if ((contentMode == 0 || contentMode == 10 || contentMode == 20) && rect().contains(ev->pos()) && ev->button() == Qt::LeftButton) {
            if (ev->modifiers().testFlag(Qt::ShiftModifier)) {
                ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
            }
            else {
                on_cmdView_clicked();
            }
        }
        else if (!ui->cbSelected->isVisible() && (contentMode == 1 || contentMode == 11 || contentMode == 21) && ev->button() == Qt::LeftButton && ev->modifiers().testFlag(Qt::ShiftModifier)) {
            ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
        }
    }
}

void SnapmaticWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    ProfileWidget::mouseDoubleClickEvent(ev);

    const int contentMode = getContentMode();
    if (!ui->cbSelected->isVisible() && (contentMode == 1 || contentMode == 11 || contentMode == 21) && ev->button() == Qt::LeftButton) {
        on_cmdView_clicked();
    }
}

void SnapmaticWidget::setSelected(bool isSelected)
{
    ui->cbSelected->setChecked(isSelected);
}

void SnapmaticWidget::pictureSelected()
{
    setSelected(!ui->cbSelected->isChecked());
}

void SnapmaticWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    emit contextMenuTriggered(ev);
}

void SnapmaticWidget::dialogNextPictureRequested()
{
    emit nextPictureRequested((QWidget*)sender());
}

void SnapmaticWidget::dialogPreviousPictureRequested()
{
    emit previousPictureRequested((QWidget*)sender());
}

void SnapmaticWidget::on_cbSelected_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        emit widgetSelected();
    }
    else if (arg1 == Qt::Unchecked) {
        emit widgetDeselected();
    }
}

void SnapmaticWidget::adjustTextColor()
{
    if (isHidden()) {
        ui->labPicStr->setStyleSheet(QString("QLabel{color: rgb(%1, %2, %3);}").arg(QString::number(highlightHiddenColor.red()), QString::number(highlightHiddenColor.green()), QString::number(highlightHiddenColor.blue())));
    }
    else {
        ui->labPicStr->setStyleSheet("");
    }
}

bool SnapmaticWidget::makePictureHidden()
{
    if (smpic->setPictureHidden()) {
        adjustTextColor();
        return true;
    }
    return false;
}

bool SnapmaticWidget::makePictureVisible()
{
    if (smpic->setPictureVisible()) {
        adjustTextColor();
        return true;
    }
    return false;
}

void SnapmaticWidget::makePictureHiddenSlot()
{
    if (!makePictureHidden())
        QMessageBox::warning(this, QApplication::translate("UserInterface", "Hide In-game"), QApplication::translate("SnapmaticWidget", "Failed to hide %1 In-game from your Snapmatic pictures").arg("\""+smpic->getPictureTitle()+"\""));
}

void SnapmaticWidget::makePictureVisibleSlot()
{
    if (!makePictureVisible())
        QMessageBox::warning(this, QApplication::translate("UserInterface", "Show In-game"), QApplication::translate("SnapmaticWidget", "Failed to show %1 In-game from your Snapmatic pictures").arg("\""+smpic->getPictureTitle()+"\""));
}

void SnapmaticWidget::editSnapmaticProperties()
{
    SnapmaticEditor *snapmaticEditor = new SnapmaticEditor(crewDB, profileDB, this);
    snapmaticEditor->setSnapmaticPicture(smpic);
    snapmaticEditor->setModal(true);
    snapmaticEditor->show();
    snapmaticEditor->exec();
    delete snapmaticEditor;
}

void SnapmaticWidget::editSnapmaticRawJson()
{
    JsonEditorDialog *jsonEditor = new JsonEditorDialog(smpic, this);
    jsonEditor->setModal(true);
    jsonEditor->show();
    jsonEditor->exec();
    delete jsonEditor;
}

void SnapmaticWidget::editSnapmaticImage()
{
    QImage *currentImage = new QImage(smpic->getImage());
    ImportDialog *importDialog = new ImportDialog(profileName, this);
    importDialog->setImage(currentImage);
    importDialog->enableOverwriteMode();
    importDialog->setModal(true);
    importDialog->exec();
    if (importDialog->isImportAgreed()) {
        const QByteArray previousPicture = smpic->getPictureStream();
        bool success = smpic->setImage(importDialog->image(), importDialog->isUnlimitedBuffer());
        if (success) {
            QString currentFilePath = smpic->getPictureFilePath();
            QString originalFilePath = smpic->getOriginalPictureFilePath();
            QString backupFileName = originalFilePath % ".bak";
            if (!QFile::exists(backupFileName)) {
                QFile::copy(currentFilePath, backupFileName);
            }
            if (!smpic->exportPicture(currentFilePath)) {
                smpic->setPictureStream(previousPicture);
                QMessageBox::warning(this, QApplication::translate("ImageEditorDialog", "Snapmatic Image Editor"), QApplication::translate("ImageEditorDialog", "Patching of Snapmatic Image failed because of I/O Error"));
                return;
            }
            smpic->emitCustomSignal("PictureUpdated");
#ifdef GTA5SYNC_TELEMETRY
            QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            telemetrySettings.beginGroup("Telemetry");
            bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
            telemetrySettings.endGroup();
            if (pushUsageData && Telemetry->canPush()) {
                QJsonDocument jsonDocument;
                QJsonObject jsonObject;
                jsonObject["Type"] = "ImageEdited";
                jsonObject["ExtraFlags"] = "Interface";
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
        }
        else {
            QMessageBox::warning(this, QApplication::translate("ImageEditorDialog", "Snapmatic Image Editor"), QApplication::translate("ImageEditorDialog", "Patching of Snapmatic Image failed because of Image Error"));
            return;
        }
    }
    delete importDialog;
}

void SnapmaticWidget::openMapViewer()
{
    SnapmaticPicture *picture = smpic;
    SnapmaticProperties currentProperties = picture->getSnapmaticProperties();
    MapLocationDialog *mapLocDialog = new MapLocationDialog(currentProperties.location.x, currentProperties.location.y, this);
    mapLocDialog->setCayoPerico(currentProperties.location.isCayoPerico);
    mapLocDialog->setModal(true);
    mapLocDialog->show();
    mapLocDialog->exec();
    if (mapLocDialog->propUpdated()) {
        // Update Snapmatic Properties
        currentProperties.location.x = mapLocDialog->getXpos();
        currentProperties.location.y = mapLocDialog->getYpos();
        currentProperties.location.z = 0;

        // Update Snapmatic Picture
        QString currentFilePath = picture->getPictureFilePath();
        QString originalFilePath = picture->getOriginalPictureFilePath();
        QString backupFileName = originalFilePath % ".bak";
        if (!QFile::exists(backupFileName)) {
            QFile::copy(currentFilePath, backupFileName);
        }
        SnapmaticProperties fallbackProperties = picture->getSnapmaticProperties();
        picture->setSnapmaticProperties(currentProperties);
        if (!picture->exportPicture(currentFilePath)) {
            QMessageBox::warning(this, SnapmaticEditor::tr("Snapmatic Properties"), SnapmaticEditor::tr("Patching of Snapmatic Properties failed because of I/O Error"));
            picture->setSnapmaticProperties(fallbackProperties);
        }
#ifdef GTA5SYNC_TELEMETRY
        else {
            QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            telemetrySettings.beginGroup("Telemetry");
            bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
            telemetrySettings.endGroup();
            if (pushUsageData && Telemetry->canPush()) {
                QJsonDocument jsonDocument;
                QJsonObject jsonObject;
                jsonObject["Type"] = "LocationEdited";
                jsonObject["ExtraFlags"] = "Interface";
                jsonObject["EditedSize"] = QString::number(picture->getContentMaxLength());
#if QT_VERSION >= 0x060000
                jsonObject["EditedTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                jsonObject["EditedTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                jsonDocument.setObject(jsonObject);
                Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
            }
        }
#endif
    }
    delete mapLocDialog;
}

bool SnapmaticWidget::isSelected()
{
    return ui->cbSelected->isChecked();
}

bool SnapmaticWidget::isHidden()
{
    return smpic->isHidden();
}

void SnapmaticWidget::setSelectionMode(bool selectionMode)
{
    ui->cbSelected->setVisible(selectionMode);
}

void SnapmaticWidget::selectAllWidgets()
{
    emit allWidgetsSelected();
}

void SnapmaticWidget::deselectAllWidgets()
{
    emit allWidgetsDeselected();
}

SnapmaticPicture* SnapmaticWidget::getPicture()
{
    return smpic;
}

QString SnapmaticWidget::getPicturePath()
{
    return smpic->getPictureFilePath();
}

QString SnapmaticWidget::getWidgetType()
{
    return "SnapmaticWidget";
}
