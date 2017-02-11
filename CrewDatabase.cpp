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

#include "StandardPaths.h"
#include "CrewDatabase.h"
#include "config.h"
#include <QFile>
#include <QDir>

CrewDatabase::CrewDatabase(QObject *parent) : QObject(parent)
{
    QDir dir;
    dir.mkpath(StandardPaths::dataLocation());
    dir.setPath(StandardPaths::dataLocation());
    QString dirPath = dir.absolutePath();
    QString defaultConfPath = dirPath + QDir::separator() + "crews.ini";

    QSettings confPathSettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    confPathSettings.beginGroup("Database");
    QString confPathFile = confPathSettings.value("Crews", defaultConfPath).toString();
    confPathSettings.endGroup();

    crewDB = new QSettings(confPathFile, QSettings::IniFormat);
    crewDB->beginGroup("Crews");
}

CrewDatabase::~CrewDatabase()
{
    crewDB->endGroup();
    delete crewDB;
}

QStringList CrewDatabase::getCrews()
{
    QStringList compatibleCrewList = crewDB->childKeys();
    crewDB->endGroup();
    crewDB->beginGroup("CrewList");
    QStringList crewIDs = crewDB->value("IDs", QStringList()).toStringList();
    crewIDs.append(compatibleCrewList);
    crewIDs.removeDuplicates();
    crewDB->endGroup();
    crewDB->beginGroup("Crews");
    return crewIDs;
}

QString CrewDatabase::getCrewName(int crewID)
{
    return crewDB->value(QString::number(crewID), crewID).toString();
}

void CrewDatabase::setCrewName(int crewID, QString crewName)
{
    crewDB->setValue(QString::number(crewID), crewName);
}

void CrewDatabase::addCrew(int crewID)
{
    QStringList crews = getCrews();
    crews.append(QString::number(crewID));
    crews.removeDuplicates();
    crewDB->endGroup();
    crewDB->beginGroup("CrewList");
    crewDB->setValue("IDs", crews);
    crewDB->endGroup();
    crewDB->beginGroup("Crews");
}
