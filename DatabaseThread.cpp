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

#include "DatabaseThread.h"
#include "CrewDatabase.h"
#include "AppEnv.h"
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
    threadRunning = true;
}

void DatabaseThread::run()
{
    QEventLoop threadLoop;
    QStringList crewList;

    // Quick time scan
    if (crewList.length() <= 3)
    {
        scanCrewReference(crewList, 2500);
        scanCrewMembersList(crewList, 3, 2500);
        emit playerNameUpdated();
    }
    else if (crewList.length() <= 5)
    {
        scanCrewReference(crewList, 2500);
        scanCrewMembersList(crewList, 2, 2500);
        emit playerNameUpdated();
    }

    QEventLoop *waitingLoop = new QEventLoop();
    QTimer::singleShot(10000, waitingLoop, SLOT(quit()));
    waitingLoop->exec();
    delete waitingLoop;

    while (threadRunning)
    {
        crewList = crewDB->getCrews();

        // Long time scan
        scanCrewReference(crewList, 10000);
        scanCrewMembersList(crewList, crewMaxPages, 10000);
        emit playerNameUpdated();

        QTimer::singleShot(300000, &threadLoop, SLOT(quit()));
        threadLoop.exec();
    }
}

void DatabaseThread::scanCrewReference(QStringList crewList, int requestDelay)
{
    foreach (const QString &crewID, crewList)
    {
        if (crewID != "0")
        {
            QNetworkAccessManager *netManager = new QNetworkAccessManager();

            QNetworkRequest netRequest(AppEnv::getCrewFetchingUrl(crewID));
            netRequest.setRawHeader("User-Agent", AppEnv::getUserAgent());
            netRequest.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
            netRequest.setRawHeader("Accept-Language", "en-US;q=0.5,en;q=0.3");
            netRequest.setRawHeader("Connection", "keep-alive");

            QNetworkReply *netReply = netManager->get(netRequest);

            QEventLoop *downloadLoop = new QEventLoop();
            QObject::connect(netReply, SIGNAL(finished()), downloadLoop, SLOT(quit()));
            QTimer::singleShot(30000, downloadLoop, SLOT(quit()));
            downloadLoop->exec();
            delete downloadLoop;

            if (netReply->isFinished())
            {
                QByteArray crewJson = netReply->readAll();
                QJsonDocument crewDocument = QJsonDocument::fromJson(crewJson);
                QJsonObject crewObject = crewDocument.object();
                QVariantMap crewMap = crewObject.toVariantMap();
                QString crewName;
                bool isFound = false;

                if (crewMap.contains("activities"))
                {
                    QList<QVariant> activitiesList = crewMap["activities"].toList();
                    foreach (const QVariant &activitiesVariant, activitiesList)
                    {
                        QMap<QString, QVariant> activityRootMap = activitiesVariant.toMap();
                        foreach(const QVariant &activityRootVariant, activityRootMap)
                        {
                            QMap<QString, QVariant> activityMap = activityRootVariant.toMap();
                            foreach(const QVariant &activityVariant, activityMap)
                            {
                                QMap<QString, QVariant> activityFinalMap = activityVariant.toMap();
                                if (activityFinalMap.contains("id") && activityFinalMap["id"] == crewID)
                                {
                                    if (activityFinalMap.contains("name") && isFound == false)
                                    {
                                        isFound = true;
                                        crewName = activityFinalMap["name"].toString();
                                    }
                                }
                            }
                        }
                    }
                }
                if (!crewName.isNull())
                {
                    crewDB->setCrewName(crewID.toInt(), crewName);
                }
            }

            QEventLoop *waitingLoop = new QEventLoop();
            QTimer::singleShot(requestDelay, waitingLoop, SLOT(quit()));
            waitingLoop->exec();
            delete waitingLoop;

            netReply->deleteLater();
            delete netReply;
            netManager->deleteLater();
            delete netManager;
        }
    }
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

                QNetworkRequest netRequest(AppEnv::getPlayerFetchingUrl(crewID, QString::number(currentPage)));
                netRequest.setRawHeader("User-Agent", AppEnv::getUserAgent());
                netRequest.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
                netRequest.setRawHeader("Accept-Language", "en-US;q=0.5,en;q=0.3");
                netRequest.setRawHeader("Connection", "keep-alive");

                QNetworkReply *netReply = netManager->get(netRequest);

                QEventLoop *downloadLoop = new QEventLoop();
                QObject::connect(netReply, SIGNAL(finished()), downloadLoop, SLOT(quit()));
                QTimer::singleShot(30000, downloadLoop, SLOT(quit()));
                downloadLoop->exec();
                delete downloadLoop;

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

                    QEventLoop *waitingLoop = new QEventLoop();
                    QTimer::singleShot(requestDelay, waitingLoop, SLOT(quit()));
                    waitingLoop->exec();
                    delete waitingLoop;

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
