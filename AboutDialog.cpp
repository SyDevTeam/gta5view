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

#include <QStringBuilder>
#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include "config.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    aboutStr = ui->labAbout->text();
    titleStr = this->windowTitle();
    QString appVersion = qApp->applicationVersion();
    QString buildType = GTA5SYNC_BUILDTYPE;
    buildType.replace("_", " ");
    QString projectBuild = QString("%1, %2").arg(__DATE__, __TIME__);
    QString buildStr = QString("%1, %2").arg(QT_VERSION_STR, GTA5SYNC_COMPILER);
#ifdef GTA5SYNC_ENABLED
     QString projectDes = tr("A project for viewing and sync Grand Theft Auto V Snapmatic<br/>\nPictures and Savegames");
#else
     QString projectDes = tr("A project for viewing Grand Theft Auto V Snapmatic<br/>\nPictures and Savegames");
#endif
    ui->labAbout->setText(aboutStr.arg(appVersion % " (" % buildType % ")", buildStr, qVersion(), projectBuild, GTA5SYNC_APPVENDORLINK, GTA5SYNC_APPVENDOR, GTA5SYNC_COPYRIGHT, GTA5SYNC_APPSTR, projectDes));
    this->setWindowTitle(titleStr.arg(GTA5SYNC_APPSTR));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
