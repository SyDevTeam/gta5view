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

#include "SnapmaticPicture.h"
#include "ProfileInterface.h"
#include "PictureExport.h"
#include "ProfileWidget.h"
#include "ExportThread.h"
#include "SavegameData.h"
#include "config.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QFileInfo>
#include <QDebug>
#include <QFile>

ExportThread::ExportThread(QMap<ProfileWidget*,QString> profileMap, QString exportDirectory, bool pictureCopyEnabled, bool pictureExportEnabled, int exportCount, QObject *parent) : QThread(parent),
    profileMap(profileMap), exportDirectory(exportDirectory), pictureCopyEnabled(pictureCopyEnabled), pictureExportEnabled(pictureExportEnabled), exportCount(exportCount)
{

}

void ExportThread::run()
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
    settings.endGroup();
    // End Picture Settings

    int intExportProgress = 0;
    foreach(ProfileWidget *widget, profileMap.keys())
    {
        if (widget->isSelected())
        {
            if (widget->getWidgetType() == "SnapmaticWidget")
            {
                SnapmaticWidget *picWidget = (SnapmaticWidget*)widget;
                SnapmaticPicture *picture = picWidget->getPicture();

                if (pictureExportEnabled)
                {
                    QString exportFileName = PictureExport::getPictureFileName(picture);
                    if (exportFileName.right(4) != ".jpg" && exportFileName.right(4) != ".png")
                    {
                        exportFileName.append(".jpg");
                    }

                    intExportProgress++;
                    emit exportStringUpdate(ProfileInterface::tr("Export file %1 of %2 files").arg(QString::number(intExportProgress), QString::number(exportCount)));
                    emit exportProgressUpdate(intExportProgress);

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

                    bool isSaved;
                    if (useCustomQuality)
                    {
                        isSaved = exportPicture.save(exportDirectory + "/" + exportFileName, "JPEG", customQuality);
                    }
                    else
                    {
                        isSaved = exportPicture.save(exportDirectory + "/" + exportFileName, "JPEG", 100);
                    }

                    if (!isSaved)
                    {
                        failedExportPictures.append(exportFileName);
                    }
                }
                if (pictureCopyEnabled)
                {
                    QString exportFileName = PictureExport::getPictureFileName(picture);
                    if (exportFileName.right(4) != ".g5e")
                    {
                        exportFileName.append(".g5e");
                    }

                    intExportProgress++;
                    emit exportStringUpdate(ProfileInterface::tr("Export file %1 of %2 files").arg(QString::number(intExportProgress), QString::number(exportCount)));
                    emit exportProgressUpdate(intExportProgress);

                    QString exportFilePath = exportDirectory + "/" + exportFileName;
                    if (QFile::exists(exportFilePath)) {QFile::remove(exportFilePath);}
                    if (!picture->exportPicture(exportDirectory + "/" + exportFileName, true))
                    {
                        failedCopyPictures.append(exportFileName);
                    }
                }
            }
            else if (widget->getWidgetType() == "SavegameWidget")
            {
                SavegameWidget *sgdWidget = (SavegameWidget*)widget;
                SavegameData *savegame = sgdWidget->getSavegame();

                QString originalFileName = savegame->getSavegameFileName();
                QFileInfo originalFileInfo(originalFileName);
                QString exportFileName = originalFileInfo.fileName();

                intExportProgress++;
                emit exportStringUpdate(ProfileInterface::tr("Export file %1 of %2 files").arg(QString::number(intExportProgress), QString::number(exportCount)));
                emit exportProgressUpdate(intExportProgress);

                QString exportFilePath = exportDirectory + "/" + exportFileName;
                if (QFile::exists(exportFilePath)) {QFile::remove(exportFilePath);}
                if (!QFile::copy(originalFileName, exportFilePath))
                {
                    failedSavegames.append(exportFileName);
                }
            }
        }
    }
    emit exportFinished();
}

QStringList ExportThread::getFailedCopyPictures()
{
    return failedCopyPictures;
}

QStringList ExportThread::getFailedExportPictures()
{
    return failedExportPictures;
}

QStringList ExportThread::getFailedSavegames()
{
    return failedSavegames;
}
