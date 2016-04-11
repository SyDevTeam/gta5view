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

#define _CRT_SECURE_NO_WARNINGS
#include "AppEnv.h"
#include "StandardPaths.h"
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <iostream>
using namespace std;

AppEnv::AppEnv()
{

}

QString AppEnv::getGameFolder(bool *ok)
{
    QDir dir;
    QString GTAV_FOLDER(getenv("GTAV_FOLDER"));
    if (GTAV_FOLDER != "")
    {
        dir.setPath(GTAV_FOLDER);
        if (dir.exists())
        {
            *ok = true;
#ifdef GTA5SYNC_WIN
            _putenv(QString("GTAV_FOLDER=" + dir.absolutePath()).toStdString().c_str());
#else
            setenv("GTAV_FOLDER", dir.absolutePath().toStdString().c_str(), 1);
#endif
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
#ifdef GTA5SYNC_WIN
        _putenv(QString("GTAV_FOLDER=" + dir.absolutePath()).toStdString().c_str());
#else
        setenv("GTAV_FOLDER", dir.absolutePath().toStdString().c_str(), 1);
#endif
        return dir.absolutePath();
    }

    dir.setPath(GTAV_defaultFolder);
    if (dir.exists())
    {
        *ok = true;
#ifdef GTA5SYNC_WIN
        _putenv(QString("GTAV_FOLDER=" + dir.absolutePath()).toStdString().c_str());
#else
        setenv("GTAV_FOLDER", dir.absolutePath().toStdString().c_str(), 1);
#endif
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
#ifdef GTA5SYNC_WIN
        _putenv(QString("GTAV_FOLDER=" + dir.absolutePath()).toStdString().c_str());
#else
        setenv("GTAV_FOLDER", dir.absolutePath().toStdString().c_str(), 1);
#endif
        return true;
    }
    return false;
}
