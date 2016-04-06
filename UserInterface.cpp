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
#include "StandardPaths.h"
#include "AboutDialog.h"
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMap>

UserInterface::UserInterface(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent) :
    QMainWindow(parent), profileDB(profileDB), crewDB(crewDB), threadDB(threadDB),
    ui(new Ui::UserInterface)
{
    ui->setupUi(this);
    profileOpen = 0;
    profileUI = 0;
    ui->menuProfile->setEnabled(false);
    defaultWindowTitle = this->windowTitle();

    this->setWindowTitle(defaultWindowTitle.arg(tr("Select profile")));

    // init settings
    QSettings SyncSettings("Syping", "gta5sync");
    SyncSettings.beginGroup("dir");
    bool forceDir = SyncSettings.value("force", false).toBool();

    // init folder
    QString GTAV_defaultFolder = StandardPaths::documentsLocation() + "/Rockstar Games/GTA V";
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
        QMessageBox::warning(this, tr("gta5sync"), tr("Grand Theft Auto V Folder not found!"));
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
    if (GTAV_Profiles.length() == 0)
    {
        QPushButton *reloadBtn = new QPushButton(tr("Reload"), ui->swSelection);
        reloadBtn->setObjectName("Reload");
        reloadBtn->setAutoDefault(true);
        ui->swSelection->layout()->addWidget(reloadBtn);

        QObject::connect(reloadBtn, SIGNAL(clicked(bool)), this, SLOT(reloadProfiles_clicked()));
    }
    else foreach(const QString &GTAV_Profile, GTAV_Profiles)
    {
        QPushButton *profileBtn = new QPushButton(GTAV_Profile, ui->swSelection);
        profileBtn->setObjectName(GTAV_Profile);
        profileBtn->setMinimumSize(0, 40);
        profileBtn->setAutoDefault(true);
        ui->swSelection->layout()->addWidget(profileBtn);

        QObject::connect(profileBtn, SIGNAL(clicked(bool)), this, SLOT(profileButton_clicked()));
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

void UserInterface::reloadProfiles_clicked()
{
    QStringList gta5sync_a = qApp->arguments();
    if (gta5sync_a.length() >= 1)
    {
        QProcess gta5sync_p;
        QString gta5sync_exe = gta5sync_a.at(0);
        gta5sync_a.removeAt(0);
        gta5sync_p.startDetached(gta5sync_exe, gta5sync_a);
    }
    else
    {
        QMessageBox::warning(this, tr("Reload profiles"), tr("Not able to reload profiles"));
    }
}

void UserInterface::profileButton_clicked()
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
    QObject::connect(profileUI, SIGNAL(profileLoaded()), this, SLOT(profileLoaded()));
    this->setWindowTitle(defaultWindowTitle.arg(profileName));
}

void UserInterface::closeProfile()
{
    if (profileOpen)
    {
        profileOpen = false;
        ui->menuProfile->setEnabled(false);
        ui->swProfile->removeWidget(profileUI);
        profileUI->deleteLater();
        delete profileUI;
    }
    this->setWindowTitle(defaultWindowTitle.arg(tr("Select profile")));
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

void UserInterface::on_actionAbout_gta5sync_triggered()
{
    AboutDialog *aboutDialog = new AboutDialog(this);
    aboutDialog->setWindowFlags(aboutDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
    aboutDialog->setModal(true);
    aboutDialog->show();
    aboutDialog->exec();
    aboutDialog->deleteLater();
    delete aboutDialog;
}

void UserInterface::profileLoaded()
{
    ui->menuProfile->setEnabled(true);
}

void UserInterface::on_actionSelect_all_triggered()
{
    profileUI->selectAllWidgets();
}

void UserInterface::on_actionDeselect_all_triggered()
{
    profileUI->deselectAllWidgets();
}

void UserInterface::on_actionExport_selected_triggered()
{
    profileUI->exportSelected();
}

void UserInterface::on_actionDelete_selected_triggered()
{
    profileUI->deleteSelected();
}
