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

#ifndef CREWDATABASE_H
#define CREWDATABASE_H

#include <QSettings>
#include <QObject>
#include <QMutex>
#include <QMap>

class CrewDatabase : public QObject
{
    Q_OBJECT
public:
    explicit CrewDatabase(QObject *parent = 0);
    QString getCrewName(QString crewID);
    QString getCrewName(int crewID);
    QStringList getCompatibleCrews();
    QStringList getCrews();
    void setAddingCrews(bool addingCrews);
    bool isCompatibleCrew(QString crewNID);
    bool isCompatibleCrew(int crewID);
    bool isAddingCrews();
    ~CrewDatabase();

private:
    mutable QMutex mutex;
    bool addProcess;
    QSettings *crewDB;
    QStringList getCrews_p();
    QStringList getCompatibleCrews_p();

public slots:
    void setCrewName(int crewID, QString crewName);
    void addCrew(int crewID);
};

#endif // CREWDATABASE_H
