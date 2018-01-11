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

#include "config.h"
#include "PictureExport.h"
#include "PictureDialog.h"
#include "StandardPaths.h"
#include "SidebarGenerator.h"
#include <QStringBuilder>
#include <QDesktopWidget>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QRegExp>
#include <QDebug>

#if QT_VERSION >= 0x050000
#include <QSaveFile>
#endif

PictureExport::PictureExport()
{

}

void PictureExport::exportAsPicture(QWidget *parent, SnapmaticPicture *picture)
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);

    // Picture Settings
    // Quality Settings
    settings.beginGroup("Pictures");
    int defaultQuality = 100;
    QSize defExportSize = QSize(960, 536);
    int customQuality = settings.value("CustomQuality", defaultQuality).toInt();
    if (customQuality < 1 || customQuality > 100)
    {
        customQuality = 100;
    }
    bool useCustomQuality = settings.value("CustomQualityEnabled", false).toBool();

    // Size Settings
    QSize cusExportSize = settings.value("CustomSize", defExportSize).toSize();
    if (cusExportSize.width() > 3840)
    {
        cusExportSize.setWidth(3840);
    }
    else if (cusExportSize.height() > 2160)
    {
        cusExportSize.setHeight(2160);
    }
    if (cusExportSize.width() < 1)
    {
        cusExportSize.setWidth(1);
    }
    else if (cusExportSize.height() < 1)
    {
        cusExportSize.setHeight(1);
    }
    QString sizeMode = settings.value("ExportSizeMode", "Default").toString();
    Qt::AspectRatioMode aspectRatio = (Qt::AspectRatioMode)settings.value("AspectRatio", Qt::KeepAspectRatio).toInt();
    QString defaultExportFormat = settings.value("DefaultExportFormat", ".jpg").toString();
    settings.endGroup();
    // End Picture Settings

    settings.beginGroup("FileDialogs");
    bool dontUseNativeDialog = settings.value("DontUseNativeDialog", false).toBool();
    settings.beginGroup("ExportAsPicture");

