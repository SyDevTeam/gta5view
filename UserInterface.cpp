/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping Gaming Team
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

#include "UserInterface.h"
#include "ui_UserInterface.h"
#include "ProfileInterface.h"
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMap>

#ifdef QT5_MODE
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

UserInterface::UserInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UserInterface)
{
    ui->setupUi(this);

    // init settings
    QSettings SyncSettings("Syping Gaming Team", "gta5sync");
    SyncSettings.beginGroup("dir");
    bool forceDir = SyncSettings.value("force", false).toBool();

    // init folder
#ifdef QT5_MODE
    QString GTAV_defaultFolder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Rockstar Games/GTA V";
#else
    QString GTAV_defaultFolder = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Rockstar Games/GTA V";
#endif
    QDir GTAV_Dir;
    if (forceDir)
    {
        GTAV_Folder = SyncSettings.value("dir", GTAV_defaultFolder).toString();
    }
    else
    {
        GTAV_Folder = GTAV_defaultFolder;
    }
    GTAV_Dir.setPath(GTAV_Folder);
    if (GTAV_Dir.exists())
    {
        QDir::setCurrent(GTAV_Folder);
    }
    else
    {
        QMessageBox::warning(this, tr("GTA V Sync"), tr("GTA V Folder not found!"));
    }

    // profiles init
    QDir GTAV_ProfilesDir;
    GTAV_ProfilesFolder = GTAV_Folder + "/Profiles";
    GTAV_ProfilesDir.setPath(GTAV_ProfilesFolder);

    QStringList GTAV_Profiles = GTAV_ProfilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);

    if (GTAV_Profiles.length() >= 1)
    {
        QString profileName = GTAV_Profiles.at(0);
        ProfileInterface *profile1 = new ProfileInterface();
        ui->swProfile->addWidget(profile1);
        ui->swProfile->setCurrentWidget(profile1);
        profile1->setProfileFolder(GTAV_ProfilesFolder + "/" + profileName, profileName);
        profile1->setupProfileInterface();
    }
}

UserInterface::~UserInterface()
{
    delete ui;
}

void UserInterface::on_actionExit_triggered()
{
    this->close();
}
