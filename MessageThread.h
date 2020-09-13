/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2020 Syping
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

#ifndef MESSAGETHREAD_H
#define MESSAGETHREAD_H

#include <QJsonObject>
#include <QObject>
#include <QThread>

class MessageThread : public QThread
{
    Q_OBJECT
public:
    explicit MessageThread(uint cacheId, QObject *parent = 0);

public slots:
    void terminateThread();

private:
    bool threadRunning;
    uint cacheId;

protected:
    void run();

signals:
    void messagesArrived(const QJsonObject &messageObject);
    void updateCacheId(uint cacheId);
    void threadTerminated();
};

#endif // MESSAGETHREAD_H
