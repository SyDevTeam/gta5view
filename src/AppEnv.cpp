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

#include "config.h"
#include "AppEnv.h"
#include "StringParser.h"
#include "StandardPaths.h"
#include <QtGlobal>
#include <QStringBuilder>
#include <QApplication>
#include <QSettings>
#include <QScreen>
#include <QDebug>
#include <QRect>
#include <QDir>

#if QT_VERSION < 0x050000
#include <QDesktopWidget>
#endif

AppEnv::AppEnv()
{
}

// Build Stuff

QString AppEnv::getBuildDateTime()
{
    return GTA5SYNC_BUILDDATETIME;
}

QString AppEnv::getBuildCode()
{
    return GTA5SYNC_BUILDCODE;
}

// Folder Stuff

QString AppEnv::getGTAVFolder(bool *ok)
{
    QDir dir;
    QString GTAV_FOLDER = QString::fromUtf8(qgetenv("GTAV_FOLDER"));
    if (!GTAV_FOLDER.isEmpty()) {
        dir.setPath(GTAV_FOLDER);
        if (dir.exists()) {
            if (ok)
                *ok = true;
            return dir.absolutePath();
        }
    }

#ifdef Q_OS_UNIX
    // TODO: Try to locate the Steam Proton GTA V folder
    const QString GTAV_defaultFolder = StandardPaths::documentsLocation() % "/Rockstar Games/GTA V";
#else
    const QString GTAV_defaultFolder = StandardPaths::documentsLocation() % "/Rockstar Games/GTA V";
#endif
    QString GTAV_returnFolder = GTAV_defaultFolder;

    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("dir");
    bool forceDir = settings.value("force", false).toBool();
    GTAV_returnFolder = settings.value("dir", GTAV_defaultFolder).toString();
    settings.endGroup();

    settings.beginGroup("GameDirectory");
    settings.beginGroup("GTA V");
    forceDir = settings.value("ForceCustom", forceDir).toBool();
    GTAV_returnFolder = settings.value("Directory", GTAV_returnFolder).toString();
    settings.endGroup();
    settings.endGroup();

    if (forceDir) {
        dir.setPath(GTAV_returnFolder);
        if (dir.exists()) {
            if (ok)
                *ok = true;
            return dir.absolutePath();
        }
    }

    dir.setPath(GTAV_defaultFolder);
    if (dir.exists()) {
        if (ok)
            *ok = true;
        return dir.absolutePath();
    }

    if (!forceDir) {
        dir.setPath(GTAV_returnFolder);
        if (dir.exists()) {
            if (ok)
                *ok = true;
            return dir.absolutePath();
        }
    }

    if (ok)
        *ok = false;
    return QString();
}

QString AppEnv::getRDR2Folder(bool *ok)
{
    QDir dir;
    QString RDR2_FOLDER = QString::fromUtf8(qgetenv("RDR2_FOLDER"));
    if (!RDR2_FOLDER.isEmpty()) {
        dir.setPath(RDR2_FOLDER);
        if (dir.exists()) {
            if (ok)
                *ok = true;
            return dir.absolutePath();
        }
    }

#ifdef Q_OS_UNIX
    // TODO: Try to locate the Steam Proton RDR 2 folder
    const QString RDR2_defaultFolder = StandardPaths::documentsLocation() % "/Rockstar Games/Red Dead Redemption 2";
#else
    const QString RDR2_defaultFolder = StandardPaths::documentsLocation() % "/Rockstar Games/Red Dead Redemption 2";
#endif
    QString RDR2_returnFolder = RDR2_defaultFolder;

    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("GameDirectory");
    settings.beginGroup("RDR 2");
    bool forceDir = settings.value("ForceCustom", false).toBool();
    RDR2_returnFolder = settings.value("Directory", RDR2_defaultFolder).toString();
    settings.endGroup();
    settings.endGroup();

    if (forceDir) {
        dir.setPath(RDR2_returnFolder);
        if (dir.exists()) {
            if (ok)
                *ok = true;
            return dir.absolutePath();
        }
    }

    dir.setPath(RDR2_defaultFolder);
    if (dir.exists()) {
        if (ok)
            *ok = true;
        return dir.absolutePath();
    }

    if (!forceDir) {
        dir.setPath(RDR2_returnFolder);
        if (dir.exists()) {
            if (ok)
                *ok = true;
            return dir.absolutePath();
        }
    }

    if (ok)
        *ok = false;
    return QString();
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
    return StringParser::convertBuildedString(GTA5SYNC_SHARE % QLatin1String("/APPNAME:/translations"));
#endif
#else
#ifdef GTA5SYNC_INLANG
    return StringParser::convertBuildedString(GTA5SYNC_INLANG);
#else
    return QLatin1String(":/tr");
#endif
#endif
}

