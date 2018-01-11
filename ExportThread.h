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

#ifndef EXPORTTHREAD_H
#define EXPORTTHREAD_H

#include "SnapmaticWidget.h"
#include "SavegameWidget.h"
#include "ProfileWidget.h"
#include <QThread>
#include <QMap>

class ExportThread : public QThread
{
    Q_OBJECT
public:
    explicit ExportThread(QMap<ProfileWidget*,QString> profileMap, QString exportDirectory, bool pictureCopyEnabled, bool pictureExportEnabled, int exportCount, QObject *parent = 0);
    QStringList getFailedSavegames();
    QStringList getFailedCopyPictures();
    QStringList getFailedExportPictures();

protected:
    void run();

private:
    QMap <ProfileWidget*, QString> profileMap;
    QString exportDirectory;
    bool pictureCopyEnabled;
    bool pictureExportEnabled;
    int exportCount;
    QStringList failedSavegames;
    QStringList failedCopyPictures;
    QStringList failedExportPictures;

signals:
    void exportStringUpdate(QString currentFileName);
    void exportProgressUpdate(int currentProgressValue);
    void exportFinished();
};

#endif // EXPORTTHREAD_H
