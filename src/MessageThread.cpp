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

#include "TranslationClass.h"
#include "MessageThread.h"
#include "AppEnv.h"
#include "config.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QEventLoop>
#include <QUrlQuery>
#include <QTimer>
#include <QDebug>
#include <QUrl>

MessageThread::MessageThread(uint cacheId, QObject *parent) : QThread(parent), cacheId(cacheId)
{
    threadRunning = true;
}

void MessageThread::run()
{
    QEventLoop threadLoop;

    QObject::connect(this, SIGNAL(threadTerminated()), &threadLoop, SLOT(quit()));

    while (threadRunning) {
        {
#ifdef GTA5SYNC_MOTD_WEBURL
            QUrl motdWebUrl = QUrl(GTA5SYNC_MOTD_WEBURL);
#else
            QUrl motdWebUrl = QUrl("https://motd.syping.de/gta5view-dev/");
#endif
            QUrlQuery urlQuery(motdWebUrl);
            urlQuery.addQueryItem("code", GTA5SYNC_BUILDCODE);
            urlQuery.addQueryItem("cacheid", QString::number(cacheId));
            urlQuery.addQueryItem("lang", Translator->getCurrentLanguage());
            urlQuery.addQueryItem("version", GTA5SYNC_APPVER);
            motdWebUrl.setQuery(urlQuery);

            QNetworkAccessManager *netManager = new QNetworkAccessManager();
            QNetworkRequest netRequest(motdWebUrl);
            netRequest.setRawHeader("User-Agent", AppEnv::getUserAgent());
            QNetworkReply *netReply = netManager->get(netRequest);

            QEventLoop downloadLoop;
            QObject::connect(netManager, SIGNAL(finished(QNetworkReply*)), &downloadLoop, SLOT(quit()));
            QObject::connect(this, SIGNAL(threadTerminated()), &threadLoop, SLOT(quit()));
            QTimer::singleShot(60000, &downloadLoop, SLOT(quit()));
            downloadLoop.exec();

            if (netReply->isFinished()) {
                QByteArray jsonContent = netReply->readAll();
                QString headerData = QString::fromUtf8(netReply->rawHeader("gta5view"));
                if (!headerData.isEmpty()) {
                    QMap<QString,QString> headerMap;
                    const QStringList headerVarList = headerData.split(';');
                    for (QString headerVar : headerVarList) {
                        QStringList varValueList = headerVar.split('=');
                        if (varValueList.length() >= 2) {
                            const QString variable = varValueList.at(0).trimmed();
                            varValueList.removeFirst();
                            const QString value = varValueList.join('=');
                            headerMap.insert(variable, value);
                        }
                    }
                    if (headerMap.value("update", "false") == "true") {
                        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonContent);
                        emit messagesArrived(jsonDocument.object());
                    }
                    if (headerMap.contains("cache")) {
                        bool uintOk;
                        uint cacheVal = headerMap.value("cache").toUInt(&uintOk);
                        if (uintOk) {
                            cacheId = cacheVal;
                            emit updateCacheId(cacheId);
                        }
                    }
                }
            }

            delete netReply;
            delete netManager;
        }

        QTimer::singleShot(300000, &threadLoop, SLOT(quit()));
        threadLoop.exec();
    }
}

void MessageThread::terminateThread()
{
    threadRunning = false;
    emit threadTerminated();
}
