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

#include "config.h"
#include "AppEnv.h"
#include "StringParser.h"
#include "StandardPaths.h"
#include <QtGlobal>
#include <QStringBuilder>
#include <QDesktopWidget>
#include <QApplication>
#include <QSettings>
#include <QScreen>
#include <QDebug>
#include <QRect>
#include <QDir>
#include <iostream>
using namespace std;

AppEnv::AppEnv()
{

}

// Build Stuff

QString AppEnv::getBuildDateTime()
{
    return GTA5SYNC_BUILDDATETIME;
}

// Folder Stuff

QString AppEnv::getGameFolder(bool *ok)
{
    QDir dir;
    QString GTAV_FOLDER = QString::fromUtf8(qgetenv("GTAV_FOLDER"));
    if (GTAV_FOLDER != "")
    {
        dir.setPath(GTAV_FOLDER);
        if (dir.exists())
        {
            if (ok != NULL) *ok = true;
            qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
            return dir.absolutePath();
        }
    }

    QString GTAV_defaultFolder = StandardPaths::documentsLocation() % QDir::separator() % "Rockstar Games" % QDir::separator() % "GTA V";
    QString GTAV_returnFolder = GTAV_defaultFolder;

    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("dir");
    bool forceDir = settings.value("force", false).toBool();
    GTAV_returnFolder = settings.value("dir", GTAV_defaultFolder).toString();
    settings.endGroup();

    if (forceDir)
    {
        dir.setPath(GTAV_returnFolder);
        if (dir.exists())
        {
            if (ok != 0) *ok = true;
            qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
            return dir.absolutePath();
        }
    }

    dir.setPath(GTAV_defaultFolder);
    if (dir.exists())
    {
        if (ok != 0) *ok = true;
        qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
        return dir.absolutePath();
    }

    if (!forceDir)
    {
        dir.setPath(GTAV_returnFolder);
        if (dir.exists())
        {
            if (ok != 0) *ok = true;
            qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
            return dir.absolutePath();
        }
    }

    if (ok != 0) *ok = false;
    return "";
}

bool AppEnv::setGameFolder(QString gameFolder)
{
    QDir dir;
    dir.setPath(gameFolder);
    if (dir.exists())
    {
        qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
        return true;
    }
    return false;
}

QString AppEnv::getExLangFolder()
{
    return StringParser::convertBuildedString(GTA5SYNC_LANG);
}

QString AppEnv::getInLangFolder()
{
#ifdef GTA5SYNC_QCONF
#ifdef GTA5SYNC_INLANG
    return StringParser::convertBuildedString(GTA5SYNC_INLANG);
#else
    return StringParser::convertBuildedString(GTA5SYNC_SHARE % QLatin1String("SEPARATOR:APPNAME:SEPARATOR:translations"));
#endif
#else
#ifdef GTA5SYNC_INLANG
    return StringParser::convertBuildedString(GTA5SYNC_INLANG);
#else
    return QString(":/tr");
#endif
#endif
}

QString AppEnv::getPluginsFolder()
{
    return StringParser::convertBuildedString(GTA5SYNC_PLUG);
}

// Web Stuff

QByteArray AppEnv::getUserAgent()
{
    return QString("Mozilla/5.0 (X11; Linux x86_64; rv:45.0) Gecko/20100101 Firefox/45.0 %1/%2").arg(GTA5SYNC_APPSTR, GTA5SYNC_APPVER).toUtf8();
}

// QUrl AppEnv::getCrewFetchingUrl(QString crewID)
// {
//     return QUrl(QString("https://socialclub.rockstargames.com/reference/crewfeed/%1").arg(crewID));
// }

QUrl AppEnv::getCrewFetchingUrl(QString crewID)
{
    return QUrl(QString("https://socialclub.rockstargames.com/crew/%1/%1").arg(crewID));
}

QUrl AppEnv::getPlayerFetchingUrl(QString crewID, QString pageNumber)
{
    return QUrl(QString("https://socialclub.rockstargames.com/crewsapi/GetMembersList?crewId=%1&pageNumber=%2").arg(crewID, pageNumber));
}

QUrl AppEnv::getPlayerFetchingUrl(QString crewID, int pageNumber)
{
    return getPlayerFetchingUrl(crewID, QString::number(pageNumber));
}

qreal AppEnv::screenRatio()
{
#if QT_VERSION >= 0x050000
    qreal dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
#else
    qreal dpi = qApp->desktop()->logicalDpiX();
#endif
#ifdef Q_OS_MAC
    return (dpi / 72);
#else
    return (dpi / 96);
#endif
}
