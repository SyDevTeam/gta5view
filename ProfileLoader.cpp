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

#include "ProfileLoader.h"
#include "SnapmaticPicture.h"
#include "SavegameData.h"
#include "CrewDatabase.h"
#include <QStringList>
#include <QString>
#include <QFile>
#include <QDir>

ProfileLoader::ProfileLoader(QString profileFolder, CrewDatabase *crewDB, QObject *parent) : QThread(parent), profileFolder(profileFolder), crewDB(crewDB)
{

}

void ProfileLoader::run()
{
    int curFile = 1;
    QDir profileDir;
    QList<int> crewList;
    profileDir.setPath(profileFolder);

    // Seek pictures and savegames
    profileDir.setNameFilters(QStringList("SGTA*"));
    QStringList SavegameFiles = profileDir.entryList(QDir::Files | QDir::NoDot, QDir::NoSort);
    QStringList BackupFiles = SavegameFiles.filter(".bak", Qt::CaseInsensitive);
    profileDir.setNameFilters(QStringList("PGTA*"));
    QStringList SnapmaticPics = profileDir.entryList(QDir::Files | QDir::NoDot, QDir::NoSort);
    BackupFiles.append(SnapmaticPics.filter(".bak", Qt::CaseInsensitive));

    SavegameFiles.removeDuplicates();
    SnapmaticPics.removeDuplicates();
    foreach(const QString &BackupFile, BackupFiles)
    {
        SavegameFiles.removeAll(BackupFile);
        SnapmaticPics.removeAll(BackupFile);
    }

    int maximumV = SavegameFiles.length() + SnapmaticPics.length();

    // Loading pictures and savegames
    emit loadingProgress(curFile, maximumV);
    foreach(const QString &SavegameFile, SavegameFiles)
    {
        emit loadingProgress(curFile, maximumV);
        QString sgdPath = profileFolder + QDir::separator() + SavegameFile;
        SavegameData *savegame = new SavegameData(sgdPath);
        if (savegame->readingSavegame())
        {
            emit savegameLoaded(savegame, sgdPath);
        }
        curFile++;
    }
    foreach(const QString &SnapmaticPic, SnapmaticPics)
    {
        emit loadingProgress(curFile, maximumV);
        QString picturePath = profileFolder + QDir::separator() + SnapmaticPic;
        SnapmaticPicture *picture = new SnapmaticPicture(picturePath);
        if (picture->readingPicture(true, true, true))
        {
            emit pictureLoaded(picture);
            int crewNumber = picture->getSnapmaticProperties().crewID;
            if (!crewList.contains(crewNumber))
            {
                crewList.append(crewNumber);
            }
        }
        curFile++;
    }

    // adding found crews
    foreach(int crewID, crewList)
    {
        crewDB->addCrew(crewID);
    }
}

void ProfileLoader::preloaded()
{

}

void ProfileLoader::loaded()
{

}
