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

#include "CrewDatabase.h"
#include <QFile>
#include <QDir>

#ifdef QT5_MODE
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

CrewDatabase::CrewDatabase(QObject *parent) : QObject(parent)
{
    QDir dir;
#ifdef QT5_MODE
    dir.setPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#else
    dir.setPath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
#endif
    dir.mkdir("../gta5sync");
    QString dirPath = dir.absolutePath();
    QString defaultConfPath = dirPath + "/crews.ini";

    QSettings confPathSettings("Syping","gta5sync");
    confPathSettings.beginGroup("Database");
    QString confPathFile = confPathSettings.value("Location", defaultConfPath).toString();
    confPathSettings.endGroup();

    crewDB = new QSettings(confPathFile, QSettings::IniFormat);
    crewDB->beginGroup("Crews");
}

CrewDatabase::~CrewDatabase()
{
    crewDB->endGroup();
}

QStringList CrewDatabase::getCrews()
{
    return crewDB->childKeys();
}

void CrewDatabase::addCrew(int crewID)
{
    crewDB->setValue(QString::number(crewID), crewID);
}
