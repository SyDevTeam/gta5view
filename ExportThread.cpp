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

#include "SnapmaticPicture.h"
#include "PictureExport.h"
#include "ProfileWidget.h"
#include "ExportThread.h"
#include "SavegameData.h"
#include <QFileInfo>
#include <QFile>

ExportThread::ExportThread(QMap<ProfileWidget*,QString> profileMap, QString exportDirectory, bool pictureCopyEnabled, bool pictureExportEnabled, int exportCount, QObject *parent) : QThread(parent),
    profileMap(profileMap), exportDirectory(exportDirectory), pictureCopyEnabled(pictureCopyEnabled), pictureExportEnabled(pictureExportEnabled), exportCount(exportCount)
{

}

void ExportThread::run()
{
    int intExportProgress = 0;
    foreach(ProfileWidget *widget, profileMap.keys())
    {
        if (widget->isSelected())
        {
            if (profileMap[widget] == "SnapmaticWidget")
            {
                SnapmaticWidget *picWidget = (SnapmaticWidget*)widget;
                SnapmaticPicture *picture = picWidget->getPicture();

                if (pictureExportEnabled)
                {
                    QString exportFileName = PictureExport::getPictureFileName(picture);

                    intExportProgress++;
                    emit exportStringUpdate(ProfileInterface::tr("Export file %1 of %2 files").arg(QString::number(intExportProgress), QString::number(exportCount)));
                    emit exportProgressUpdate(intExportProgress);

                    if (!picture->getPicture().save(exportDirectory + "/" + exportFileName, "JPEG", 100))
                    {
                        failedExportPictures.append(exportFileName);
                    }
                }
                if (pictureCopyEnabled)
                {
                    QString originalFileName = picture->getPictureFileName();
                    QFileInfo originalFileInfo(originalFileName);
                    QString exportFileName = originalFileInfo.fileName();

                    intExportProgress++;
                    emit exportStringUpdate(ProfileInterface::tr("Export file %1 of %2 files").arg(QString::number(intExportProgress), QString::number(exportCount)));
                    emit exportProgressUpdate(intExportProgress);

                    QString exportFilePath = exportDirectory + "/" + exportFileName;
                    if (QFile::exists(exportFilePath)) {QFile::remove(exportFilePath);}
                    if (!QFile::copy(originalFileName, exportFilePath))
                    {
                        failedCopyPictures.append(exportFileName);
                    }
                }
            }
            else if (profileMap[widget] == "SavegameWidget")
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
