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

#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "SnapmaticPicture.h"
#include "ProfileInterface.h"
#include "ProfileDatabase.h"
#include "DatabaseThread.h"
#include "CrewDatabase.h"
#include "SavegameData.h"
#include <QMainWindow>
#include <QMouseEvent>
#include <QCloseEvent>
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
    void setupDirEnv();
    ~UserInterface();

private slots:
    void closeProfile();
    void profileLoaded();
    void changeFolder_clicked();
    void profileButton_clicked();
    void on_cmdReload_clicked();
    void on_actionExit_triggered();
    void on_actionSelect_profile_triggered();
    void on_actionAbout_gta5sync_triggered();
    void on_actionSelect_all_triggered();
    void on_actionDeselect_all_triggered();
    void on_actionExport_selected_triggered();
    void on_actionDelete_selected_triggered();
    void on_actionOptions_triggered();
    void on_action_Import_triggered();
    void on_actionOpen_File_triggered();
    void on_actionSelect_GTA_Folder_triggered();
    void on_action_Enable_In_game_triggered();
    void on_action_Disable_In_game_triggered();
    void settingsApplied(int contentMode, bool languageChanged);

protected:
    void closeEvent(QCloseEvent *ev);

private:
    ProfileDatabase *profileDB;
    CrewDatabase *crewDB;
    DatabaseThread *threadDB;
    Ui::UserInterface *ui;
    ProfileInterface *profileUI;
    QList<QPushButton*> profileBtns;
    QString profileName;
    bool profileOpen;
    int contentMode;
    QString language;
    QString defaultWindowTitle;
    QString GTAV_Folder;
    QString GTAV_ProfilesFolder;
    QStringList GTAV_Profiles;
    void setupProfileUi();
    void openProfile(const QString &profileName);
    void openSelectProfile();
    void retranslateUi();

    // Open File
    bool openFile(QString selectedFile, bool warn = true);
    void openSavegameFile(SavegameData *savegame);
    void openSnapmaticFile(SnapmaticPicture *picture);
};

#endif // USERINTERFACE_H
