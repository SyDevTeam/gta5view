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

#include "SnapmaticPicture.h"
#include "ProfileLoader.h"
#include "SavegameData.h"
#include "CrewDatabase.h"
#include <QStringBuilder>
#include <QVector>
#include <QString>
#include <QFile>
#ifdef Q_OS_WIN
#include <QDir>
#include <QList>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

ProfileLoader::ProfileLoader(QString profileFolder, CrewDatabase *crewDB, QObject *parent) : QThread(parent), profileFolder(profileFolder), crewDB(crewDB)
{
}

void ProfileLoader::run()
{
    int curFile = 1;
    int maximumV = 0;
    QVector<int> crewList;
    QVector<QString> savegameFiles;
    QVector<QString> snapmaticPics;

    QDir dir(profileFolder);
    const QStringList files = dir.entryList(QDir::Files);
    for (const QString &fileName : files) {
        if ((fileName.startsWith("SGTA5") || fileName.startsWith("SRDR3")) && !fileName.endsWith(".bak")) {
            savegameFiles << fileName;
            maximumV++;
        }
        if ((fileName.startsWith("PGTA5") || fileName.startsWith("PRDR3")) && !fileName.endsWith(".bak")) {
            snapmaticPics << fileName;
            maximumV++;
        }
    }

    // Directory successfully scanned
    emit directoryScanned(savegameFiles, snapmaticPics);

    // Loading pictures and savegames
    emit loadingProgress(curFile, maximumV);
    for (const QString &SavegameFile : qAsConst(savegameFiles)) {
        emit loadingProgress(curFile, maximumV);
        const QString sgdPath = profileFolder % "/" % SavegameFile;
        SavegameData *savegame = new SavegameData(sgdPath);
        if (savegame->readingSavegame()) {
            emit savegameLoaded(savegame, sgdPath);
        }
        curFile++;
    }
    for (const QString &SnapmaticPic : qAsConst(snapmaticPics)) {
        emit loadingProgress(curFile, maximumV);
        const QString picturePath = profileFolder % "/" % SnapmaticPic;
        SnapmaticPicture *picture = new SnapmaticPicture(picturePath);
        if (picture->readingPicture(true)) {
            if (picture->isFormatSwitched()) {
                picture->setSnapmaticFormat(SnapmaticFormat::PGTA5_Format);
                if (picture->exportPicture(picturePath, SnapmaticFormat::PGTA5_Format)) {
                    emit pictureFixed(picture);
                }
            }
            emit pictureLoaded(picture);
            int crewNumber = picture->getSnapmaticProperties().crewID;
            if (!crewList.contains(crewNumber)) {
                crewList += crewNumber;
            }
        }
        curFile++;
    }

    // adding found crews
    crewDB->setAddingCrews(true);
    for (int crewID : qAsConst(crewList)) {
        crewDB->addCrew(crewID);
    }
    crewDB->setAddingCrews(false);
}

void ProfileLoader::preloaded()
{
}

void ProfileLoader::loaded()
{
}