QString AppEnv::getPluginsFolder()
{
    return StringParser::convertBuildedString(GTA5SYNC_PLUG);
}

QString AppEnv::getImagesFolder()
{
#if defined(GTA5SYNC_QCONF) && defined(GTA5SYNC_CMAKE)
#ifdef Q_OS_WIN
    return StringParser::convertBuildedString(GTA5SYNC_SHARE % QLatin1String("/resources"));
#else
    return StringParser::convertBuildedString(GTA5SYNC_SHARE % QLatin1String("/APPNAME:/resources"));
#endif
#else
    return QLatin1String(":/img");
#endif
}

QString AppEnv::getShareFolder()
{
    return StringParser::convertBuildedString(GTA5SYNC_SHARE);
}

// Web Stuff

QByteArray AppEnv::getUserAgent()
{
    return QString("Mozilla/5.0 %1/%2").arg(GTA5SYNC_APPSTR, GTA5SYNC_APPVER).toUtf8();
}

QUrl AppEnv::getCrewFetchingUrl(QString crewID)
{
    return QUrl(QString("https://socialclub.rockstargames.com/crew/%1/%1").arg(crewID));
}

QUrl AppEnv::getPlayerFetchingUrl(QString crewID, QString pageNumber)
{
    return QUrl(QString("https://socialclub.rockstargames.com/crewsapi/GetMembersList?crewId=%1&pageNumber=%2&pageSize=5000").arg(crewID, pageNumber));
}

QUrl AppEnv::getPlayerFetchingUrl(QString crewID, int pageNumber)
{
    return getPlayerFetchingUrl(crewID, QString::number(pageNumber));
}

// Game Stuff

GameVersion AppEnv::getGTAVVersion()
{
#ifdef Q_OS_WIN
    QString argumentValue;
#ifdef _WIN64
    argumentValue = "\\WOW6432Node";
#endif
    QSettings registrySettingsSc(QString("HKEY_LOCAL_MACHINE\\SOFTWARE%1\\Rockstar Games\\Grand Theft Auto V").arg(argumentValue), QSettings::NativeFormat);
    QString installFolderSc = registrySettingsSc.value("InstallFolder", "").toString();
    QDir installFolderScDir(installFolderSc);
    bool scVersionInstalled = false;
    if (!installFolderSc.isEmpty() && installFolderScDir.exists()) {
#ifdef GTA5SYNC_DEBUG
        qDebug() << "gameVersionFoundSocialClubVersion";
#endif
        scVersionInstalled = true;
    }

    QSettings registrySettingsSteam(QString("HKEY_LOCAL_MACHINE\\SOFTWARE%1\\Rockstar Games\\GTAV").arg(argumentValue), QSettings::NativeFormat);
    QString installFolderSteam = registrySettingsSteam.value("installfoldersteam", "").toString();
    if (installFolderSteam.right(5) == "\\GTAV") {
        installFolderSteam = installFolderSteam.remove(installFolderSteam.length() - 5, 5);
    }
    QDir installFolderSteamDir(installFolderSteam);
    bool steamVersionInstalled = false;
    if (!installFolderSteam.isEmpty() && installFolderSteamDir.exists()) {
#ifdef GTA5SYNC_DEBUG
        qDebug() << "gameVersionFoundSteamVersion";
#endif
        steamVersionInstalled = true;
    }

    if (scVersionInstalled && steamVersionInstalled) {
        return GameVersion::BothVersions;
    }
    else if (scVersionInstalled) {
        return GameVersion::SocialClubVersion;
    }
    else if (steamVersionInstalled) {
        return GameVersion::SteamVersion;
    }
    else {
        return GameVersion::NoVersion;
    }
#else
    return GameVersion::NoVersion;
#endif
}

