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

#include "ProfileInterface.h"
#include "ui_ProfileInterface.h"
#include "SnapmaticWidget.h"
#include "DatabaseThread.h"
#include "SavegameWidget.h"
#include "StandardPaths.h"
#include "ProfileLoader.h"
#include <QSpacerItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPalette>
#include <QRegExp>
#include <QDebug>
#include <QColor>
#include <QFile>
#include <QUrl>
#include <QDir>

ProfileInterface::ProfileInterface(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent) :
    QWidget(parent), profileDB(profileDB), crewDB(crewDB), threadDB(threadDB),
    ui(new Ui::ProfileInterface)
{
    ui->setupUi(this);
    ui->cmdImport->setEnabled(false);
    ui->cmdCloseProfile->setEnabled(false);
    loadingStr = ui->labProfileLoading->text();
    profileFolder = "";
    profileLoader = 0;
    saSpacerItem = 0;

    QPalette palette;
    QColor baseColor = palette.base().color();
    ui->saProfile->setStyleSheet("QWidget#saProfileContent{background-color: rgb(" + QString::number(baseColor.red()) + "," + QString::number(baseColor.green()) + "," + QString::number(baseColor.blue()) + ")}");
}

ProfileInterface::~ProfileInterface()
{
    foreach(SavegameData *savegame, savegames)
    {
        savegame->deleteLater();
        delete savegame;
    }
    foreach(SnapmaticPicture *picture, pictures)
    {
        pictures.removeAll(picture);
        picture->deleteLater();
        delete picture;
    }
    foreach(QWidget *widget, widgets)
    {
        widgets.removeAll(widget);
        widget->deleteLater();
        delete widget;
    }
    profileLoader->deleteLater();
    delete profileLoader;
    delete ui;
}

void ProfileInterface::setProfileFolder(QString folder, QString profile)
{
    profileFolder = folder;
    profileName = profile;
}

void ProfileInterface::setupProfileInterface()
{
    ui->labProfileLoading->setText(tr("Loading..."));
    profileLoader = new ProfileLoader(profileFolder, crewDB);
    QObject::connect(profileLoader, SIGNAL(savegameLoaded(SavegameData*, QString)), this, SLOT(on_savegameLoaded(SavegameData*, QString)));
    QObject::connect(profileLoader, SIGNAL(pictureLoaded(SnapmaticPicture*, QString)), this, SLOT(on_pictureLoaded(SnapmaticPicture*, QString)));
    QObject::connect(profileLoader, SIGNAL(loadingProgress(int,int)), this, SLOT(on_loadingProgress(int,int)));
    QObject::connect(profileLoader, SIGNAL(finished()), this, SLOT(on_profileLoaded()));
    profileLoader->start();
}

void ProfileInterface::on_savegameLoaded(SavegameData *savegame, QString savegamePath)
{
    SavegameWidget *sgdWidget = new SavegameWidget();
    sgdWidget->setSavegameData(savegame, savegamePath);
    ui->vlSavegame->addWidget(sgdWidget);
    widgets.append(sgdWidget);
    savegames.append(savegame);
    QObject::connect(sgdWidget, SIGNAL(savegameDeleted()), this, SLOT(on_savegameDeleted()));
}

void ProfileInterface::on_pictureLoaded(SnapmaticPicture *picture, QString picturePath)
{
    SnapmaticWidget *picWidget = new SnapmaticWidget(profileDB, threadDB);
    picWidget->setSnapmaticPicture(picture, picturePath);
    ui->vlSnapmatic->addWidget(picWidget);
    widgets.append(picWidget);
    pictures.append(picture);
    QObject::connect(picWidget, SIGNAL(pictureDeleted()), this, SLOT(on_pictureDeleted()));
}

void ProfileInterface::on_loadingProgress(int value, int maximum)
{
    ui->pbPictureLoading->setMaximum(maximum);
    ui->pbPictureLoading->setValue(value);
    ui->labProfileLoading->setText(loadingStr.arg(QString::number(value), QString::number(maximum)));
}

void ProfileInterface::on_profileLoaded()
{
    saSpacerItem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->saProfileContent->layout()->addItem(saSpacerItem);
    ui->swProfile->setCurrentWidget(ui->pageProfile);
    ui->cmdCloseProfile->setEnabled(true);
    ui->cmdImport->setEnabled(true);
}

void ProfileInterface::on_savegameDeleted()
{
    SavegameWidget *sgdWidget = (SavegameWidget*)sender();
    widgets.removeAll(sgdWidget);
    sgdWidget->deleteLater();
    delete sgdWidget;
}

void ProfileInterface::on_pictureDeleted()
{
    SnapmaticWidget *picWidget = (SnapmaticWidget*)sender();
    widgets.removeAll(picWidget);
    picWidget->deleteLater();
    delete picWidget;
}

void ProfileInterface::on_cmdCloseProfile_clicked()
{
    emit profileClosed();
}

