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

#ifndef PROFILEINTERFACE_H
#define PROFILEINTERFACE_H

#include "SnapmaticPicture.h"
#include "SnapmaticWidget.h"
#include "ProfileDatabase.h"
#include "DatabaseThread.h"
#include "ProfileLoader.h"
#include "SavegameData.h"
#include "CrewDatabase.h"
#include <QWidget>
#include <QList>

namespace Ui {
class ProfileInterface;
}

class ProfileInterface : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileInterface(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent = 0);
    void setProfileFolder(QString folder, QString profile);
    void setupProfileInterface();
    ~ProfileInterface();

private slots:
    void on_cmdCloseProfile_clicked();
    void on_pictureLoaded(SnapmaticPicture *picture, QString picturePath);
    void on_savegameLoaded(SavegameData *savegame, QString savegamePath);
    void on_loadingProgress(int value, int maximum);
    void on_pictureDeleted();
    void on_profileLoaded();

private:
    ProfileDatabase *profileDB;
    CrewDatabase *crewDB;
    DatabaseThread *threadDB;
    Ui::ProfileInterface *ui;

    ProfileLoader *profileLoader;
    QList<SavegameData*> savegames;
    QList<SnapmaticPicture*> pictures;
    QList<QWidget*> widgets;
    QString profileFolder;
    QString profileName;
    QString loadingStr;

signals:
    void profileClosed();
};

#endif // PROFILEINTERFACE_H
