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

// DONT USE THE SYNC FRAMEWORK NOW

#ifndef QT5_MODE

#ifndef SYNCFRAMEWORK_H
#define SYNCFRAMEWORK_H

#include <QObject>
#include <QString>

class SyncFramework : public QObject
{
    Q_OBJECT
public:
    explicit SyncFramework(QObject *parent = 0);
    void setPort(int port);
    void setHost(QString hostname);
    void setUsername(QString username);
    void setPassword(QString password);
    void setSyncFolder(QString folder);
    void testServer();

private:
    int serverPort;
    QString serverHostname;
    QString serverUsername;
    QString serverPassword;
    QString serverSyncFolder;

private slots:
    void fileDownloaded(bool isDone);
    void fileUploaded(bool isDone);

};

#endif
#endif // SYNCFRAMEWORK_H
