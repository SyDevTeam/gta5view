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

#include "UserInterface.h"
#include "ui_UserInterface.h"
#include "ProfileInterface.h"
#include "SnapmaticPicture.h"
#include "SidebarGenerator.h"
#include "SavegameDialog.h"
#include "StandardPaths.h"
#include "OptionsDialog.h"
#include "PictureDialog.h"
#include "SavegameData.h"
#include "AboutDialog.h"
#include "IconLoader.h"
#include "AppEnv.h"
#include "config.h"
#include <QtGlobal>
#include <QStyleFactory>
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
    contentMode = 0;
    profileOpen = 0;
    profileUI = 0;
    ui->menuProfile->setEnabled(false);
    ui->actionSelect_profile->setEnabled(false);
    ui->actionAbout_gta5sync->setIcon(IconLoader::loadingAppIcon());
    ui->actionAbout_gta5sync->setText(tr("&About %1").arg(GTA5SYNC_APPSTR));
    defaultWindowTitle = tr("%2 - %1").arg("%1", GTA5SYNC_APPSTR);

    this->setWindowTitle(defaultWindowTitle.arg(tr("Select Profile")));
    ui->labVersion->setText(ui->labVersion->text().arg(GTA5SYNC_APPSTR, GTA5SYNC_APPVER));

    if (QIcon::hasThemeIcon("dialog-close"))
    {
        ui->cmdClose->setIcon(QIcon::fromTheme("dialog-close"));
    }
    if (QIcon::hasThemeIcon("preferences-system"))
    {
#ifndef Q_OS_MACOS // Setting icon for preferences/settings/options lead to a crash in Mac OS X
        ui->actionOptions->setIcon(QIcon::fromTheme("preferences-system"));
#endif
    }
    if (QIcon::hasThemeIcon("application-exit"))
    {
#ifndef Q_OS_MACOS // Setting icon for exit/quit lead to a crash in Mac OS X
        ui->actionExit->setIcon(QIcon::fromTheme("application-exit"));
#endif
    }

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    resize(625 * screenRatio, 500 * screenRatio);
    ui->vlUserInterface->setSpacing(6 * screenRatio);
    ui->vlUserInterface->setContentsMargins(9 * screenRatio, 9 * screenRatio, 9 * screenRatio, 9 * screenRatio);
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
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Profile");
    QString defaultProfile = settings.value("Default", "").toString();

    bool contentModeOk;
    contentMode = settings.value("ContentMode", 0).toInt(&contentModeOk);
    if (contentMode != 0 && contentMode != 1 && contentMode != 2)
    {
        contentMode = 0;
    }

    if (folderExists)
    {
        QDir GTAV_ProfilesDir;
        GTAV_ProfilesFolder = GTAV_Folder + QDir::separator() + "Profiles";
        GTAV_ProfilesDir.setPath(GTAV_ProfilesFolder);

        GTAV_Profiles = GTAV_ProfilesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
        setupProfileUi();

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
        GTAV_Profiles = QStringList();
        setupProfileUi();
    }
    settings.endGroup();
}

void UserInterface::setupProfileUi()
{
    qreal screenRatio = AppEnv::screenRatio();
    if (GTAV_Profiles.length() == 0)
    {
        QPushButton *changeDirBtn = new QPushButton(tr("Select &GTA V Folder..."), ui->swSelection);
        changeDirBtn->setObjectName("cmdChangeDir");
        changeDirBtn->setMinimumSize(0, 40 * screenRatio);
        changeDirBtn->setAutoDefault(true);
        ui->vlButtons->addWidget(changeDirBtn);
        profileBtns.append(changeDirBtn);

        QObject::connect(changeDirBtn, SIGNAL(clicked(bool)), this, SLOT(changeFolder_clicked()));
    }
    else foreach(const QString &GTAV_Profile, GTAV_Profiles)
    {
        QPushButton *profileBtn = new QPushButton(GTAV_Profile, ui->swSelection);
        profileBtn->setObjectName(GTAV_Profile);
        profileBtn->setMinimumSize(0, 40 * screenRatio);
        profileBtn->setAutoDefault(true);
        ui->vlButtons->addWidget(profileBtn);
        profileBtns.append(profileBtn);

        QObject::connect(profileBtn, SIGNAL(clicked(bool)), this, SLOT(profileButton_clicked()));
    }
    profileBtns.at(0)->setFocus();
}

