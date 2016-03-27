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
    QDir profileDir;
    profileDir.setPath(profileFolder);
    profileDir.setNameFilters(QStringList("SGTA*"));
    QStringList SavegameFiles = profileDir.entryList(QDir::Files | QDir::NoDot, QDir::NoSort);
    QStringList BackupFiles = SavegameFiles.filter(".bak", Qt::CaseInsensitive);
    foreach(const QString &BackupFile, BackupFiles)
    {
        SavegameFiles.removeAll(BackupFile);
    }
    foreach(const QString &SavegameFile, SavegameFiles)
    {
        QString sgdPath = profileFolder + "/" + SavegameFile;
        SavegameData *savegame = new SavegameData(sgdPath);
        if (savegame->readingSavegame())
        {
            emit savegameLoaded(savegame, sgdPath);
        }
    }

    profileDir.setNameFilters(QStringList("PGTA*"));
    QStringList SnapmaticPics = profileDir.entryList(QDir::Files | QDir::NoDot, QDir::NoSort);
    foreach(const QString &SnapmaticPic, SnapmaticPics)
    {
        QString picturePath = profileFolder + "/" + SnapmaticPic;
        SnapmaticPicture *picture = new SnapmaticPicture(picturePath);
        if (picture->readingPicture())
        {
            emit pictureLoaded(picture, picturePath);
            crewDB->addCrew(picture->getCrewNumber());
        }
    }
}
