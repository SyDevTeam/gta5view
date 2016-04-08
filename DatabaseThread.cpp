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
    crewMaxPages = 83;
}

void DatabaseThread::run()
{
    QEventLoop threadLoop;
    QStringList crewList;

    // Quick time scan
    if (crewList.length() <= 3)
    {
        scanCrewMembersList(crewList, 3, 1000);
        emit playerNameUpdated();
    }
    else if (crewList.length() <= 5)
    {
        scanCrewMembersList(crewList, 2, 1000);
        emit playerNameUpdated();
    }

    QEventLoop waitingLoop;
    QTimer::singleShot(10000, &waitingLoop, SLOT(quit()));
    waitingLoop.exec();

dbtBegin:
    crewList = crewDB->getCrews();

    // Long time scan
    scanCrewMembersList(crewList, crewMaxPages, 10000);
    emit playerNameUpdated();

    QTimer::singleShot(300000, &threadLoop, SLOT(quit()));
    threadLoop.exec();
    goto dbtBegin;
}

void DatabaseThread::scanCrewMembersList(QStringList crewList, int maxPages, int requestDelay)
{
    foreach (const QString &crewID, crewList)
    {
        if (crewID != "0")
        {
            int currentPage = 0;
            int foundPlayers = 0;
            int totalPlayers = 1000;

            while(foundPlayers < totalPlayers && currentPage < maxPages)
            {
                QNetworkAccessManager *netManager = new QNetworkAccessManager();

                QString memberListUrl = "https://socialclub.rockstargames.com/crewsapi/GetMembersList?crewId=" + crewID + "&pageNumber=" + QString::number(currentPage);

                QNetworkRequest netRequest(memberListUrl);
                netRequest.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:45.0) Gecko/20100101 Firefox/45.0 gta5sync/1.0");
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

                    if (crewMap.contains("Total")) { totalPlayers = crewMap["Total"].toInt(); }

                    if (crewMap.contains("Members"))
                    {
                        QList<QVariant> memberList = crewMap["Members"].toList();
                        foreach (const QVariant &memberVariant, memberList)
                        {
                            QMap<QString, QVariant> memberMap = memberVariant.toMap();
                            foundPlayers++;
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

                    QEventLoop waitingLoop;
                    QTimer::singleShot(requestDelay, &waitingLoop, SLOT(quit()));
                    waitingLoop.exec();

                    currentPage++;
                }

                netReply->deleteLater();
                delete netReply;
                netManager->deleteLater();
                delete netManager;
            }
        }
    }
}
