/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2017-2018 Syping
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

#include "ImportDialog.h"
#include "ui_ImportDialog.h"
#include "SidebarGenerator.h"
#include "StandardPaths.h"
#include "AppEnv.h"
#include "config.h"
#include <QStringBuilder>
#include <QImageReader>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QDebug>
#include <QFile>
#include <QRgb>

// IMAGES VALUES
#define snapmaticResolutionW 960
#define snapmaticResolutionH 536
#define snapmaticAvatarResolution 470
#define snapmaticAvatarPlacementW 145
#define snapmaticAvatarPlacementH 66

ImportDialog::ImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    // Set Window Flags
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);
    importAgreed = false;
    insideAvatarZone = false;
    avatarAreaImage = QImage(":/img/avatarareaimport.png");
    selectedColour = QColor::fromRgb(0, 0, 0, 255);

    // Set Icon for OK Button
    if (QIcon::hasThemeIcon("dialog-ok"))
    {
        ui->cmdOK->setIcon(QIcon::fromTheme("dialog-ok"));
    }
    else if (QIcon::hasThemeIcon("gtk-ok"))
    {
        ui->cmdOK->setIcon(QIcon::fromTheme("gtk-ok"));
    }

    // Set Icon for Cancel Button
    if (QIcon::hasThemeIcon("dialog-cancel"))
    {
        ui->cmdCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
    }
    else if (QIcon::hasThemeIcon("gtk-cancel"))
    {
        ui->cmdCancel->setIcon(QIcon::fromTheme("gtk-cancel"));
    }

    ui->cbIgnore->setChecked(false);
    ui->labColour->setText(tr("Background Colour: <span style=\"color: %1\">%1</span>").arg(selectedColour.name()));
    ui->labBackgroundImage->setText(tr("Background Image:"));
    ui->cmdBackgroundWipe->setVisible(false);

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    snapmaticResolutionLW = 516 * screenRatio; // 430
    snapmaticResolutionLH = 288 * screenRatio; // 240
    ui->labPicture->setMinimumSize(snapmaticResolutionLW, snapmaticResolutionLH);

    ui->vlButtom->setSpacing(6 * screenRatio);
#ifndef Q_OS_MAC
    ui->vlButtom->setContentsMargins(9 * screenRatio, 6 * screenRatio, 9 * screenRatio, 9 * screenRatio);
#else
    if (QApplication::style()->objectName() == "macintosh")
    {
        ui->vlButtom->setContentsMargins(9 * screenRatio, 9 * screenRatio, 9 * screenRatio, 9 * screenRatio);
    }
    else
    {
        ui->vlButtom->setContentsMargins(9 * screenRatio, 6 * screenRatio, 9 * screenRatio, 9 * screenRatio);
    }
#endif

    setMaximumSize(sizeHint());
    setMinimumSize(sizeHint());
    setFixedSize(sizeHint());
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

