/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2020 Syping
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
#include "AppEnv.h"
#include "config.h"
#include <QStringBuilder>
#include <QApplication>
#include <QSaveFile>
#include <QFileInfo>
#include <QFile>

#if QT_VERSION >= 0x050000
#include <QScreen>
#else
#include <QDesktopWidget>
#endif

ExportThread::ExportThread(QMap<ProfileWidget*,QString> profileMap, QString exportDirectory, bool pictureCopyEnabled, bool pictureExportEnabled, int exportCount, QObject *parent) : QThread(parent),
    profileMap(profileMap), exportDirectory(exportDirectory), pictureCopyEnabled(pictureCopyEnabled), pictureExportEnabled(pictureExportEnabled), exportCount(exportCount)
{
}

void ExportThread::run()
{
    size_t intExportProgress = 0;
    for (ProfileWidget *widget : profileMap.keys()) {
        if (widget->isSelected()) {
            if (widget->getWidgetType() == "SnapmaticWidget") {
                SnapmaticWidget *picWidget = qobject_cast<SnapmaticWidget*>(widget);
                SnapmaticPicture *picture = picWidget->getPicture();

                if (pictureExportEnabled) {
                    QString exportFileName = PictureExport::getPictureFileName(picture);
                    if (!exportFileName.endsWith(".jpg"))
                        exportFileName += ".jpg";

                    intExportProgress++;
                    emit exportStringUpdate(ProfileInterface::tr("Export file %1 of %2 files").arg(QString::number(intExportProgress), QString::number(exportCount)));
                    emit exportProgressUpdate(intExportProgress);

                    QSaveFile exportFile(exportDirectory % "/" % exportFileName);
                    exportFile.write(picture->getPictureStream());
                    bool isSaved = exportFile.commit();

                    if (!isSaved)
                        failedExportPictures += exportFileName;
                }
                if (pictureCopyEnabled) {
                    QString exportFileName = PictureExport::getPictureFileName(picture);
                    if (!exportFileName.endsWith(".g5e"))
                        exportFileName += ".g5e";

                    intExportProgress++;
                    emit exportStringUpdate(ProfileInterface::tr("Export file %1 of %2 files").arg(QString::number(intExportProgress), QString::number(exportCount)));
                    emit exportProgressUpdate(intExportProgress);

                    QString exportFilePath = exportDirectory % "/" % exportFileName;
                    if (QFile::exists(exportFilePath))
                        QFile::remove(exportFilePath);
                    if (!picture->exportPicture(exportDirectory % "/" % exportFileName, SnapmaticFormat::G5E_Format))
                        failedCopyPictures += exportFileName;
                }
            }
            else if (widget->getWidgetType() == "SavegameWidget") {
                SavegameWidget *sgdWidget = qobject_cast<SavegameWidget*>(widget);
                SavegameData *savegame = sgdWidget->getSavegame();

                QString originalFileName = savegame->getSavegameFileName();
                QFileInfo originalFileInfo(originalFileName);
                QString exportFileName = originalFileInfo.fileName();

                intExportProgress++;
                emit exportStringUpdate(ProfileInterface::tr("Export file %1 of %2 files").arg(QString::number(intExportProgress), QString::number(exportCount)));
                emit exportProgressUpdate(intExportProgress);

                QString exportFilePath = exportDirectory % "/" % exportFileName;
                if (QFile::exists(exportFilePath))
                    QFile::remove(exportFilePath);
                if (!QFile::copy(originalFileName, exportFilePath))
                    failedSavegames += exportFileName;
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
