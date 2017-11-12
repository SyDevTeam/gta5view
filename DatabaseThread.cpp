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

#define crewMaxPages 83
#define maxLoadFails 3

DatabaseThread::DatabaseThread(CrewDatabase *crewDB, QObject *parent) : QThread(parent), crewDB(crewDB)
{
    continueLastCrew = true;
    threadRunning = true;
}

void DatabaseThread::run()
{
    QEventLoop threadLoop;

    QStringList crewList;
    QStringList crewListR;

    // Register thread loop end signal
    QObject::connect(this, SIGNAL(threadTerminated()), &threadLoop, SLOT(quit()));

    // Setup crewList for Quick time scan
    crewList = crewDB->getCrews();
    if (!crewList.isEmpty())
    {
        crewListR = deleteCompatibleCrews(crewList);
    }
    else
    {
        while (crewList.isEmpty() && threadRunning)
        {
            QTimer::singleShot(1000, &threadLoop, SLOT(quit()));
            threadLoop.exec();
            if (!crewDB->isAddingCrews())
            {
                crewList = crewDB->getCrews();
            }
        }
        if (threadRunning)
        {
            crewListR = deleteCompatibleCrews(crewList);
        }
    }

    // Only do QTS when Thread should be run
    if (threadRunning)
    {
        // Quick time scan
#ifdef GTA5SYNC_DEBUG
        qDebug() << "Start QTS";
#endif
        if (crewListR.length() <= 5)
        {
            scanCrewReference(crewListR, 2500);
            emit crewNameUpdated();
        }
        if (crewList.length() <= 3)
        {
            scanCrewMembersList(crewList, 3, 2500);
            emit playerNameUpdated();
        }
        else if (crewList.length() <= 5)
        {
            scanCrewMembersList(crewList, 2, 2500);
            emit playerNameUpdated();
        }

        if (threadRunning)
        {
            QTimer::singleShot(10000, &threadLoop, SLOT(quit()));
            threadLoop.exec();
        }
    }

    while (threadRunning)
    {
        crewList = crewDB->getCrews();
        crewListR = deleteCompatibleCrews(crewList);

        // Long time scan
#ifdef GTA5SYNC_DEBUG
        qDebug() << "Start LTS";
#endif
        scanCrewReference(crewListR, 10000);
        emit crewNameUpdated();
        scanCrewMembersList(crewList, crewMaxPages, 10000);
        emit playerNameUpdated();

        if (threadRunning)
        {
            QTimer::singleShot(300000, &threadLoop, SLOT(quit()));
            threadLoop.exec();
        }
    }
}

void DatabaseThread::scanCrewReference(const QStringList &crewList, const int &requestDelay)
{
    for (QString crewID : crewList)
    {
        if (threadRunning && crewID != QLatin1String("0"))
        {
            QNetworkAccessManager *netManager = new QNetworkAccessManager();
            QNetworkRequest netRequest(AppEnv::getCrewFetchingUrl(crewID));
#if QT_VERSION >= 0x050600
            netRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
            netRequest.setRawHeader("User-Agent", AppEnv::getUserAgent());
            netRequest.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
            netRequest.setRawHeader("Accept-Language", "en-US;q=0.5,en;q=0.3");
            netRequest.setRawHeader("Connection", "keep-alive");

            QNetworkReply *netReply = netManager->get(netRequest);

            QEventLoop *downloadLoop = new QEventLoop();
            QObject::connect(netReply, SIGNAL(finished()), downloadLoop, SLOT(quit()));
            if (!continueLastCrew) { QObject::connect(this, SIGNAL(threadTerminated()), downloadLoop, SLOT(quit())); }
            QTimer::singleShot(30000, downloadLoop, SLOT(quit()));
            downloadLoop->exec();
            downloadLoop->disconnect();
            delete downloadLoop;

            if (netReply->isFinished())
            {
                QString crewName;
                QByteArray crewHtml = netReply->readAll();
                QStringList crewHtmlSplit1 = QString::fromUtf8(crewHtml).split("<title>Rockstar Games Social Club - Crew : ");
                if (crewHtmlSplit1.length() >= 2)
                {
                    QStringList crewHtmlSplit2 = QString(crewHtmlSplit1.at(1)).split("</title>");
                    if (crewHtmlSplit2.length() >= 1)
                    {
                        crewName = crewHtmlSplit2.at(0);
                    }
                }
                if (!crewName.isEmpty())
                {
                    emit crewNameFound(crewID.toInt(), crewName);
                }
            }

            if (threadRunning)
            {
                QEventLoop *waitingLoop = new QEventLoop();
                QTimer::singleShot(requestDelay, waitingLoop, SLOT(quit()));
                if (!continueLastCrew) { QObject::connect(this, SIGNAL(threadTerminated()), waitingLoop, SLOT(quit())); }
                waitingLoop->exec();
                waitingLoop->disconnect();
                delete waitingLoop;
            }

            delete netReply;
            delete netManager;
        }
    }
}

