/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2023 Syping
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

#ifndef APPENV_H
#define APPENV_H

#include <QString>
#include <QUrl>

enum class GameVersion : int { NoVersion = 0, SocialClubVersion = 1, SteamVersion = 2, BothVersions = 3 };
enum class GameLanguage : int { Undefined = 0, English = 1, French = 2, Italian = 3, German = 4, Spanish = 5, Mexican = 6, Brasilian = 7, Russian = 8, Polish = 9, Japanese = 10, SChinese = 11, TChinese = 12, Korean = 13 };

class AppEnv
{
public:
    AppEnv();

    // Build Stuff
    static QString getBuildDateTime();
    static QString getBuildCode();

    // Folder Stuff
    static QString getGTAVFolder(bool *ok = 0);
    static QString getRDR2Folder(bool *ok = 0);
    static QString getExLangFolder();
    static QString getInLangFolder();
    static QString getImagesFolder();
    static QString getPluginsFolder();
    static QString getShareFolder();

    // Web Stuff
    static QByteArray getUserAgent();
    static QUrl getCrewFetchingUrl(QString crewID);
    static QUrl getPlayerFetchingUrl(QString crewID, QString pageNumber);
    static QUrl getPlayerFetchingUrl(QString crewID, int pageNumber);

    // Game Stuff
    static GameVersion getGTAVVersion();
    static GameLanguage getGTAVLanguage(GameVersion gameVersion);
    static GameLanguage gameLanguageFromString(QString gameLanguage);
    static QString gameLanguageToString(GameLanguage gameLanguage);

    // Screen Stuff
    static qreal screenRatio();
    static qreal screenRatioPR();
};

#endif // APPENV_H
