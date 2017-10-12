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
#include <QStringBuilder>
#include <QMutexLocker>
#include <QDebug>
#include <QFile>
#include <QDir>

CrewDatabase::CrewDatabase(QObject *parent) : QObject(parent)
{
    QDir dir;
    dir.mkpath(StandardPaths::dataLocation());
    dir.setPath(StandardPaths::dataLocation());
    QString dirPath = dir.absolutePath();
    QString defaultConfPath = dirPath % "/crews.ini";

    QSettings confPathSettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    confPathSettings.beginGroup("Database");
    QString confPathFile = confPathSettings.value("Crews", defaultConfPath).toString();
    confPathSettings.endGroup();

    crewDB = new QSettings(confPathFile, QSettings::IniFormat);
    crewDB->beginGroup("Crews");

    addProcess = false;
}

CrewDatabase::~CrewDatabase()
{
    crewDB->endGroup();
    delete crewDB;
}

QStringList CrewDatabase::getCrews()
{
    QMutexLocker locker(&mutex);
#ifdef GTA5SYNC_DEBUG
    qDebug() << "getCrews";
#endif
    return getCrews_p();
}

QStringList CrewDatabase::getCrews_p()
{
#ifdef GTA5SYNC_DEBUG
    qDebug() << "getCrews_p";
#endif
    QStringList compatibleCrewList = getCompatibleCrews_p();
    crewDB->endGroup();
    crewDB->beginGroup("CrewList");
    QStringList crewIDs = crewDB->value("IDs", QStringList()).toStringList();
    crewIDs += compatibleCrewList;
    crewIDs.removeDuplicates();
    crewDB->endGroup();
    crewDB->beginGroup("Crews");
    return crewIDs;
}

QStringList CrewDatabase::getCompatibleCrews()
{
    QMutexLocker locker(&mutex);
#ifdef GTA5SYNC_DEBUG
    qDebug() << "getCompatibleCrews";
#endif
    return getCompatibleCrews_p();
}

QStringList CrewDatabase::getCompatibleCrews_p()
{
#ifdef GTA5SYNC_DEBUG
    qDebug() << "getCompatibleCrews_p";
#endif
    return crewDB->childKeys();
}

QString CrewDatabase::getCrewName(int crewID)
{
    QMutexLocker locker(&mutex);
#ifdef GTA5SYNC_DEBUG
    qDebug() << "getCrewName" << crewID;
#endif
    QString crewStr = crewDB->value(QString::number(crewID), crewID).toString();
    if (crewID == 0) crewStr = tr("No Crew", "");
    return crewStr;
}

void CrewDatabase::setCrewName(int crewID, QString crewName)
{
    QMutexLocker locker(&mutex);
#ifdef GTA5SYNC_DEBUG
    qDebug() << "setCrewName" << crewID << crewName;
#endif
    crewDB->setValue(QString::number(crewID), crewName);
}

void CrewDatabase::addCrew(int crewID)
{
    QMutexLocker locker(&mutex);
#ifdef GTA5SYNC_DEBUG
    qDebug() << "addCrew" << crewID;
#endif
    QStringList crews = getCrews_p();
    crews += QString::number(crewID);
    crews.removeDuplicates();
    crewDB->endGroup();
    crewDB->beginGroup("CrewList");
    crewDB->setValue("IDs", crews);
    crewDB->endGroup();
    crewDB->beginGroup("Crews");
}

bool CrewDatabase::isCompatibleCrew(QString crewNID)
{
    QMutexLocker locker(&mutex);
#ifdef GTA5SYNC_DEBUG
    qDebug() << "isCompatibleCrew" << crewNID;
#endif
    return crewDB->contains(crewNID);
}

bool CrewDatabase::isCompatibleCrew(int crewID)
{
    QMutexLocker locker(&mutex);
#ifdef GTA5SYNC_DEBUG
    qDebug() << "isCompatibleCrew" << crewID;
#endif
    return crewDB->contains(QString::number(crewID));
}

void CrewDatabase::setAddingCrews(bool addingCrews)
{
    QMutexLocker locker(&mutex);
#ifdef GTA5SYNC_DEBUG
    qDebug() << "setAddingCrews" << addingCrews;
#endif
    addProcess = addingCrews;
}

bool CrewDatabase::isAddingCrews()
{
    QMutexLocker locker(&mutex);
#ifdef GTA5SYNC_DEBUG
    qDebug() << "isAddingCrews";
#endif
    return addProcess;
}
