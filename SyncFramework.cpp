/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping Gaming Team
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

#include "SyncFramework.h"
#include <QEventLoop>
#include <QFtp>

SyncFramework::SyncFramework(QObject *parent) : QObject(parent)
{
    // INIT DEFAULT
    serverPort = 21;
    serverHostname = "";
    serverUsername = "";
    serverPassword = "";
    serverSyncFolder = "gta5sync";
}

void SyncFramework::testServer()
{
    QEventLoop ftpLoop;
    QFtp *ftpConnection = new QFtp();
    connect(ftpConnection, SIGNAL(done(bool)), &ftpLoop, SLOT(quit()));
    ftpConnection->connectToHost(serverHostname, serverPort);
    if (serverUsername != "")
    {
        ftpConnection->login(serverHostname, serverPassword);
    }
    else
    {
        ftpConnection->login();
    }
    ftpConnection->close();
    ftpLoop.exec();
}

void SyncFramework::setPort(int port)
{
    serverPort = port;
}

void SyncFramework::setHost(QString hostname)
{
    serverHostname = hostname;
}

void SyncFramework::setUsername(QString username)
{
    serverUsername = username;
}

void SyncFramework::setPassword(QString password)
{
    serverPassword = password;
}

void SyncFramework::setSyncFolder(QString folder)
{
    serverSyncFolder = folder;
}

void SyncFramework::fileDownloaded(bool isDone)
{
    Q_UNUSED(isDone)
}

void SyncFramework::fileUploaded(bool isDone)
{
    Q_UNUSED(isDone)
}

#endif
