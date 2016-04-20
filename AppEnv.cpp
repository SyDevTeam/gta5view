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

#include "config.h"
#include "AppEnv.h"
#include "StringParser.h"
#include "StandardPaths.h"
#include <QDir>
#include <QDebug>
#include <QtGlobal>
#include <QSettings>
#include <iostream>
using namespace std;

AppEnv::AppEnv()
{

}

// Folder Stuff

QString AppEnv::getGameFolder(bool *ok)
{
    QDir dir;
    QString GTAV_FOLDER(qgetenv("GTAV_FOLDER"));
    if (GTAV_FOLDER != "")
    {
        dir.setPath(GTAV_FOLDER);
        if (dir.exists())
        {
            *ok = true;
            qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
            return dir.absolutePath();
        }
    }

    QString GTAV_defaultFolder = StandardPaths::documentsLocation() + QDir::separator() + "Rockstar Games" + QDir::separator() + "GTA V";
    QString GTAV_returnFolder = GTAV_defaultFolder;

    QSettings SyncSettings("Syping", "gta5sync");
    SyncSettings.beginGroup("dir");
    bool forceDir = SyncSettings.value("force", false).toBool();
    if (forceDir)
    {
        GTAV_returnFolder = SyncSettings.value("dir", GTAV_defaultFolder).toString();
    }
    SyncSettings.endGroup();

    dir.setPath(GTAV_returnFolder);
    if (dir.exists())
    {
        *ok = true;
        qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
        return dir.absolutePath();
    }

    dir.setPath(GTAV_defaultFolder);
    if (dir.exists())
    {
        *ok = true;
        qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
        return dir.absolutePath();
    }

    *ok = false;
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

QString AppEnv::getLangFolder()
{
    return StringParser::convertBuildedString(QString::fromUtf8(GTA5SYNC_LANG));
}

QString AppEnv::getPluginsFolder()
{
    return StringParser::convertBuildedString(QString::fromUtf8(GTA5SYNC_PLUG));
}

// Web Stuff

QByteArray AppEnv::getUserAgent()
{
    return QString("Mozilla/5.0 (X11; Linux x86_64; rv:45.0) Gecko/20100101 Firefox/45.0 gta5sync/1.0").toUtf8();
}

QUrl AppEnv::getPlayerFetchingUrl(QString crewID, QString pageNumber)
{
    return QUrl(QString("https://socialclub.rockstargames.com/crewsapi/GetMembersList?crewId=%1&pageNumber=%2").arg(crewID, pageNumber));
}
