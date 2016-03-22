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

#include "DatabaseThread.h"
#include "CrewDatabase.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QVariantMap>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QUrl>

DatabaseThread::DatabaseThread(CrewDatabase *crewDB, QObject *parent) : QThread(parent), crewDB(crewDB)
{
}

void DatabaseThread::run()
{
    QNetworkAccessManager *netManager = new QNetworkAccessManager();
    QEventLoop threadLoop;
    dbtBegin:

    QStringList crewList = crewDB->getCrews();
    foreach (const QString &crewID, crewList)
    {
        QString memberListUrl = "http://socialclub.rockstargames.com/crewsapi/GetMembersList?crewId=" + crewID;

        QNetworkRequest netRequest(memberListUrl);
        netRequest.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.3; Win64; x64; rv:45.0) Gecko/20100101 Firefox/45.0");
        netRequest.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        netRequest.setRawHeader("Accept-Language", "en-US;q=0.5,en;q=0.3");
        netRequest.setRawHeader("Connection", "keep-alive");

        QNetworkReply *netReply = netManager->get(netRequest);

        QEventLoop downloadLoop;
        QObject::connect(netReply, SIGNAL(finished()), &downloadLoop, SLOT(quit()));
        QTimer::singleShot(30000, &downloadLoop, SLOT(quit()));
        downloadLoop.exec();

        if (netReply->isFinished())
        {
            QByteArray crewJson = netReply->readAll();
            QJsonDocument crewDocument = QJsonDocument::fromJson(crewJson);
            QJsonObject crewObject = crewDocument.object();
            QVariantMap crewMap = crewObject.toVariantMap();
            if (crewMap.contains("Members"))
            {
                QList<QVariant> memberList = crewMap["Members"].toList();
                foreach (const QVariant &memberVariant, memberList)
                {
                    QMap<QString, QVariant> memberMap = memberVariant.toMap();
                    if (memberMap.contains("RockstarId") && memberMap.contains("Name"))
                    {
                        int RockstarId = memberMap["RockstarId"].toInt();
                        QString memberName = memberMap["Name"].toString();
                        if (memberName != "" && RockstarId != 0)
                        {
                            emit playerNameFound(RockstarId, memberName);
                        }
                    }
                }
            }
        }
    }

    QTimer::singleShot(300000, &threadLoop, SLOT(quit()));
    threadLoop.exec();
    goto dbtBegin;
}