fileDialogPreSave: //Work?
    QFileDialog fileDialog(parent);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, dontUseNativeDialog);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite, true);
    fileDialog.setDefaultSuffix("suffix");
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(PictureDialog::tr("Export as Picture..."));
    fileDialog.setLabelText(QFileDialog::Accept, PictureDialog::tr("Export"));

    QStringList filters;
    filters << PictureDialog::tr("JPEG Graphics (*.jpg *.jpeg)");
    filters << PictureDialog::tr("Portable Network Graphics (*.png)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.setDirectory(settings.value("Directory", StandardPaths::picturesLocation()).toString());
    fileDialog.restoreGeometry(settings.value(parent->objectName() % "+Geometry", "").toByteArray());

    QString newPictureFileName = getPictureFileName(picture) % defaultExportFormat;
    fileDialog.selectFile(newPictureFileName);

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString saveFileFormat;
            QString selectedFile = selectedFiles.at(0);

            if (selectedFile.right(4) == ".jpg")
            {
                saveFileFormat = "JPEG";
            }
            else if (selectedFile.right(4) == ".jpeg")
            {
                saveFileFormat = "JPEG";
            }
            else if (selectedFile.right(4) == ".png")
            {
                saveFileFormat = "PNG";
            }
            else if (selectedFile.right(7) == ".suffix")
            {
                if (fileDialog.selectedNameFilter() == "JPEG picture (*.jpg)")
                {
                    selectedFile.replace(".suffix", ".jpg");
                }
                else if (fileDialog.selectedNameFilter() == "Portable Network Graphics (*.png)")
                {
                    selectedFile.replace(".suffix", ".png");
                }
                else
                {
                    selectedFile.replace(".suffix", ".jpg");
                }
            }

            if (QFile::exists(selectedFile))
            {
                if (QMessageBox::No == QMessageBox::warning(parent, PictureDialog::tr("Export as Picture"), PictureDialog::tr("Overwrite %1 with current Snapmatic picture?").arg("\""+selectedFile+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
                {
                    goto fileDialogPreSave; //Work?
                }
            }

            // Scale Picture
            QImage exportPicture = picture->getImage();
            if (sizeMode == "Desktop")
            {
                QRect desktopResolution = QApplication::desktop()->screenGeometry();
                exportPicture = exportPicture.scaled(desktopResolution.width(), desktopResolution.height(), aspectRatio, Qt::SmoothTransformation);
            }
            else if (sizeMode == "Custom")
            {
                exportPicture = exportPicture.scaled(cusExportSize, aspectRatio, Qt::SmoothTransformation);
            }

            int errorId = 0;
            bool isSaved = false;
#if QT_VERSION >= 0x050000
            QSaveFile *picFile = new QSaveFile(selectedFile);
#else
            QFile *picFile = new QFile(selectedFile);
#endif
            if (picFile->open(QIODevice::WriteOnly))
            {
                isSaved = exportPicture.save(picFile, saveFileFormat.toStdString().c_str(), useCustomQuality ? customQuality : defaultQuality);
#if QT_VERSION >= 0x050000
                if (isSaved)
                {
                    isSaved = picFile->commit();
                }
                else
                {
                    errorId = 1;
                }
#else
                picFile->close();
#endif
            }
            else
            {
                errorId = 2;
            }
            delete picFile;

            if (!isSaved)
            {
                switch (errorId)
                {
                case 0:
                    QMessageBox::warning(parent, PictureDialog::tr("Export as Picture"), PictureDialog::tr("Failed to export the picture because the system occurred a write failure"));
                    break;
                case 1:
                    QMessageBox::warning(parent, PictureDialog::tr("Export as Picture"), PictureDialog::tr("Failed to export the picture because the format detection failures"));
                    break;
                case 2:
                    QMessageBox::warning(parent, PictureDialog::tr("Export as Picture"), PictureDialog::tr("Failed to export the picture because the file can't be written"));
                    break;
                default:
                    QMessageBox::warning(parent, PictureDialog::tr("Export as Picture"), PictureDialog::tr("Failed to export the picture because of an unknown reason"));
                }
                goto fileDialogPreSave; //Work?
            }
        }
        else
        {
            QMessageBox::warning(parent, PictureDialog::tr("Export as Picture"), PictureDialog::tr("No valid file is selected"));
            goto fileDialogPreSave; //Work?
        }
    }

    settings.setValue(parent->objectName() % "+Geometry", fileDialog.saveGeometry());
    settings.setValue("Directory", fileDialog.directory().absolutePath());
    settings.endGroup();
    settings.endGroup();
}

void PictureExport::exportAsSnapmatic(QWidget *parent, SnapmaticPicture *picture)
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("FileDialogs");
    bool dontUseNativeDialog = settings.value("DontUseNativeDialog", false).toBool();
    settings.beginGroup("ExportAsSnapmatic");

    QString adjustedPicPath = picture->getOriginalPictureFileName();

fileDialogPreSave: //Work?
    QFileInfo sgdFileInfo(adjustedPicPath);
    QFileDialog fileDialog(parent);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, dontUseNativeDialog);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite, true);
    fileDialog.setDefaultSuffix(".rem");
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(PictureDialog::tr("Export as Snapmatic..."));
    fileDialog.setLabelText(QFileDialog::Accept, PictureDialog::tr("Export"));

    QStringList filters;
    filters << PictureDialog::tr("GTA V Export (*.g5e)");
    filters << PictureDialog::tr("GTA V Raw Export (*.auto)");
    filters << PictureDialog::tr("Snapmatic pictures (PGTA*)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.setDirectory(settings.value("Directory", StandardPaths::documentsLocation()).toString());
    fileDialog.restoreGeometry(settings.value(parent->objectName() % "+Geometry", "").toByteArray());
    fileDialog.selectFile(QString(picture->getExportPictureFileName() % ".g5e"));

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);
            bool isAutoExt = false;
            if (selectedFile.right(5) == ".auto")
            {
                isAutoExt = true;
                QString dirPath = QFileInfo(selectedFile).dir().path();
                QString stockFileName = sgdFileInfo.fileName();
                selectedFile = dirPath % "/" % stockFileName;
            }
            else if (selectedFile.right(4) == ".rem")
            {
                selectedFile.remove(selectedFile.length() - 4, 4);
            }

            if (QFile::exists(selectedFile))
            {
                if (QMessageBox::No == QMessageBox::warning(parent, PictureDialog::tr("Export as Snapmatic"), PictureDialog::tr("Overwrite %1 with current Snapmatic picture?").arg("\""+selectedFile+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
                {
                    goto fileDialogPreSave; //Work?
                }
            }

            if (selectedFile.right(4) == ".g5e")
            {
                bool isExported = picture->exportPicture(selectedFile, SnapmaticFormat::G5E_Format);
                if (!isExported)
                {
                    QMessageBox::warning(parent, PictureDialog::tr("Export as Snapmatic"), PictureDialog::tr("Failed to export current Snapmatic picture"));
                    goto fileDialogPreSave; //Work?
                }
            }
            else
            {
                bool isCopied = picture->exportPicture(selectedFile, SnapmaticFormat::PGTA_Format);
                if (!isCopied)
                {
                    QMessageBox::warning(parent, PictureDialog::tr("Export as Snapmatic"), PictureDialog::tr("Failed to export current Snapmatic picture"));
                    goto fileDialogPreSave; //Work?
                }
                else
                {
                    if (isAutoExt) QMessageBox::information(parent, PictureDialog::tr("Export as Snapmatic"), PictureDialog::tr("Exported Snapmatic to \"%1\" because of using the .auto extension.").arg(selectedFile));
                }
            }
        }
        else
        {
            QMessageBox::warning(parent, PictureDialog::tr("Export as Snapmatic"), PictureDialog::tr("No valid file is selected"));
            goto fileDialogPreSave; //Work?
        }
    }

    settings.setValue(parent->objectName() % "+Geometry", fileDialog.saveGeometry());
    settings.setValue("Directory", fileDialog.directory().absolutePath());
    settings.endGroup();
}

QString PictureExport::getPictureFileName(SnapmaticPicture *picture)
{
    return picture->getExportPictureFileName();
}
