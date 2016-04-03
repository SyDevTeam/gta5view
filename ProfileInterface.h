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
#include "ProfileWidget.h"
#include "SavegameData.h"
#include "CrewDatabase.h"
#include <QSpacerItem>
#include <QWidget>
#include <QList>
#include <QMap>

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

public slots:
    void selectAllWidgets();
    void deselectAllWidgets();
    void exportSelected();
    void deleteSelected();

private slots:
    void on_cmdCloseProfile_clicked();
    void on_cmdImport_clicked();
    void pictureLoaded(SnapmaticPicture *picture, QString picturePath);
    void savegameLoaded(SavegameData *savegame, QString savegamePath);
    void loadingProgress(int value, int maximum);
    void pictureDeleted();
    void savegameDeleted();
    void profileLoaded_p();
    void profileWidgetSelected();
    void profileWidgetDeselected();

private:
    ProfileDatabase *profileDB;
    CrewDatabase *crewDB;
    DatabaseThread *threadDB;
    Ui::ProfileInterface *ui;

    ProfileLoader *profileLoader;
    QList<SavegameData*> savegames;
    QList<SnapmaticPicture*> pictures;
    QMap<ProfileWidget*,QString> widgets;
    QSpacerItem *saSpacerItem;
    QString profileFolder;
    QString profileName;
    QString loadingStr;
    int selectedWidgts;

    bool importSnapmaticPicture(SnapmaticPicture *picture, QString picPath);
    bool importSavegameData(SavegameData *savegame, QString sgdPath);

signals:
    void profileLoaded();
    void profileClosed();
};

#endif // PROFILEINTERFACE_H
