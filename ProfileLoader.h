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

#ifndef PROFILELOADER_H
#define PROFILELOADER_H

#include "SnapmaticPicture.h"
#include "SavegameData.h"
#include "CrewDatabase.h"
#include <QThread>
#include <QDir>

class ProfileLoader : public QThread
{
    Q_OBJECT
public:
    explicit ProfileLoader(QString profileFolder, CrewDatabase *crewDB, QObject *parent = 0);

protected:
    void run();

private:
    QString profileFolder;
    CrewDatabase *crewDB;
    ProfileLoader *profileLoader;

private slots:
    void preloaded();
    void loaded();

signals:
    void pictureLoaded(SnapmaticPicture *picture);
    void savegameLoaded(SavegameData *savegame, QString savegamePath);
    void loadingProgress(int value, int maximum);
};

#endif // PROFILELOADER_H
