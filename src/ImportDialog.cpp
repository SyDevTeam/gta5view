/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2017-2023 Syping
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

#include "ui_ImportDialog.h"
#include "SnapmaticPicture.h"
#include "SidebarGenerator.h"
#include "StandardPaths.h"
#include "ImportDialog.h"
#include "imagecropper.h"
#include "AppEnv.h"
#include "config.h"
#include <QStringBuilder>
#include <QInputDialog>
#include <QImageReader>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QDebug>
#include <QStyle>
#include <QFile>
#include <QRgb>

// IMAGES VALUES
#define snapmaticAvatarResolution 470
#define snapmaticAvatarPlacementW 145
#define snapmaticAvatarPlacementH 66

ImportDialog::ImportDialog(QString profileName, QWidget *parent) :
    QDialog(parent), profileName(profileName),
    ui(new Ui::ImportDialog)
{
    // Set Window Flags
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    ui->setupUi(this);
    ui->cmdOK->setDefault(true);
    ui->cmdOK->setFocus();
    importAgreed = false;
    settingsLocked = false;
    watermarkAvatar = true;
    watermarkPicture = false;
    insideAvatarZone = false;
    avatarAreaImage = QImage(AppEnv::getImagesFolder() % "/avatarareaimport.png");
    selectedColour = QColor::fromRgb(0, 0, 0, 255);

    // Set Icon for OK Button
    if (QIcon::hasThemeIcon("dialog-ok")) {
        ui->cmdOK->setIcon(QIcon::fromTheme("dialog-ok"));
    }
    else if (QIcon::hasThemeIcon("gtk-ok")) {
        ui->cmdOK->setIcon(QIcon::fromTheme("gtk-ok"));
    }

    // Set Icon for Cancel Button
    if (QIcon::hasThemeIcon("dialog-cancel")) {
        ui->cmdCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
    }
    else if (QIcon::hasThemeIcon("gtk-cancel")) {
        ui->cmdCancel->setIcon(QIcon::fromTheme("gtk-cancel"));
    }

    ui->cbIgnore->setChecked(false);
    ui->labColour->setText(tr("Background Colour: <span style=\"color: %1\">%1</span>").arg(selectedColour.name()));
    ui->labBackgroundImage->setText(tr("Background Image:"));
    ui->cmdBackgroundWipe->setVisible(false);

    // Snapmatic Resolution
    ui->cbResolution->addItem("GTA V (536p)", QSize(960, 536));
    ui->cbResolution->addItem("FiveM (1072p)", QSize(1920, 1072));
    ui->cbResolution->addItem("RDR 2 (1080p)", QSize(1920, 1080));
    ui->cbResolution->addItem("Preset (720p)", QSize(1280, 720));
    ui->cbResolution->addItem("Preset (1440p)", QSize(2560, 1440));
    ui->cbResolution->addItem("Preset (2160p)", QSize(3840, 2160));

    // Set Import Settings
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Import");
    QString currentProfile = settings.value("Profile", "Default").toString();
    settings.endGroup();
    processSettings(currentProfile);

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    snapmaticResolutionLW = 516 * screenRatio; // 430
    snapmaticResolutionLH = 288 * screenRatio; // 240
    ui->labPicture->setMinimumSize(snapmaticResolutionLW, snapmaticResolutionLH);

    ui->vlButtom->setSpacing(6 * screenRatio);
#ifndef Q_OS_MAC
    ui->vlButtom->setContentsMargins(9 * screenRatio, 6 * screenRatio, 9 * screenRatio, 9 * screenRatio);
#else
#if QT_VERSION >= 0x060000
    if (QApplication::style()->objectName() == "macos") {
#else
    if (QApplication::style()->objectName() == "macintosh") {
#endif
        ui->vlButtom->setContentsMargins(9 * screenRatio, 9 * screenRatio, 9 * screenRatio, 9 * screenRatio);
    }
    else {
        ui->vlButtom->setContentsMargins(9 * screenRatio, 6 * screenRatio, 9 * screenRatio, 9 * screenRatio);
    }
#endif

    // Options menu
    optionsMenu.addAction(tr("&Import new Picture..."), this, SLOT(importNewPicture()));
    optionsMenu.addAction(tr("&Crop Picture..."), this, SLOT(cropPicture()));
    optionsMenu.addSeparator();
    optionsMenu.addAction(tr("&Load Settings..."), this, SLOT(loadImportSettings()));
    optionsMenu.addAction(tr("&Save Settings..."), this, SLOT(saveImportSettings()));
    ui->cmdOptions->setMenu(&optionsMenu);

    const QSize windowSize = sizeHint();
    setMinimumSize(windowSize);
    setMaximumSize(windowSize);
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

void ImportDialog::processImage()
{
    if (workImage.isNull())
        return;

    QImage snapmaticImage = workImage;
    QPixmap snapmaticPixmap(snapmaticResolution);
    snapmaticPixmap.fill(selectedColour);
    QPainter snapmaticPainter(&snapmaticPixmap);
    qreal screenRatioPR = AppEnv::screenRatioPR();
    if (!backImage.isNull()) {
        if (!ui->cbStretch->isChecked()) {
            int diffWidth = 0;
            int diffHeight = 0;
            if (backImage.width() != snapmaticResolution.width()) {
                diffWidth = snapmaticResolution.width() - backImage.width();
                diffWidth = diffWidth / 2;
            }
            else if (backImage.height() != snapmaticResolution.height()) {
                diffHeight = snapmaticResolution.height() - backImage.height();
                diffHeight = diffHeight / 2;
            }
            snapmaticPainter.drawImage(0 + diffWidth, 0 + diffHeight, backImage);
        }
        else {
            snapmaticPainter.drawImage(0, 0, QImage(backImage).scaled(snapmaticResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
        if (ui->cbAvatar->isChecked() && ui->cbForceAvatarColour->isChecked()) {
            snapmaticPainter.fillRect(snapmaticAvatarPlacementW, snapmaticAvatarPlacementH, snapmaticAvatarResolution, snapmaticAvatarResolution, selectedColour);
        }
    }
    if (insideAvatarZone) {
        // Avatar mode
        int diffWidth = 0;
        int diffHeight = 0;
        if (ui->cbIgnore->isChecked()) {
            snapmaticImage = snapmaticImage.scaled(snapmaticAvatarResolution, snapmaticAvatarResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        else if (ui->cbBorderless->isChecked()) {
            snapmaticImage = snapmaticImage.scaled(snapmaticAvatarResolution, snapmaticAvatarResolution, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            if (snapmaticImage.width() > snapmaticAvatarResolution) {
                int diffWidth = snapmaticImage.width() - snapmaticAvatarResolution;
                diffWidth = diffWidth / 2;
                QImage croppedImage(snapmaticAvatarResolution, snapmaticAvatarResolution, QImage::Format_ARGB32);
                croppedImage.fill(Qt::transparent);
                QPainter croppedPainter(&croppedImage);
                croppedPainter.drawImage(0 - diffWidth, 0, snapmaticImage);
                croppedPainter.end();
                snapmaticImage = croppedImage;
            }
            else if (snapmaticImage.height() > snapmaticAvatarResolution) {
                int diffHeight = snapmaticImage.height() - snapmaticAvatarResolution;
                diffHeight = diffHeight / 2;
                QImage croppedImage(snapmaticAvatarResolution, snapmaticAvatarResolution, QImage::Format_ARGB32);
                croppedImage.fill(Qt::transparent);
                QPainter croppedPainter(&croppedImage);
                croppedPainter.drawImage(0, 0 - diffHeight, snapmaticImage);
                croppedPainter.end();
                snapmaticImage = croppedImage;
            }
        }
        else {
            snapmaticImage = snapmaticImage.scaled(snapmaticAvatarResolution, snapmaticAvatarResolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (snapmaticImage.width() > snapmaticImage.height()) {
                diffHeight = snapmaticAvatarResolution - snapmaticImage.height();
                diffHeight = diffHeight / 2;
            }
            else if (snapmaticImage.width() < snapmaticImage.height()) {
                diffWidth = snapmaticAvatarResolution - snapmaticImage.width();
                diffWidth = diffWidth / 2;
            }
        }
        snapmaticPainter.drawImage(snapmaticAvatarPlacementW + diffWidth, snapmaticAvatarPlacementH + diffHeight, snapmaticImage);
        if (ui->cbWatermark->isChecked())
            processWatermark(&snapmaticPainter);
        imageTitle = tr("Custom Avatar", "Custom Avatar Description in SC, don't use Special Character!");
    }
    else {
        // Picture mode
        int diffWidth = 0;
        int diffHeight = 0;
        if (ui->cbIgnore->isChecked()) {
            snapmaticImage = snapmaticImage.scaled(snapmaticResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        else if (ui->cbBorderless->isChecked()) {
            snapmaticImage = snapmaticImage.scaled(snapmaticResolution, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            if (snapmaticImage.width() > snapmaticResolution.width()) {
                int diffWidth = snapmaticImage.width() - snapmaticResolution.width();
                diffWidth = diffWidth / 2;
                QImage croppedImage(snapmaticResolution, QImage::Format_ARGB32);
                croppedImage.fill(Qt::transparent);
                QPainter croppedPainter(&croppedImage);
                croppedPainter.drawImage(0 - diffWidth, 0, snapmaticImage);
                croppedPainter.end();
                snapmaticImage = croppedImage;
            }
            else if (snapmaticImage.height() > snapmaticResolution.height()) {
                int diffHeight = snapmaticImage.height() - snapmaticResolution.height();
                diffHeight = diffHeight / 2;
                QImage croppedImage(snapmaticResolution, QImage::Format_ARGB32);
                croppedImage.fill(Qt::transparent);
                QPainter croppedPainter(&croppedImage);
                croppedPainter.drawImage(0, 0 - diffHeight, snapmaticImage);
                croppedPainter.end();
                snapmaticImage = croppedImage;
            }
        }
        else {
            snapmaticImage = snapmaticImage.scaled(snapmaticResolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (snapmaticImage.width() != snapmaticResolution.width()) {
                diffWidth = snapmaticResolution.width() - snapmaticImage.width();
                diffWidth = diffWidth / 2;
            }
            else if (snapmaticImage.height() != snapmaticResolution.height()) {
                diffHeight = snapmaticResolution.height() - snapmaticImage.height();
                diffHeight = diffHeight / 2;
            }
        }
        snapmaticPainter.drawImage(0 + diffWidth, 0 + diffHeight, snapmaticImage);
        if (ui->cbWatermark->isChecked())
            processWatermark(&snapmaticPainter);
        imageTitle = tr("Custom Picture", "Custom Picture Description in SC, don't use Special Character!");
    }
    snapmaticPainter.end();
    newImage = snapmaticPixmap.toImage();
#if QT_VERSION >= 0x050600
    snapmaticPixmap.setDevicePixelRatio(screenRatioPR);
#endif
    ui->labPicture->setPixmap(snapmaticPixmap.scaled(snapmaticResolutionLW * screenRatioPR, snapmaticResolutionLH * screenRatioPR, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void ImportDialog::reworkImage()
{
    workImage = QImage();
    if (origImage.width() == origImage.height()) {
        if (ui->cbResolution->currentIndex() == 0) {
            insideAvatarZone = true;
            ui->cbAvatar->setChecked(true);
        }
        else {
            insideAvatarZone = false;
            ui->cbAvatar->setChecked(false);
        }
        if (origImage.height() > snapmaticResolution.height()) {
            workImage = origImage.scaled(snapmaticResolution.height(), snapmaticResolution.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        else {
            workImage = origImage;
        }
    }
    else if (origImage.width() > snapmaticResolution.width() && origImage.width() > origImage.height()) {
        insideAvatarZone = false;
        ui->cbAvatar->setChecked(false);
        workImage = origImage.scaledToWidth(snapmaticResolution.width(), Qt::SmoothTransformation);
    }
    else if (origImage.height() > snapmaticResolution.height() && origImage.height() > origImage.width()) {
        insideAvatarZone = false;
        ui->cbAvatar->setChecked(false);
        workImage = origImage.scaledToHeight(snapmaticResolution.height(), Qt::SmoothTransformation);
    }
    else {
        insideAvatarZone = false;
        ui->cbAvatar->setChecked(false);
        workImage = origImage;
    }
    processImage();
}

void ImportDialog::processWatermark(QPainter *snapmaticPainter)
{
    bool blackWatermark = false;
    bool redWatermark = false;
    if (selectedColour.red() > 127) {
        if (selectedColour.green() > 127 || selectedColour.blue() > 127) {
            redWatermark = true;
        }
    }
    else {
        redWatermark = true;
    }
    if (selectedColour.lightness() > 127) {
        blackWatermark = true;
    }
    // draw watermark
    if (redWatermark) {
        const QImage viewWatermark = QImage(AppEnv::getImagesFolder() % "/watermark_2r.png");
        snapmaticPainter->drawImage(snapmaticResolution.width() - viewWatermark.width(), 0, viewWatermark);
    }
    else
    {
        QImage viewWatermark = QImage(AppEnv::getImagesFolder() % "/watermark_2b.png");
        if (!blackWatermark) {
            viewWatermark.invertPixels(QImage::InvertRgb);
        }
        snapmaticPainter->drawImage(snapmaticResolution.width() - viewWatermark.width(), 0, viewWatermark);
    }
    QImage textWatermark = QImage(AppEnv::getImagesFolder() % "/watermark_1b.png");
    if (!blackWatermark) {
        textWatermark.invertPixels(QImage::InvertRgb);
    }
    snapmaticPainter->drawImage(snapmaticResolution.width() - textWatermark.width(), 0, textWatermark);
}

void ImportDialog::processSettings(QString settingsProfile, bool setDefault)
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Import");
    if (setDefault) {
        settings.setValue("Profile", settingsProfile);
    }
    if (settingsProfile == "Default") {
        watermarkAvatar = true;
        watermarkPicture = false;
        selectedColour = QColor::fromRgb(0, 0, 0, 255);
        backImage = QImage();
        ui->cbBorderless->setChecked(false);
        ui->cbStretch->setChecked(false);
        ui->cbForceAvatarColour->setChecked(false);
        ui->cbUnlimited->setChecked(false);
        ui->cbImportAsIs->setChecked(false);
        ui->cbResolution->setCurrentIndex(0);
    }
    else {
        settings.beginGroup(settingsProfile);
        watermarkAvatar = settings.value("WatermarkAvatar", true).toBool();
        watermarkPicture = settings.value("WatermarkPicture", false).toBool();
        backImage = qvariant_cast<QImage>(settings.value("BackgroundImage", QImage()));
        selectedColour = qvariant_cast<QColor>(settings.value("SelectedColour", QColor::fromRgb(0, 0, 0, 255)));
        ui->cbBorderless->setChecked(settings.value("BorderlessImage", false).toBool());
        ui->cbStretch->setChecked(settings.value("BackgroundStretch", false).toBool());
        ui->cbForceAvatarColour->setChecked(settings.value("ForceAvatarColour", false).toBool());
        ui->cbUnlimited->setChecked(settings.value("UnlimitedBuffer", false).toBool());
        ui->cbImportAsIs->setChecked(settings.value("ImportAsIs", false).toBool());
        const QVariant data = settings.value("Resolution", QSize(960, 536));
#if QT_VERSION >= 0x060000
        if (data.typeId() == QMetaType::QSize)
#else
        if (data.type() == QVariant::Size)
#endif
        {
            int index = ui->cbResolution->findData(data);
            if (index != -1) {
                ui->cbResolution->setCurrentIndex(index);
            }
        }
        settings.endGroup();
    }
    if (!workImage.isNull()) {
        if (ui->cbAvatar->isChecked()) {
            ui->cbWatermark->setChecked(watermarkAvatar);
        }
        else {
            ui->cbWatermark->setChecked(watermarkPicture);
        }
    }
    ui->labColour->setText(tr("Background Colour: <span style=\"color: %1\">%1</span>").arg(selectedColour.name()));
    if (!backImage.isNull()) {
        ui->labBackgroundImage->setText(tr("Background Image: %1").arg(tr("Storage", "Background Image: Storage")));
        ui->cmdBackgroundWipe->setVisible(true);
    }
    else {
        ui->labBackgroundImage->setText(tr("Background Image:"));
        ui->cmdBackgroundWipe->setVisible(false);
    }
    settings.endGroup();
}

void ImportDialog::saveSettings(QString settingsProfile)
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Import");
    settings.beginGroup(settingsProfile);
    settings.setValue("WatermarkAvatar", watermarkAvatar);
    settings.setValue("WatermarkPicture", watermarkPicture);
    settings.setValue("BackgroundImage", backImage);
    settings.setValue("SelectedColour", selectedColour);
    settings.setValue("BorderlessImage", ui->cbBorderless->isChecked());
    settings.setValue("BackgroundStretch", ui->cbStretch->isChecked());
    settings.setValue("ForceAvatarColour", ui->cbForceAvatarColour->isChecked());
#if QT_VERSION >= 0x050000
    const QVariant data = ui->cbResolution->currentData();
#else
    const QVariant data = ui->cbResolution->itemData(ui->cbResolution->currentIndex());
#endif
#if QT_VERSION >= 0x060000
    if (data.typeId() == QMetaType::QSize)
#else
    if (data.type() == QVariant::Size)
#endif
    {
        settings.setValue("Resolution", data);
    }
    else {
        settings.setValue("Resolution", QSize(960, 536));
    }
    settings.setValue("UnlimitedBuffer", ui->cbUnlimited->isChecked());
    settings.setValue("ImportAsIs", ui->cbImportAsIs->isChecked());
    settings.endGroup();
    settings.setValue("Profile", settingsProfile);
    settings.endGroup();
}

void ImportDialog::cropPicture()
{
    qreal screenRatio = AppEnv::screenRatio();

    QDialog cropDialog(this);
#if QT_VERSION >= 0x050000
    cropDialog.setObjectName(QStringLiteral("CropDialog"));
#else
    cropDialog.setObjectName(QString::fromUtf8("CropDialog"));
#endif
    cropDialog.setWindowTitle(tr("Crop Picture..."));
    cropDialog.setWindowFlags(cropDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    cropDialog.setModal(true);

    QVBoxLayout cropLayout;
#if QT_VERSION >= 0x050000
    cropLayout.setObjectName(QStringLiteral("CropLayout"));
#else
    cropLayout.setObjectName(QString::fromUtf8("CropLayout"));
#endif
    cropLayout.setContentsMargins(0, 0, 0, 0);
    cropLayout.setSpacing(0);
    cropDialog.setLayout(&cropLayout);

    ImageCropper imageCropper(&cropDialog);
#if QT_VERSION >= 0x050000
    imageCropper.setObjectName(QStringLiteral("ImageCropper"));
#else
    imageCropper.setObjectName(QString::fromUtf8("ImageCropper"));
#endif
    imageCropper.setBackgroundColor(Qt::black);
    imageCropper.setCroppingRectBorderColor(QColor(255, 255, 255, 127));
    imageCropper.setImage(QPixmap::fromImage(origImage, Qt::AutoColor));
    imageCropper.setProportion(QSize(1, 1));
    imageCropper.setFixedSize(workImage.size());
    cropLayout.addWidget(&imageCropper);

    QHBoxLayout buttonLayout;
#if QT_VERSION >= 0x050000
    cropLayout.setObjectName(QStringLiteral("ButtonLayout"));
#else
    cropLayout.setObjectName(QString::fromUtf8("ButtonLayout"));
#endif
    cropLayout.addLayout(&buttonLayout);

    QPushButton cropButton(&cropDialog);
#if QT_VERSION >= 0x050000
    cropButton.setObjectName(QStringLiteral("CropButton"));
#else
    cropButton.setObjectName(QString::fromUtf8("CropButton"));
#endif
    cropButton.setMinimumSize(0, 40 * screenRatio);
    cropButton.setText(tr("&Crop"));
    cropButton.setToolTip(tr("Crop Picture"));
    QObject::connect(&cropButton, SIGNAL(clicked(bool)), &cropDialog, SLOT(accept()));

    buttonLayout.addWidget(&cropButton);

    cropDialog.show();
    cropDialog.setFixedSize(cropDialog.sizeHint());
    if (cropDialog.exec() == QDialog::Accepted) {
        QImage *croppedImage = new QImage(imageCropper.cropImage().toImage());
        setImage(croppedImage);
    }
}

void ImportDialog::importNewPicture()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("FileDialogs");
    bool dontUseNativeDialog = settings.value("DontUseNativeDialog", false).toBool();
    settings.beginGroup("ImportCopy");

fileDialogPreOpen: //Work?
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, dontUseNativeDialog);
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(QApplication::translate("ProfileInterface", "Import..."));
    fileDialog.setLabelText(QFileDialog::Accept, QApplication::translate("ProfileInterface", "Import"));

    // Getting readable Image formats
    QString imageFormatsStr = " ";
    for (const QByteArray &imageFormat : QImageReader::supportedImageFormats()) {
        imageFormatsStr += QString("*.") % QString::fromUtf8(imageFormat).toLower() % " ";
    }

    QStringList filters;
    filters << QApplication::translate("ProfileInterface", "All image files (%1)").arg(imageFormatsStr.trimmed());
    filters << QApplication::translate("ProfileInterface", "All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.setDirectory(settings.value(profileName % "+Directory", StandardPaths::documentsLocation()).toString());
    fileDialog.restoreGeometry(settings.value(profileName % "+Geometry", "").toByteArray());

    if (fileDialog.exec()) {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1) {
            QString selectedFile = selectedFiles.at(0);
            QString selectedFileName = QFileInfo(selectedFile).fileName();

            QFile snapmaticFile(selectedFile);
            if (!snapmaticFile.open(QFile::ReadOnly)) {
                QMessageBox::warning(this, QApplication::translate("ProfileInterface", "Import"), QApplication::translate("ProfileInterface", "Can't import %1 because file can't be open").arg("\""+selectedFileName+"\""));
                goto fileDialogPreOpen;
            }
            QImage *importImage = new QImage();
            QImageReader snapmaticImageReader;
            snapmaticImageReader.setDecideFormatFromContent(true);
            snapmaticImageReader.setDevice(&snapmaticFile);
            if (!snapmaticImageReader.read(importImage)) {
                QMessageBox::warning(this, QApplication::translate("ProfileInterface", "Import"), QApplication::translate("ProfileInterface", "Can't import %1 because file can't be parsed properly").arg("\""+selectedFileName+"\""));
                delete importImage;
                goto fileDialogPreOpen;
            }
            setImage(importImage);
        }
    }

    settings.setValue(profileName % "+Geometry", fileDialog.saveGeometry());
    settings.setValue(profileName % "+Directory", fileDialog.directory().absolutePath());
    settings.endGroup();
    settings.endGroup();
}

void ImportDialog::loadImportSettings()
{
    if (settingsLocked) {
        QMessageBox::information(this, tr("Load Settings..."), tr("Please import a new picture first"));
        return;
    }
    bool ok;
    QStringList profileList;
    profileList << tr("Default", "Default as Default Profile")
                << tr("Profile %1", "Profile %1 as Profile 1").arg("1")
                << tr("Profile %1", "Profile %1 as Profile 1").arg("2")
                << tr("Profile %1", "Profile %1 as Profile 1").arg("3")
                << tr("Profile %1", "Profile %1 as Profile 1").arg("4")
                << tr("Profile %1", "Profile %1 as Profile 1").arg("5");
    QString sProfile = QInputDialog::getItem(this, tr("Load Settings..."), tr("Please select your settings profile"), profileList, 0, false, &ok, windowFlags());
    if (ok) {
        QString pProfile;
        if (sProfile == tr("Default", "Default as Default Profile")) {
            pProfile = "Default";
        }
        else if (sProfile == tr("Profile %1", "Profile %1 as Profile 1").arg("1")) {
            pProfile = "Profile 1";
        }
        else if (sProfile == tr("Profile %1", "Profile %1 as Profile 1").arg("2")) {
            pProfile = "Profile 2";
        }
        else if (sProfile == tr("Profile %1", "Profile %1 as Profile 1").arg("3")) {
            pProfile = "Profile 3";
        }
        else if (sProfile == tr("Profile %1", "Profile %1 as Profile 1").arg("4"))
        {
            pProfile = "Profile 4";
        }
        else if (sProfile == tr("Profile %1", "Profile %1 as Profile 1").arg("5")) {
            pProfile = "Profile 5";
        }
        processSettings(pProfile, true);
        processImage();
    }
}

void ImportDialog::saveImportSettings()
{
    if (settingsLocked) {
        QMessageBox::information(this, tr("Save Settings..."), tr("Please import a new picture first"));
        return;
    }
    bool ok;
    QStringList profileList;
    profileList << tr("Profile %1", "Profile %1 as Profile 1").arg("1")
                << tr("Profile %1", "Profile %1 as Profile 1").arg("2")
                << tr("Profile %1", "Profile %1 as Profile 1").arg("3")
                << tr("Profile %1", "Profile %1 as Profile 1").arg("4")
                << tr("Profile %1", "Profile %1 as Profile 1").arg("5");
    QString sProfile = QInputDialog::getItem(this, tr("Save Settings..."), tr("Please select your settings profile"), profileList, 0, false, &ok, windowFlags());
    if (ok) {
        QString pProfile;
        if (sProfile == tr("Profile %1", "Profile %1 as Profile 1").arg("1")) {
            pProfile = "Profile 1";
        }
        else if (sProfile == tr("Profile %1", "Profile %1 as Profile 1").arg("2")) {
            pProfile = "Profile 2";
        }
        else if (sProfile == tr("Profile %1", "Profile %1 as Profile 1").arg("3")) {
            pProfile = "Profile 3";
        }
        else if (sProfile == tr("Profile %1", "Profile %1 as Profile 1").arg("4")) {
            pProfile = "Profile 4";
        }
        else if (sProfile == tr("Profile %1", "Profile %1 as Profile 1").arg("5")) {
            pProfile = "Profile 5";
        }
        saveSettings(pProfile);
    }
}

QImage ImportDialog::image()
{
    if (ui->cbImportAsIs->isChecked()) {
        return origImage;
    }
    else {
        return newImage;
    }
}

void ImportDialog::setImage(QImage *image_)
{
    origImage = *image_;
    workImage = QImage();
    if (image_->width() == image_->height()) {
        if (ui->cbResolution->currentIndex() == 0) {
            insideAvatarZone = true;
            ui->cbAvatar->setChecked(true);
        }
        else {
            insideAvatarZone = false;
            ui->cbAvatar->setChecked(false);
        }
        if (image_->height() > snapmaticResolution.height()) {
            workImage = image_->scaled(snapmaticResolution.height(), snapmaticResolution.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            delete image_;
        }
        else {
            workImage = *image_;
            delete image_;
        }
    }
    else if (image_->width() > snapmaticResolution.width() && image_->width() > image_->height()) {
        insideAvatarZone = false;
        ui->cbAvatar->setChecked(false);
        workImage = image_->scaledToWidth(snapmaticResolution.width(), Qt::SmoothTransformation);
        delete image_;
    }
    else if (image_->height() > snapmaticResolution.height() && image_->height() > image_->width()) {
        insideAvatarZone = false;
        ui->cbAvatar->setChecked(false);
        workImage = image_->scaledToHeight(snapmaticResolution.height(), Qt::SmoothTransformation);
        delete image_;
    }
    else {
        insideAvatarZone = false;
        ui->cbAvatar->setChecked(false);
        workImage = *image_;
        delete image_;
    }
    processImage();
    lockSettings(false);
}

void ImportDialog::lockSettings(bool lock)
{
    ui->gbAdvanced->setDisabled(lock);
    if (ui->cbImportAsIs->isChecked()) {
        ui->gbBackground->setDisabled(true);
        ui->gbSettings->setDisabled(true);
    }
    else {
        ui->gbBackground->setDisabled(lock);
        ui->gbSettings->setDisabled(lock);
    }
    ui->cmdOK->setDisabled(lock);
    settingsLocked = lock;
}

void ImportDialog::enableOverwriteMode()
{
    setWindowTitle(QApplication::translate("ImageEditorDialog", "Overwrite Image..."));
    ui->cmdOK->setText(QApplication::translate("ImageEditorDialog", "&Overwrite"));
    ui->cmdOK->setToolTip(QApplication::translate("ImageEditorDialog", "Apply changes"));
    ui->cmdCancel->setText(QApplication::translate("ImageEditorDialog", "&Close"));
    ui->cmdCancel->setToolTip(QApplication::translate("ImageEditorDialog", "Discard changes"));
    ui->cmdCancel->setDefault(true);
    ui->cmdCancel->setFocus();
    lockSettings(true);
}

bool ImportDialog::isImportAgreed()
{
    return importAgreed;
}

bool ImportDialog::isUnlimitedBuffer()
{
    return ui->cbUnlimited->isChecked();
}

bool ImportDialog::areSettingsLocked()
{
    return settingsLocked;
}

QString ImportDialog::getImageTitle()
{
    if (ui->cbImportAsIs->isChecked()) {
        return tr("Custom Picture", "Custom Picture Description in SC, don't use Special Character!");
    }
    else {
        return imageTitle;
    }
}

void ImportDialog::on_cbIgnore_toggled(bool checked)
{
    ui->cbBorderless->setDisabled(checked);
    processImage();
}

void ImportDialog::on_cbAvatar_toggled(bool checked)
{
    if (ui->cbResolution->currentIndex() != 0)
        return;

    if (!workImage.isNull() && workImage.width() == workImage.height() && !checked) {
        if (QMessageBox::No == QMessageBox::warning(this, tr("Snapmatic Avatar Zone"), tr("Are you sure to use a square image outside of the Avatar Zone?\nWhen you want to use it as Avatar the image will be detached!"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
            ui->cbAvatar->setChecked(true);
            insideAvatarZone = true;
            return;
        }
    }
    insideAvatarZone = ui->cbAvatar->isChecked();
    watermarkBlock = true;
    if (insideAvatarZone) {
        ui->cbWatermark->setChecked(watermarkAvatar);
    }
    else {
        ui->cbWatermark->setChecked(watermarkPicture);
    }
    watermarkBlock = false;
    processImage();
}

void ImportDialog::on_cmdCancel_clicked()
{
    close();
}

void ImportDialog::on_cmdOK_clicked()
{
    importAgreed = true;
    close();
}

void ImportDialog::on_labPicture_labelPainted()
{
    if (insideAvatarZone) {
        QImage avatarAreaFinalImage(avatarAreaImage);
        if (selectedColour.lightness() > 127) {
            avatarAreaFinalImage.setColor(1, qRgb(0, 0, 0));
        }
        QPainter labelPainter(ui->labPicture);
        labelPainter.drawImage(0, 0, avatarAreaFinalImage.scaled(snapmaticResolutionLW, snapmaticResolutionLH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        labelPainter.end();
    }
}

void ImportDialog::on_cmdColourChange_clicked()
{
    QColor newSelectedColour = QColorDialog::getColor(selectedColour, this, tr("Select Colour..."));
    if (newSelectedColour.isValid()) {
        selectedColour = newSelectedColour;
        ui->labColour->setText(tr("Background Colour: <span style=\"color: %1\">%1</span>").arg(selectedColour.name()));
        processImage();
    }
}

void ImportDialog::on_cmdBackgroundChange_clicked()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("FileDialogs");
    bool dontUseNativeDialog = settings.value("DontUseNativeDialog", false).toBool();
    settings.beginGroup("ImportBackground");

fileDialogPreOpen:
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, dontUseNativeDialog);
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(QApplication::translate("ProfileInterface", "Import..."));
    fileDialog.setLabelText(QFileDialog::Accept, QApplication::translate("ProfileInterface", "Import"));

    // Getting readable Image formats
    QString imageFormatsStr = " ";
    for (const QByteArray &imageFormat : QImageReader::supportedImageFormats()) {
        imageFormatsStr += QString("*.") % QString::fromUtf8(imageFormat).toLower() % " ";
    }

    QStringList filters;
    filters << QApplication::translate("ProfileInterface", "All image files (%1)").arg(imageFormatsStr.trimmed());
    filters << QApplication::translate("ProfileInterface", "All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.setDirectory(settings.value("Directory", StandardPaths::documentsLocation()).toString());
    fileDialog.restoreGeometry(settings.value("Geometry", "").toByteArray());

    if (fileDialog.exec()) {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1) {
            QString selectedFile = selectedFiles.at(0);
            QString selectedFileName = QFileInfo(selectedFile).fileName();

            QFile snapmaticFile(selectedFile);
            if (!snapmaticFile.open(QFile::ReadOnly)) {
                QMessageBox::warning(this, QApplication::translate("ProfileInterface", "Import"), QApplication::translate("ProfileInterface", "Can't import %1 because file can't be open").arg("\""+selectedFileName+"\""));
                goto fileDialogPreOpen;
            }
            QImage importImage;
            QImageReader snapmaticImageReader;
            snapmaticImageReader.setDecideFormatFromContent(true);
            snapmaticImageReader.setDevice(&snapmaticFile);
            if (!snapmaticImageReader.read(&importImage)) {
                QMessageBox::warning(this, QApplication::translate("ProfileInterface", "Import"), QApplication::translate("ProfileInterface", "Can't import %1 because file can't be parsed properly").arg("\""+selectedFileName+"\""));
                goto fileDialogPreOpen;
            }
            backImage = importImage.scaled(snapmaticResolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            backgroundPath = selectedFile;
            ui->labBackgroundImage->setText(tr("Background Image: %1").arg(tr("File", "Background Image: File")));
            ui->cmdBackgroundWipe->setVisible(true);
            processImage();
        }
    }

    settings.setValue("Geometry", fileDialog.saveGeometry());
    settings.setValue("Directory", fileDialog.directory().absolutePath());
    settings.endGroup();
    settings.endGroup();
}

void ImportDialog::on_cmdBackgroundWipe_clicked()
{
    backImage = QImage();
    ui->labBackgroundImage->setText(tr("Background Image:"));
    ui->cmdBackgroundWipe->setVisible(false);
    processImage();
}

void ImportDialog::on_cbStretch_toggled(bool checked)
{
    Q_UNUSED(checked)
    processImage();
}

void ImportDialog::on_cbForceAvatarColour_toggled(bool checked)
{
    Q_UNUSED(checked)
    processImage();
}

void ImportDialog::on_cbWatermark_toggled(bool checked)
{
    if (!watermarkBlock) {
        if (insideAvatarZone) {
            watermarkAvatar = checked;
        }
        else {
            watermarkPicture = checked;
        }
        processImage();
    }
}

void ImportDialog::on_cbBorderless_toggled(bool checked)
{
    ui->cbIgnore->setDisabled(checked);
    processImage();
}

void ImportDialog::on_cbImportAsIs_toggled(bool checked)
{
    ui->cbResolution->setDisabled(checked);
    ui->labResolution->setDisabled(checked);
    ui->gbBackground->setDisabled(checked);
    ui->gbSettings->setDisabled(checked);
}

void ImportDialog::on_cbResolution_currentIndexChanged(int index)
{
    Q_UNUSED(index)
#if QT_VERSION >= 0x050000
    const QVariant data = ui->cbResolution->currentData();
#else
    const QVariant data = ui->cbResolution->itemData(ui->cbResolution->currentIndex());
#endif
#if QT_VERSION >= 0x060000
    if (data.typeId() == QMetaType::QSize)
#else
    if (data.type() == QVariant::Size)
#endif
    {
        const QSize dataSize = data.toSize();
        if (dataSize == QSize(960, 536)) {
            ui->cbAvatar->setEnabled(true);
            snapmaticResolution = dataSize;
            reworkImage();
        }
        else {
            if (!workImage.isNull() && workImage.width() == workImage.height() && ui->cbAvatar->isChecked()) {
                if (QMessageBox::No == QMessageBox::warning(this, tr("Snapmatic Avatar Zone"), tr("Are you sure to use a square image outside of the Avatar Zone?\nWhen you want to use it as Avatar the image will be detached!"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
                    ui->cbResolution->setCurrentIndex(0);
                    ui->cbAvatar->setChecked(true);
                    insideAvatarZone = true;
                    return;
                }
            }
            ui->cbAvatar->setChecked(false);
            ui->cbAvatar->setDisabled(true);
            insideAvatarZone = false;
            ui->cbWatermark->setChecked(watermarkPicture);
            snapmaticResolution = dataSize;
            reworkImage();
        }
    }
}
