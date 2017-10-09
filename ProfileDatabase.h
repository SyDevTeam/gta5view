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

#ifndef PROFILEDATABASE_H
#define PROFILEDATABASE_H

#include <QSettings>
#include <QObject>
#include <QMutex>
#include <QMap>

class ProfileDatabase : public QObject
{
    Q_OBJECT
public:
    explicit ProfileDatabase(QObject *parent = 0);
    QString getPlayerName(int playerID);
    QStringList getPlayers();
    ~ProfileDatabase();

private:
    mutable QMutex mutex;
    QSettings *profileDB;

public slots:
    void setPlayerName(int playerID, QString playerName);

};

#endif // PROFILEDATABASE_H
