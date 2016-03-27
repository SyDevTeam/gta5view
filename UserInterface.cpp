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

#include "UserInterface.h"
#include "ui_UserInterface.h"
#include "ProfileInterface.h"
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QPushButton>
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

UserInterface::UserInterface(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent) :
    QMainWindow(parent), profileDB(profileDB), crewDB(crewDB), threadDB(threadDB),
    ui(new Ui::UserInterface)
{
    ui->setupUi(this);
    profileOpen = 0;
    profileUI = 0;

    // init settings
    QSettings SyncSettings("Syping", "gta5sync");
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
        QMessageBox::warning(this, tr("gta5sync"), tr("GTA V Folder not found!"));
    }
    SyncSettings.endGroup();

    // profiles init
    SyncSettings.beginGroup("Profile");
    QString defaultProfile = SyncSettings.value("Default", "").toString();
    QDir GTAV_ProfilesDir;
    GTAV_ProfilesFolder = GTAV_Folder + "/Profiles";
    GTAV_ProfilesDir.setPath(GTAV_ProfilesFolder);

    QStringList GTAV_Profiles = GTAV_ProfilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
    setupProfileUi(GTAV_Profiles);

    if (GTAV_Profiles.length() == 1)
    {
        openProfile(GTAV_Profiles.at(0));
    }
    else if(GTAV_Profiles.contains(defaultProfile))
    {
        openProfile(defaultProfile);
    }
    SyncSettings.endGroup();
}

void UserInterface::setupProfileUi(QStringList GTAV_Profiles)
{
    foreach(const QString &GTAV_Profile, GTAV_Profiles)
    {
        QPushButton *profileBtn = new QPushButton(GTAV_Profile, ui->swSelection);
        profileBtn->setObjectName(GTAV_Profile);
        profileBtn->setMinimumSize(0, 40);
        profileBtn->setAutoDefault(true);
        ui->swSelection->layout()->addWidget(profileBtn);

        QObject::connect(profileBtn, SIGNAL(clicked(bool)), this, SLOT(on_profileButton_clicked()));
    }
    QSpacerItem *buttomSpacerItem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->swSelection->layout()->addItem(buttomSpacerItem);

    QHBoxLayout *footerLayout = new QHBoxLayout();
    footerLayout->setObjectName("footerLayout");
    ui->swSelection->layout()->addItem(footerLayout);

    QSpacerItem *closeButtonSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    footerLayout->addSpacerItem(closeButtonSpacer);

    QPushButton *cmdClose = new QPushButton(tr("Close"), ui->swSelection);
    cmdClose->setObjectName("cmdClose");
    cmdClose->setAutoDefault(true);
    footerLayout->addWidget(cmdClose);

    QObject::connect(cmdClose, SIGNAL(clicked(bool)), this, SLOT(close()));
}

void UserInterface::on_profileButton_clicked()
{
    QPushButton *profileBtn = (QPushButton*)sender();
    openProfile(profileBtn->objectName());
}

void UserInterface::openProfile(QString profileName)
{
    profileOpen = true;
    profileUI = new ProfileInterface(profileDB, crewDB, threadDB);
    ui->swProfile->addWidget(profileUI);
    ui->swProfile->setCurrentWidget(profileUI);
    profileUI->setProfileFolder(GTAV_ProfilesFolder + "/" + profileName, profileName);
    profileUI->setupProfileInterface();
    QObject::connect(profileUI, SIGNAL(profileClosed()), this, SLOT(closeProfile()));
}

void UserInterface::closeProfile()
{
    if (profileOpen)
    {
        profileOpen = false;
        ui->swProfile->removeWidget(profileUI);
        profileUI->deleteLater();
        delete profileUI;
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

void UserInterface::on_actionSelect_profile_triggered()
{
    closeProfile();
    openSelectProfile();
}

void UserInterface::openSelectProfile()
{
    // not needed right now
}
