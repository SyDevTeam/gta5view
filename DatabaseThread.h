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

#ifndef DATABASETHREAD_H
#define DATABASETHREAD_H

#include "CrewDatabase.h"
#include <QObject>
#include <QThread>

class DatabaseThread : public QThread
{
    Q_OBJECT
public:
    explicit DatabaseThread(CrewDatabase *crewDB, QObject *parent = 0);

public slots:
    void doEndThread();

private:
    CrewDatabase *crewDB;
    void scanCrewMembersList(const QStringList &crewList, const int &maxPages, const int &requestDelay);
    void scanCrewReference(const QStringList &crewList, const int &requestDelay);
    void deleteCompatibleCrews(QStringList *crewList);
    QStringList deleteCompatibleCrews(const QStringList &crewList);
    bool threadRunning;
    int plyrPerReq;

protected:
    void run();

signals:
    void crewNameFound(int crewID, QString crewName);
    void crewNameUpdated();
    void playerNameFound(int playerID, QString playerName);
    void playerNameUpdated();
    void threadEndCommited();
};

#endif // DATABASETHREAD_H
