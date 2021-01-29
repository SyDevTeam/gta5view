/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2021 Syping
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

QString AppEnv::getGameFolder(bool *ok)
{
    QDir dir;
    QString GTAV_FOLDER = QString::fromUtf8(qgetenv("GTAV_FOLDER"));
    if (GTAV_FOLDER != "") {
        dir.setPath(GTAV_FOLDER);
        if (dir.exists()) {
            if (ok != NULL)
                *ok = true;
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

    if (forceDir) {
        dir.setPath(GTAV_returnFolder);
        if (dir.exists()) {
            if (ok != 0)
                *ok = true;
            qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
            return dir.absolutePath();
        }
    }

    dir.setPath(GTAV_defaultFolder);
    if (dir.exists()) {
        if (ok != 0)
            *ok = true;
        qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
        return dir.absolutePath();
    }

    if (!forceDir) {
        dir.setPath(GTAV_returnFolder);
        if (dir.exists()) {
            if (ok != 0)
                *ok = true;
            qputenv("GTAV_FOLDER", dir.absolutePath().toUtf8());
            return dir.absolutePath();
        }
    }

    if (ok != 0)
        *ok = false;
    return QString();
}

bool AppEnv::setGameFolder(QString gameFolder)
{
    QDir dir;
    dir.setPath(gameFolder);
    if (dir.exists()) {
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
#if QT_VERSION >= 0x050400
#ifdef Q_OS_WIN
    QString kernelVersion = QSysInfo::kernelVersion();
    const QStringList &kernelVersionList = kernelVersion.split(".");
    if (kernelVersionList.length() > 2) {
        kernelVersion = kernelVersionList.at(0) % "." % kernelVersionList.at(1);
    }
    QString runArch = QSysInfo::buildCpuArchitecture();
    if (runArch == "x86_64") {
        runArch = "Win64; x64";
    }
    else if (runArch == "i686") {
        const QString &curArch = QSysInfo::currentCpuArchitecture();
        if (curArch == "x86_64") {
            runArch = "WOW64";
        }
        else if (curArch == "i686") {
            runArch = "Win32; x86";
        }
    }
    return QString("Mozilla/5.0 (Windows NT %1; %2) %3/%4").arg(kernelVersion, runArch, GTA5SYNC_APPSTR, GTA5SYNC_APPVER).toUtf8();
#else
    return QString("Mozilla/5.0 (%1; %2) %3/%4").arg(QSysInfo::kernelType(), QSysInfo::kernelVersion(), GTA5SYNC_APPSTR, GTA5SYNC_APPVER).toUtf8();
#endif
#else
    return QString("Mozilla/5.0 %1/%2").arg(GTA5SYNC_APPSTR, GTA5SYNC_APPVER).toUtf8();
#endif
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

GameVersion AppEnv::getGameVersion()
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

GameLanguage AppEnv::getGameLanguage(GameVersion gameVersion)
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

bool AppEnv::setGameLanguage(GameVersion gameVersion, GameLanguage gameLanguage)
{
    bool socialClubVersion = false;
    bool steamVersion = false;
    if (gameVersion == GameVersion::SocialClubVersion) {
        socialClubVersion = true;
    }
    else if (gameVersion == GameVersion::SteamVersion) {
        steamVersion = true;
    }
    else if (gameVersion == GameVersion::BothVersions) {
        socialClubVersion = true;
        steamVersion = true;
    }
    else {
        return false;
    }
    if (socialClubVersion) {
#ifdef Q_OS_WIN
        QString argumentValue;
#ifdef _WIN64
        argumentValue = "\\WOW6432Node";
#endif
        QSettings registrySettingsSc(QString("HKEY_LOCAL_MACHINE\\SOFTWARE%1\\Rockstar Games\\Grand Theft Auto V").arg(argumentValue), QSettings::NativeFormat);
        if (gameLanguage != GameLanguage::Undefined) {
            registrySettingsSc.setValue("Language", gameLanguageToString(gameLanguage));
        }
        else {
            registrySettingsSc.remove("Language");
        }
        registrySettingsSc.sync();
        if (registrySettingsSc.status() != QSettings::NoError) {
            return false;
        }
#else
        Q_UNUSED(gameLanguage)
#endif
    }
    if (steamVersion) {
#ifdef Q_OS_WIN
        QString argumentValue;
#ifdef _WIN64
        argumentValue = "\\WOW6432Node";
#endif
        QSettings registrySettingsSteam(QString("HKEY_LOCAL_MACHINE\\SOFTWARE%1\\Rockstar Games\\Grand Theft Auto V Steam").arg(argumentValue), QSettings::NativeFormat);
        if (gameLanguage != GameLanguage::Undefined) {
            registrySettingsSteam.setValue("Language", gameLanguageToString(gameLanguage));
        }
        else {
            registrySettingsSteam.remove("Language");
        }
        registrySettingsSteam.sync();
        if (registrySettingsSteam.status() != QSettings::NoError) {
            return false;
        }
#else
        Q_UNUSED(gameLanguage)
#endif
    }
    return true;
}

// Screen Stuff

qreal AppEnv::screenRatio()
{
#if QT_VERSION >= 0x050000
    qreal dpi = QApplication::primaryScreen()->logicalDotsPerInch();
#else
    qreal dpi = QApplication::desktop()->logicalDpiX();
#endif
#ifdef Q_OS_MAC
    return (dpi / 72);
#else
    return (dpi / 96);
#endif
}

qreal AppEnv::screenRatioPR()
{
#if QT_VERSION >= 0x050600
    return QApplication::primaryScreen()->devicePixelRatio();
#else
    return 1;
#endif
}