void ProfileInterface::on_cmdImport_clicked()
{
    QSettings settings("Syping", "gta5sync");
    settings.beginGroup("FileDialogs");

    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setViewMode(QFileDialog::Detail);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
    fileDialog.setWindowTitle(tr("Import copy"));
    fileDialog.setWindowFlags(fileDialog.windowFlags()^Qt::WindowContextHelpButtonHint);

    QStringList filters;
    filters << tr("All profile files (SGTA* PGTA*)");
    filters << tr("Savegames files (SGTA*)");
    filters << tr("Snapmatic pictures (PGTA*)");
    filters << tr("All files (**)");
    fileDialog.setNameFilters(filters);

    QList<QUrl> sidebarUrls = fileDialog.sidebarUrls();
    QDir dir;

    // Get Documents + Desktop Location
    QString documentsLocation = StandardPaths::documentsLocation();
    QString desktopLocation = StandardPaths::desktopLocation();

    // Add Desktop Location to Sidebar
    dir.setPath(desktopLocation);
    if (dir.exists())
    {
        sidebarUrls.append(QUrl::fromLocalFile(dir.absolutePath()));
    }

    // Add Documents + GTA V Location to Sidebar
    dir.setPath(documentsLocation);
    if (dir.exists())
    {
        sidebarUrls.append(QUrl::fromLocalFile(dir.absolutePath()));
        if (dir.cd("Rockstar Games/GTA V"))
        {
            sidebarUrls.append(QUrl::fromLocalFile(dir.absolutePath()));
        }
    }

    fileDialog.setSidebarUrls(sidebarUrls);
    fileDialog.restoreState(settings.value("ImportCopy","").toByteArray());

fileDialogPreOpen:
    if (fileDialog.exec())
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        if (selectedFiles.length() == 1)
        {
            QString selectedFile = selectedFiles.at(0);
            QFileInfo selectedFileInfo(selectedFile);
            QString selectedFileName = selectedFileInfo.fileName();
            if (QFile::exists(selectedFile))
            {
                if (selectedFileName.left(4) == "PGTA")
                {
                    SnapmaticPicture *picture = new SnapmaticPicture(selectedFile);
                    if (picture->readingPicture())
                    {
                        importSnapmaticPicture(picture, selectedFile);
                    }
                    else
                    {
                        QMessageBox::warning(this, tr("Import copy"), tr("Failed to read Snapmatic picture"));
                        goto fileDialogPreOpen;
                    }
                }
                else if (selectedFileName.left(4) == "SGTA")
                {
                    SavegameData *savegame = new SavegameData(selectedFile);
                    if (savegame->readingSavegame())
                    {
                        importSavegameData(savegame, selectedFile);
                    }
                    else
                    {
                        QMessageBox::warning(this, tr("Import copy"), tr("Failed to read Savegame file"));
                        goto fileDialogPreOpen;
                    }
                }
            }
            else
            {
                QMessageBox::warning(this, tr("Import copy"), tr("No valid file is selected"));
                goto fileDialogPreOpen;
            }
        }
        else
        {
            QMessageBox::warning(this, tr("Import copy"), tr("No valid file is selected"));
            goto fileDialogPreOpen;
        }
    }

    settings.setValue("ImportCopy", fileDialog.saveState());
    settings.endGroup();
}

bool ProfileInterface::importSnapmaticPicture(SnapmaticPicture *picture, QString picPath)
{
    QFileInfo picFileInfo(picPath);
    QString picFileName = picFileInfo.fileName();
    if (picFileName.left(4) != "PGTA")
    {
        QMessageBox::warning(this, tr("Import copy"), tr("Failed to import copy of Snapmatic picture because the file not begin with PGTA"));
        return false;
    }
    else if (QFile::copy(picPath, profileFolder + "/" + picFileName))
    {
        on_pictureLoaded(picture, profileFolder + "/" + picFileName);
        return true;
    }
    else
    {
        QMessageBox::warning(this, tr("Import copy"), tr("Failed to import copy of Snapmatic picture because the copy failed"));
        return false;
    }
}

bool ProfileInterface::importSavegameData(SavegameData *savegame, QString sgdPath)
{
    QString sgdFileName;
    bool foundFree = 0;
    int currentSgd = 0;

    while (currentSgd < 15 && !foundFree)
    {
        QString sgdNumber = QString::number(currentSgd);
        if (sgdNumber.length() == 1)
        {
            sgdNumber.insert(0, "0");
        }
        sgdFileName = "SGTA00" + sgdNumber;

        if (!QFile::exists(profileFolder + "/" + sgdFileName))
        {
            foundFree = true;
        }
        currentSgd++;
    }

    if (foundFree)
    {
        if (QFile::copy(sgdPath, profileFolder + "/" + sgdFileName))
        {
            on_savegameLoaded(savegame, profileFolder + "/" + sgdFileName);
            return true;
        }
        else
        {
            QMessageBox::warning(this, tr("Import copy"), tr("Failed to import copy of Savegame file because the copy failed"));
            return false;
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Import copy"), tr("Failed to import copy of Savegame file because no free Savegame slot left"));
        return false;
    }
}
