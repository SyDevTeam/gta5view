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
#include "OptionsDialog.h"
#include "AboutDialog.h"
#include "IconLoader.h"
#include "AppEnv.h"
#include <QFileDialog>
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

    this->setWindowTitle(defaultWindowTitle.arg(tr("Select Profile")));
}

void UserInterface::setupDirEnv()
{
    bool folderExists;
    GTAV_Folder = AppEnv::getGameFolder(&folderExists);
    if (folderExists)
    {
        QDir::setCurrent(GTAV_Folder);
    }
    else
    {
        GTAV_Folder = QFileDialog::getExistingDirectory(this, tr("Select GTA V Folder..."), StandardPaths::documentsLocation(), QFileDialog::ShowDirsOnly);
        if (QFileInfo(GTAV_Folder).exists())
        {
            folderExists = true;
            QDir::setCurrent(GTAV_Folder);
            AppEnv::setGameFolder(GTAV_Folder);
        }
    }

    // profiles init
    QSettings SyncSettings("Syping", "gta5sync");
    SyncSettings.beginGroup("Profile");
    QString defaultProfile = SyncSettings.value("Default", "").toString();

    if (folderExists)
    {
        QDir GTAV_ProfilesDir;
        GTAV_ProfilesFolder = GTAV_Folder + QDir::separator() + "Profiles";
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
    }
    else
    {
        setupProfileUi(QStringList());
    }
}

void UserInterface::setupProfileUi(QStringList GTAV_Profiles)
{
    if (GTAV_Profiles.length() == 0)
    {
        QPushButton *changeDirBtn = new QPushButton(tr("Select GTA V &Folder..."), ui->swSelection);
        changeDirBtn->setObjectName("cmdChangeDir");
        changeDirBtn->setMinimumSize(0, 40);
        changeDirBtn->setAutoDefault(true);
        ui->vlButtons->addWidget(changeDirBtn);
        profileBtns.append(changeDirBtn);

        QObject::connect(changeDirBtn, SIGNAL(clicked(bool)), this, SLOT(changeFolder_clicked()));
    }
    else foreach(const QString &GTAV_Profile, GTAV_Profiles)
    {
        QPushButton *profileBtn = new QPushButton(GTAV_Profile, ui->swSelection);
        profileBtn->setObjectName(GTAV_Profile);
        profileBtn->setMinimumSize(0, 40);
        profileBtn->setAutoDefault(true);
        ui->vlButtons->addWidget(profileBtn);
        profileBtns.append(profileBtn);

        QObject::connect(profileBtn, SIGNAL(clicked(bool)), this, SLOT(profileButton_clicked()));
    }
    profileBtns.at(0)->setFocus();
}

void UserInterface::changeFolder_clicked()
{
    GTAV_Folder = QFileDialog::getExistingDirectory(this, tr("Select GTA V Folder..."), StandardPaths::documentsLocation(), QFileDialog::ShowDirsOnly);
    if (QFileInfo(GTAV_Folder).exists())
    {
        QDir::setCurrent(GTAV_Folder);
        AppEnv::setGameFolder(GTAV_Folder);
        on_cmdReload_clicked();
    }
}

void UserInterface::on_cmdReload_clicked()
{
    foreach(QPushButton *profileBtn, profileBtns)
    {
        ui->vlButtons->removeWidget(profileBtn);
        profileBtns.removeAll(profileBtn);
        delete profileBtn;
    }
    setupDirEnv();
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
    this->setWindowTitle(defaultWindowTitle.arg(tr("Select Profile")));
}

UserInterface::~UserInterface()
{
    foreach (QPushButton *profileBtn, profileBtns)
    {
        delete profileBtn;
    }
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

void UserInterface::on_actionOptions_triggered()
{
    OptionsDialog *optionsDialog = new OptionsDialog(this);
    optionsDialog->setWindowFlags(optionsDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
    optionsDialog->setModal(true);
    optionsDialog->show();
    optionsDialog->exec();
    optionsDialog->deleteLater();
    delete optionsDialog;
}
