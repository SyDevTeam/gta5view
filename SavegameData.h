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

#ifndef SAVEGAMEDATA_H
#define SAVEGAMEDATA_H

#include <QObject>

class SavegameData : public QObject
{
    Q_OBJECT
public:
    explicit SavegameData(const QString &fileName = "", QObject *parent = 0);
    bool readingSavegameFromFile(const QString &fileName);
    bool readingSavegame();
    bool isSavegameOk();
    QString getLastStep();
    QString getSavegameStr();
    QString getSavegameFileName();
    void setSavegameFileName(QString savegameFileName);

private:
    QString getSavegameDataString(const QByteArray &savegameHeader);
    QString savegameFileName;
    QString savegameStr;
    QString lastStep;
    bool savegameOk;
};

#endif // SAVEGAMEDATA_H
