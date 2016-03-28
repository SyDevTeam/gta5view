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
#include "ProfileLoader.h"
#include <QSpacerItem>
#include <QFileInfo>
#include <QPalette>
#include <QRegExp>
#include <QDebug>
#include <QColor>
#include <QFile>
#include <QDir>

ProfileInterface::ProfileInterface(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent) :
    QWidget(parent), profileDB(profileDB), crewDB(crewDB), threadDB(threadDB),
    ui(new Ui::ProfileInterface)
{
    ui->setupUi(this);
    ui->cmdCloseProfile->setEnabled(false);
    loadingStr = ui->labProfileLoading->text();
    profileFolder = "";
    profileLoader = 0;

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
    QSpacerItem *saSpacerItem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->saProfileContent->layout()->addItem(saSpacerItem);
    ui->swProfile->setCurrentWidget(ui->pageProfile);
    ui->cmdCloseProfile->setEnabled(true);
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