void UserInterface::changeFolder_clicked()
{
    on_actionSelect_GTA_Folder_triggered();
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
    profileUI->setProfileFolder(GTAV_ProfilesFolder + QDir::separator() + profileName, profileName);
    profileUI->settingsApplied(contentMode, language);
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
        ui->actionSelect_profile->setEnabled(false);
        ui->swProfile->removeWidget(profileUI);
        delete profileUI;
    }
    this->setWindowTitle(defaultWindowTitle.arg(tr("Select Profile")));
}

void UserInterface::closeEvent(QCloseEvent *ev)
{
    Q_UNUSED(ev)
    threadDB->doEndThread();
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
    aboutDialog->setWindowIcon(windowIcon());
    aboutDialog->setModal(true);
#ifdef Q_OS_ANDROID
    // Android ...
    aboutDialog->showMaximized();
#else
    aboutDialog->show();
#endif
    aboutDialog->exec();
    delete aboutDialog;
}

void UserInterface::profileLoaded()
{
    ui->menuProfile->setEnabled(true);
    ui->actionSelect_profile->setEnabled(true);
}

void UserInterface::on_actionSelect_all_triggered()
{
    if (profileOpen)
    {
        profileUI->selectAllWidgets();
    }
}

void UserInterface::on_actionDeselect_all_triggered()
{
    if (profileOpen)
    {
        profileUI->deselectAllWidgets();
    }
}

void UserInterface::on_actionExport_selected_triggered()
{
    if (profileOpen)
    {
        profileUI->exportSelected();
    }
}

void UserInterface::on_actionDelete_selected_triggered()
{
    if (profileOpen)
    {
        profileUI->deleteSelected();
    }
}

void UserInterface::on_actionOptions_triggered()
{
    OptionsDialog *optionsDialog = new OptionsDialog(profileDB, this);
    optionsDialog->setWindowIcon(windowIcon());
    optionsDialog->commitProfiles(GTAV_Profiles);
    QObject::connect(optionsDialog, SIGNAL(settingsApplied(int, QString)), this, SLOT(settingsApplied(int, QString)));

    optionsDialog->setModal(true);
#ifdef Q_OS_ANDROID
    // Android ...
    optionsDialog->showMaximized();
#else
    optionsDialog->show();
#endif
    optionsDialog->exec();

    delete optionsDialog;
}

void UserInterface::on_action_Import_triggered()
{
    if (profileOpen)
    {
        profileUI->importFiles();
    }
}

void UserInterface::on_actionOpen_File_triggered()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("FileDialogs");

fileDialogPreOpen:
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);
    fileDialog.setWindowTitle(tr("Open File..."));

    QStringList filters;
    filters << ProfileInterface::tr("All profile files (*.g5e SGTA* PGTA*)");
    filters << ProfileInterface::tr("GTA V Export (*.g5e)");
    filters << ProfileInterface::tr("Savegames files (SGTA*)");
    filters << ProfileInterface::tr("Snapmatic pictures (PGTA*)");
    filters << ProfileInterface::tr("All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = SidebarGenerator::generateSidebarUrls(fileDialog.sidebarUrls());

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.setDirectory(settings.value("OpenDialogDirectory", StandardPaths::documentsLocation()).toString());
    fileDialog.restoreGeometry(settings.value("OpenDialogGeometry","").toByteArray());

    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);
            if (!openFile(selectedFile, true)) goto fileDialogPreOpen;
        }
    }

    settings.setValue("OpenDialogGeometry", fileDialog.saveGeometry());
    settings.setValue("OpenDialogDirectory", fileDialog.directory().absolutePath());
    settings.endGroup();
}