void ImportDialog::processImage()
{
    if (workImage.isNull()) return;
    QImage snapmaticImage = workImage;
    QPixmap snapmaticPixmap(snapmaticResolutionW, snapmaticResolutionH);
    snapmaticPixmap.fill(selectedColour);
    QPainter snapmaticPainter(&snapmaticPixmap);
    if (!backImage.isNull())
    {
        if (!ui->cbStretch->isChecked())
        {
            int diffWidth = 0;
            int diffHeight = 0;
            if (backImage.width() != snapmaticResolutionW)
            {
                diffWidth = snapmaticResolutionW - backImage.width();
                diffWidth = diffWidth / 2;
            }
            else if (backImage.height() != snapmaticResolutionH)
            {
                diffHeight = snapmaticResolutionH - backImage.height();
                diffHeight = diffHeight / 2;
            }
            snapmaticPainter.drawImage(0 + diffWidth, 0 + diffHeight, backImage);
        }
        else
        {
            snapmaticPainter.drawImage(0, 0, QImage(backImage).scaled(snapmaticResolutionW, snapmaticResolutionH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
        if (ui->cbAvatar->isChecked() && ui->cbForceAvatarColour->isChecked())
        {
            snapmaticPainter.fillRect(snapmaticAvatarPlacementW, snapmaticAvatarPlacementH, snapmaticAvatarResolution, snapmaticAvatarResolution, selectedColour);
        }
    }
    if (insideAvatarZone)
    {
        // Avatar mode
        int diffWidth = 0;
        int diffHeight = 0;
        if (!ui->cbIgnore->isChecked())
        {
            snapmaticImage = snapmaticImage.scaled(snapmaticAvatarResolution, snapmaticAvatarResolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (snapmaticImage.width() > snapmaticImage.height())
            {
                diffHeight = snapmaticAvatarResolution - snapmaticImage.height();
                diffHeight = diffHeight / 2;
            }
            else if (snapmaticImage.width() < snapmaticImage.height())
            {
                diffWidth = snapmaticAvatarResolution - snapmaticImage.width();
                diffWidth = diffWidth / 2;
            }
        }
        else
        {
            snapmaticImage = snapmaticImage.scaled(snapmaticAvatarResolution, snapmaticAvatarResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        snapmaticPainter.drawImage(snapmaticAvatarPlacementW + diffWidth, snapmaticAvatarPlacementH + diffHeight, snapmaticImage);
        imageTitle = tr("Custom Avatar", "Custom Avatar Description in SC, don't use Special Character!");
    }
    else
    {
        // Picture mode
        int diffWidth = 0;
        int diffHeight = 0;
        if (!ui->cbIgnore->isChecked())
        {
            snapmaticImage = snapmaticImage.scaled(snapmaticResolutionW, snapmaticResolutionH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (snapmaticImage.width() != snapmaticResolutionW)
            {
                diffWidth = snapmaticResolutionW - snapmaticImage.width();
                diffWidth = diffWidth / 2;
            }
            else if (snapmaticImage.height() != snapmaticResolutionH)
            {
                diffHeight = snapmaticResolutionH - snapmaticImage.height();
                diffHeight = diffHeight / 2;
            }
        }
        else
        {
            snapmaticImage = snapmaticImage.scaled(snapmaticResolutionW, snapmaticResolutionH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        snapmaticPainter.drawImage(0 + diffWidth, 0 + diffHeight, snapmaticImage);
        imageTitle = tr("Custom Picture", "Custom Picture Description in SC, don't use Special Character!");
    }
    snapmaticPainter.end();
    newImage = snapmaticPixmap.toImage();
    ui->labPicture->setPixmap(snapmaticPixmap.scaled(snapmaticResolutionLW, snapmaticResolutionLH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

QImage ImportDialog::image()
{
    return newImage;
}

void ImportDialog::setImage(QImage *image_)
{
    workImage = QImage();
    if (image_->width() == image_->height())
    {
        insideAvatarZone = true;
        ui->cbAvatar->setChecked(true);
        if (image_->height() > snapmaticResolutionH)
        {
            workImage = image_->scaled(snapmaticResolutionH, snapmaticResolutionH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            delete image_;
        }
        else
        {
            workImage = *image_;
            delete image_;
        }
    }
    else if (image_->width() > snapmaticResolutionW && image_->width() > image_->height())
    {
        workImage = image_->scaledToWidth(snapmaticResolutionW, Qt::SmoothTransformation);
        delete image_;
    }
    else if (image_->height() > snapmaticResolutionH && image_->height() > image_->width())
    {
        workImage = image_->scaledToHeight(snapmaticResolutionH, Qt::SmoothTransformation);
        delete image_;
    }
    else
    {
        workImage = *image_;
        delete image_;
    }
    processImage();
}

bool ImportDialog::isImportAgreed()
{
    return importAgreed;
}

QString ImportDialog::getImageTitle()
{
    return imageTitle;
}

void ImportDialog::on_cbIgnore_toggled(bool checked)
{
    Q_UNUSED(checked)
    processImage();
}

void ImportDialog::on_cbAvatar_toggled(bool checked)
{
    if (workImage.width() == workImage.height() && !checked)
    {
        if (QMessageBox::No == QMessageBox::warning(this, tr("Snapmatic Avatar Zone"), tr("Are you sure to use a square image outside of the Avatar Zone?\nWhen you want to use it as Avatar the image will be detached!"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
        {
            ui->cbAvatar->setChecked(true);
            insideAvatarZone = true;
            return;
        }
    }
    insideAvatarZone = ui->cbAvatar->isChecked();
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
    if (insideAvatarZone)
    {
        QImage avatarAreaFinalImage(avatarAreaImage);
        if (selectedColour.lightness() > 127)
        {
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
    if (newSelectedColour.isValid())
    {
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
    for (QByteArray imageFormat : QImageReader::supportedImageFormats())
    {
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

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);
            QString selectedFileName = QFileInfo(selectedFile).fileName();

            QFile snapmaticFile(selectedFile);
            if (!snapmaticFile.open(QFile::ReadOnly))
            {
                QMessageBox::warning(this, QApplication::translate("ProfileInterface", "Import"), QApplication::translate("ProfileInterface", "Can't import %1 because file can't be open").arg("\""+selectedFileName+"\""));
                goto fileDialogPreOpen;
            }
            QImage importImage;
            QImageReader snapmaticImageReader;
            snapmaticImageReader.setDecideFormatFromContent(true);
            snapmaticImageReader.setDevice(&snapmaticFile);
            if (!snapmaticImageReader.read(&importImage))
            {
                QMessageBox::warning(this, QApplication::translate("ProfileInterface", "Import"), QApplication::translate("ProfileInterface", "Can't import %1 because file can't be parsed properly").arg("\""+selectedFileName+"\""));
                goto fileDialogPreOpen;
            }
            backImage = importImage.scaled(snapmaticResolutionW, snapmaticResolutionH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
