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

#include "ImageEditorDialog.h"
#include "ui_ImageEditorDialog.h"
#include "ProfileInterface.h"
#include "SidebarGenerator.h"
#include "StandardPaths.h"
#include "ImportDialog.h"
#include "AppEnv.h"
#include "config.h"
#include <QStringBuilder>
#include <QImageReader>
#include <QFileDialog>
#include <QMessageBox>

ImageEditorDialog::ImageEditorDialog(SnapmaticPicture *picture, QString profileName, QWidget *parent) :
    QDialog(parent), smpic(picture), profileName(profileName),
    ui(new Ui::ImageEditorDialog)
{
    // Set Window Flags
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);
    ui->cmdClose->setDefault(true);
    ui->cmdClose->setFocus();

    // Set Icon for Close Button
    if (QIcon::hasThemeIcon("dialog-close"))
    {
        ui->cmdClose->setIcon(QIcon::fromTheme("dialog-close"));
    }
    else if (QIcon::hasThemeIcon("gtk-close"))
    {
        ui->cmdClose->setIcon(QIcon::fromTheme("gtk-close"));
    }

    // Set Icon for Import Button
    if (QIcon::hasThemeIcon("document-import"))
    {
        ui->cmdReplace->setIcon(QIcon::fromTheme("document-import"));
    }

    // Set Icon for Overwrite Button
    if (QIcon::hasThemeIcon("document-save"))
    {
        ui->cmdSave->setIcon(QIcon::fromTheme("document-save"));
    }
    else if (QIcon::hasThemeIcon("gtk-save"))
    {
        ui->cmdSave->setIcon(QIcon::fromTheme("gtk-save"));
    }

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();

    snapmaticResolutionLW = 516 * screenRatio; // 430
    snapmaticResolutionLH = 288 * screenRatio; // 240
    ui->labPicture->setMinimumSize(snapmaticResolutionLW, snapmaticResolutionLH);
    ui->labCapacity->setText(tr("Capacity: %1").arg(QString::number(qRound((double)picture->getContentMaxLength() / 1024)) % " KB"));

    imageIsChanged = false;
    pictureCache = picture->getImage();
    ui->labPicture->setPixmap(QPixmap::fromImage(pictureCache).scaled(snapmaticResolutionLW, snapmaticResolutionLH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    setMaximumSize(sizeHint());
    setMinimumSize(sizeHint());
    setFixedSize(sizeHint());
}

ImageEditorDialog::~ImageEditorDialog()
{
    delete ui;
}

void ImageEditorDialog::on_cmdClose_clicked()
{
    close();
}

void ImageEditorDialog::on_cmdReplace_clicked()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("FileDialogs");
    bool dontUseNativeDialog = settings.value("DontUseNativeDialog", false).toBool();
    settings.beginGroup("ImportReplace");

fileDialogPreOpen: //Work?
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, dontUseNativeDialog);
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(ProfileInterface::tr("Import..."));
    fileDialog.setLabelText(QFileDialog::Accept, ProfileInterface::tr("Import"));

    // Getting readable Image formats
    QString imageFormatsStr = " ";
    for (QByteArray imageFormat : QImageReader::supportedImageFormats())
    {
        imageFormatsStr += QString("*.") % QString::fromUtf8(imageFormat).toLower() % " ";
    }

    QStringList filters;
    filters << ProfileInterface::tr("All image files (%1)").arg(imageFormatsStr.trimmed());
    filters << ProfileInterface::tr("All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.setDirectory(settings.value(profileName % "+Directory", StandardPaths::documentsLocation()).toString());
    fileDialog.restoreGeometry(settings.value(profileName % "+Geometry", "").toByteArray());

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
                QMessageBox::warning(this, ProfileInterface::tr("Import"), ProfileInterface::tr("Can't import %1 because file can't be open").arg("\""+selectedFileName+"\""));
                goto fileDialogPreOpen;
            }
            QImage *importImage = new QImage();
            QImageReader snapmaticImageReader;
            snapmaticImageReader.setDecideFormatFromContent(true);
            snapmaticImageReader.setDevice(&snapmaticFile);
            if (!snapmaticImageReader.read(importImage))
            {
                QMessageBox::warning(this, ProfileInterface::tr("Import"), ProfileInterface::tr("Can't import %1 because file can't be parsed properly").arg("\""+selectedFileName+"\""));
                delete importImage;
                goto fileDialogPreOpen;
            }
            ImportDialog *importDialog = new ImportDialog(this);
            importDialog->setImage(importImage);
            importDialog->setModal(true);
            importDialog->show();
            importDialog->exec();
            if (importDialog->isImportAgreed())
            {
                pictureCache = importDialog->image();
                ui->labPicture->setPixmap(QPixmap::fromImage(pictureCache).scaled(snapmaticResolutionLW, snapmaticResolutionLH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                imageIsChanged = true;
            }
            delete importDialog;
        }
    }

    settings.setValue(profileName % "+Geometry", fileDialog.saveGeometry());
    settings.setValue(profileName % "+Directory", fileDialog.directory().absolutePath());
    settings.endGroup();
    settings.endGroup();
}

void ImageEditorDialog::on_cmdSave_clicked()
{
    if (imageIsChanged)
    {
        const QByteArray previousPicture = smpic->getPictureStream();
        bool success = smpic->setImage(pictureCache);
        if (success)
        {
            QString currentFilePath = smpic->getPictureFilePath();
            QString originalFilePath = smpic->getOriginalPictureFilePath();
            QString backupFileName = originalFilePath % ".bak";
            if (!QFile::exists(backupFileName))
            {
                QFile::copy(currentFilePath, backupFileName);
            }
            if (!smpic->exportPicture(currentFilePath))
            {
                smpic->setPictureStream(previousPicture);
                QMessageBox::warning(this, tr("Snapmatic Image Editor"), tr("Patching of Snapmatic Image failed because of I/O Error"));
                return;
            }
            smpic->emitCustomSignal("PictureUpdated");
        }
        else
        {
            QMessageBox::warning(this, tr("Snapmatic Image Editor"), tr("Patching of Snapmatic Image failed because of Image Error"));
            return;
        }
    }
    close();
}

void ImageEditorDialog::on_cmdQuestion_clicked()
{
    QMessageBox::information(this, tr("Snapmatic Image Editor"), tr("Every taken Snapmatic have a different Capacity, a Snapmatic with higher Capacity can store a picture with better quality."));

}