void DatabaseThread::scanCrewMembersList(const QStringList &crewList, const int &maxPages, const int &requestDelay)
{
    for (QString crewID : crewList)
    {
        if (threadRunning && crewID != QLatin1String("0"))
        {
            int currentFail = 0;
            int currentPage = 0;
            int foundPlayers = 0;
            int totalPlayers = 1000;

            while(foundPlayers < totalPlayers && currentPage < maxPages && (continueLastCrew ? true : threadRunning))
            {
                QNetworkAccessManager *netManager = new QNetworkAccessManager();
                QNetworkRequest netRequest(AppEnv::getPlayerFetchingUrl(crewID, currentPage));
#if QT_VERSION >= 0x050600
                netRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
                netRequest.setRawHeader("User-Agent", AppEnv::getUserAgent());
                netRequest.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
                netRequest.setRawHeader("Accept-Language", "en-US;q=0.5,en;q=0.3");
                netRequest.setRawHeader("Connection", "keep-alive");

                QNetworkReply *netReply = netManager->get(netRequest);

                QEventLoop *downloadLoop = new QEventLoop();
                QObject::connect(netReply, SIGNAL(finished()), downloadLoop, SLOT(quit()));
                if (!continueLastCrew) { QObject::connect(this, SIGNAL(threadTerminated()), downloadLoop, SLOT(quit())); }
                QTimer::singleShot(30000, downloadLoop, SLOT(quit()));
                downloadLoop->exec();
                downloadLoop->disconnect();
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
                        const QList<QVariant> memberList = crewMap["Members"].toList();
                        for (QVariant memberVariant : memberList)
                        {
                            QMap<QString, QVariant> memberMap = memberVariant.toMap();
                            if (memberMap.contains("RockstarId") && memberMap.contains("Name"))
                            {
                                int RockstarId = memberMap["RockstarId"].toInt();
                                QString memberName = memberMap["Name"].toString();
                                if (!memberName.isEmpty() && RockstarId != 0)
                                {
                                    foundPlayers++;
                                    emit playerNameFound(RockstarId, memberName);
                                }
                            }
                        }
                    }

                    currentPage++;
                }
                else
                {
                    currentFail++;
                    if (currentFail == maxLoadFails)
                    {
                        currentFail = 0;
                        currentPage++;
                    }
                }

                delete netReply;
                delete netManager;

                if (foundPlayers < totalPlayers && currentPage < maxPages && (continueLastCrew ? true : threadRunning))
                {
                    QEventLoop *waitingLoop = new QEventLoop();
                    QTimer::singleShot(requestDelay, waitingLoop, SLOT(quit()));
                    if (!continueLastCrew) { QObject::connect(this, SIGNAL(threadTerminated()), waitingLoop, SLOT(quit())); }
                    waitingLoop->exec();
                    waitingLoop->disconnect();
                    delete waitingLoop;
                }
            }
        }
    }
}

void DatabaseThread::deleteCompatibleCrews(QStringList *crewList)
{
    for (QString crewNID : *crewList)
    {
        if (crewDB->isCompatibleCrew(crewNID))
        {
            crewList->removeAll(crewNID);
        }
    }
}

QStringList DatabaseThread::deleteCompatibleCrews(const QStringList &crewList)
{
    QStringList crewListR = crewList;
    for (QString crewNID : crewListR)
    {
        if (crewDB->isCompatibleCrew(crewNID))
        {
            crewListR.removeAll(crewNID);
        }
    }
    return crewListR;
}

void DatabaseThread::terminateThread()
{
    threadRunning = false;
    emit threadTerminated();
}