GameLanguage AppEnv::getGTAVLanguage(GameVersion gameVersion)
{
    if (gameVersion == GameVersion::SocialClubVersion) {
#ifdef Q_OS_WIN
        QString argumentValue;
#ifdef _WIN64
        argumentValue = "\\WOW6432Node";
#endif
        QSettings registrySettingsSc(QString("HKEY_LOCAL_MACHINE\\SOFTWARE%1\\Rockstar Games\\Grand Theft Auto V").arg(argumentValue), QSettings::NativeFormat);
        QString languageSc = registrySettingsSc.value("Language", "").toString();
        return gameLanguageFromString(languageSc);
#else
        return GameLanguage::Undefined;
#endif
    }
    else if (gameVersion == GameVersion::SteamVersion) {
#ifdef Q_OS_WIN
        QString argumentValue;
#ifdef _WIN64
        argumentValue = "\\WOW6432Node";
#endif
        QSettings registrySettingsSteam(QString("HKEY_LOCAL_MACHINE\\SOFTWARE%1\\Rockstar Games\\Grand Theft Auto V Steam").arg(argumentValue), QSettings::NativeFormat);
        QString languageSteam = registrySettingsSteam.value("Language", "").toString();
        return gameLanguageFromString(languageSteam);
#else
        return GameLanguage::Undefined;
#endif
    }
    return GameLanguage::Undefined;
}

GameLanguage AppEnv::gameLanguageFromString(QString gameLanguage)
{
    if (gameLanguage == "en-US") {
        return GameLanguage::English;
    }
    else if (gameLanguage == "fr-FR") {
        return GameLanguage::French;
    }
    else if (gameLanguage == "it-IT") {
        return GameLanguage::Italian;
    }
    else if (gameLanguage == "de-DE") {
        return GameLanguage::German;
    }
    else if (gameLanguage == "es-ES") {
        return GameLanguage::Spanish;
    }
    else if (gameLanguage == "es-MX") {
        return GameLanguage::Mexican;
    }
    else if (gameLanguage == "pt-BR") {
        return GameLanguage::Brasilian;
    }
    else if (gameLanguage == "ru-RU") {
        return GameLanguage::Russian;
    }
    else if (gameLanguage == "pl-PL") {
        return GameLanguage::Polish;
    }
    else if (gameLanguage == "ja-JP") {
        return GameLanguage::Japanese;
    }
    else if (gameLanguage == "zh-CHS") {
        return GameLanguage::SChinese;
    }
    else if (gameLanguage == "zh-CHT") {
        return GameLanguage::TChinese;
    }
    else if (gameLanguage == "ko-KR") {
        return GameLanguage::Korean;
    }
    return GameLanguage::Undefined;
}

QString AppEnv::gameLanguageToString(GameLanguage gameLanguage)
{
    switch (gameLanguage) {
    case GameLanguage::English:
        return "en-US";
    case GameLanguage::French:
        return "fr-FR";
    case GameLanguage::Italian:
        return "it-IT";
    case GameLanguage::German:
        return "de-DE";
    case GameLanguage::Spanish:
        return "es-ES";
    case GameLanguage::Mexican:
        return "es-MX";
    case GameLanguage::Brasilian:
        return "pt-BR";
    case GameLanguage::Polish:
        return "pl-PL";
    case GameLanguage::Japanese:
        return "ja-JP";
    case GameLanguage::SChinese:
        return "zh-CHS";
    case GameLanguage::TChinese:
        return "zh-CHT";
    case GameLanguage::Korean:
        return "ko-KR";
    default:
        return "Undefinied";
    }
}

// Screen Stuff

qreal AppEnv::screenRatio()
{
    qreal dpi = QApplication::primaryScreen()->logicalDotsPerInch();
#ifdef Q_OS_MAC
    return (dpi / 72);
#else
    return (dpi / 96);
#endif
}

qreal AppEnv::screenRatioPR()
{
    return QApplication::primaryScreen()->devicePixelRatio();
}