bool UserInterface::openFile(QString selectedFile, bool warn)
{
    QString selectedFileName = QFileInfo(selectedFile).fileName();
    if (QFile::exists(selectedFile))
    {
        if (selectedFileName.left(4) == "PGTA" || selectedFileName.right(4) == ".g5e")
        {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            if (picture->readingPicture())
            {
                openSnapmaticFile(picture);
                delete picture;
                return true;
            }
            else
            {
                if (warn) QMessageBox::warning(this, tr("Open File"), ProfileInterface::tr("Failed to read Snapmatic picture"));
                delete picture;
                return false;
            }
        }
        else if (selectedFileName.left(4) == "SGTA")
        {
            SavegameData *savegame = new SavegameData(selectedFile);
            if (savegame->readingSavegame())
            {
                openSavegameFile(savegame);
                delete savegame;
                return true;
            }
            else
            {
                if (warn) QMessageBox::warning(this, tr("Open File"), ProfileInterface::tr("Failed to read Savegame file"));
                delete savegame;
                return false;
            }
        }
        else
        {
            SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
            SavegameData *savegame = new SavegameData(selectedFile);
            if (picture->readingPicture())
            {
                delete savegame;
                openSnapmaticFile(picture);
                delete picture;
                return true;
            }
            else if (savegame->readingSavegame())
            {
                delete picture;
                openSavegameFile(savegame);
                delete savegame;
                return true;
            }
            else
            {
                delete savegame;
                delete picture;
                if (warn) QMessageBox::warning(this, tr("Open File"), tr("Can't open %1 because of not valid file format").arg("\""+selectedFileName+"\""));
                return false;
            }
        }
    }
    if (warn) QMessageBox::warning(this, tr("Open File"), ProfileInterface::tr("No valid file is selected"));
    return false;
}

void UserInterface::openSnapmaticFile(SnapmaticPicture *picture)
{
    PictureDialog picDialog(profileDB, crewDB, this);
    picDialog.setSnapmaticPicture(picture, true);
    picDialog.setModal(true);

    int crewID = picture->getSnapmaticProperties().crewID;
    if (crewID != 0) { crewDB->addCrew(crewID); }

    QObject::connect(threadDB, SIGNAL(playerNameFound(int, QString)), profileDB, SLOT(setPlayerName(int, QString)));
    QObject::connect(threadDB, SIGNAL(playerNameUpdated()), &picDialog, SLOT(playerNameUpdated()));

#ifdef Q_OS_ANDROID
    // Android optimization should be put here
    picDialog.showMaximized();
#else
    picDialog.show();
    picDialog.setMinimumSize(picDialog.size());
    picDialog.setMaximumSize(picDialog.size());
#endif

    picDialog.exec();
}

void UserInterface::openSavegameFile(SavegameData *savegame)
{
    SavegameDialog sgdDialog(this);
    sgdDialog.setSavegameData(savegame, savegame->getSavegameFileName(), true);
    sgdDialog.setModal(true);
#ifdef Q_OS_ANDROID
    // Android optimization should be put here
    sgdDialog.showMaximized();
#else
    sgdDialog.show();
#endif
    sgdDialog.exec();
}

void UserInterface::settingsApplied(int _contentMode, QString _language)
{
    language = _language;
    contentMode = _contentMode;
    if (profileOpen)
    {
        profileUI->settingsApplied(contentMode, language);
    }
}

void UserInterface::on_actionSelect_GTA_Folder_triggered()
{
    QString GTAV_Folder_Temp = QFileDialog::getExistingDirectory(this, tr("Select GTA V Folder..."), StandardPaths::documentsLocation(), QFileDialog::ShowDirsOnly);
    if (QFileInfo(GTAV_Folder_Temp).exists())
    {
        GTAV_Folder = GTAV_Folder_Temp;
        QDir::setCurrent(GTAV_Folder);
        AppEnv::setGameFolder(GTAV_Folder);
        on_cmdReload_clicked();
    }
}

void UserInterface::on_action_Enable_In_game_triggered()
{
    if (profileOpen)
    {
        profileUI->enableSelected();
    }
}

void UserInterface::on_action_Disable_In_game_triggered()
{
    if (profileOpen)
    {
        profileUI->disableSelected();
    }
}
