/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping Gaming Team
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/

#include <QDesktopServices>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include "frmGTA5Sync.h"
#include "ui_frmGTA5Sync.h"

frmGTA5Sync::frmGTA5Sync(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmGTA5Sync)
{
    ui->setupUi(this);

    // init settings
    QSettings SyncSettings("Syping Gaming Team", "gta5sync");
    SyncSettings.beginGroup("dir");
    bool forceDir = SyncSettings.value("force", false).toBool();

    // init folder
    QString GTAV_defaultFolder = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "\\Rockstar Games\\GTA V";
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
        qDebug() << "GTA V folder not found";
    }

    // profiles init
    QDir GTAV_ProfilesDir;
    GTAV_ProfilesFolder = GTAV_Folder + "\\Profiles";
    GTAV_ProfilesDir.setPath(GTAV_ProfilesFolder);

    QStringList GTAV_Profiles = GTAV_ProfilesDir.entryList(QDir::NoFilter, QDir::NoSort);
    GTAV_Profiles.removeAll("..");
    GTAV_Profiles.removeAll(".");

    foreach(QString GTAV_Profile, GTAV_Profiles)
    {
        ui->cbProfile->addItem(GTAV_Profile);
    }



}

frmGTA5Sync::~frmGTA5Sync()
{
    delete ui;
}
