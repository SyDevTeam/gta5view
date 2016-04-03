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

#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "ProfileInterface.h"
#include "ProfileDatabase.h"
#include "DatabaseThread.h"
#include "CrewDatabase.h"
#include <QMainWindow>
#include <QString>
#include <QMap>

namespace Ui {
class UserInterface;
}

class UserInterface : public QMainWindow
{
    Q_OBJECT
public:
    explicit UserInterface(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent = 0);
    ~UserInterface();

private slots:
    void closeProfile();
    void profileLoaded();
    void profileButton_clicked();
    void reloadProfiles_clicked();
    void on_actionExit_triggered();
    void on_actionSelect_profile_triggered();
    void on_actionAbout_gta5sync_triggered();
    void on_actionSelect_all_triggered();
    void on_actionDeselect_all_triggered();
    void on_actionExport_selected_triggered();
    void on_actionDelete_selected_triggered();

private:
    ProfileDatabase *profileDB;
    CrewDatabase *crewDB;
    DatabaseThread *threadDB;
    Ui::UserInterface *ui;
    ProfileInterface *profileUI;
    bool profileOpen;
    QString defaultWindowTitle;
    QString GTAV_Folder;
    QString GTAV_ProfilesFolder;
    void setupProfileUi(QStringList GTAV_Profiles);
    void openProfile(QString profileName);
    void openSelectProfile();
};

#endif // USERINTERFACE_H
