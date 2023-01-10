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

#include "TranslationClass.h"
#include "GlobalString.h"
#include "config.h"
#include <QStringBuilder>
#include <QStringList>
#include <QFileInfo>
#include <QSettings>
#include <QLocale>
#include <QDebug>

GlobalString::GlobalString()
{
}

QMap<QString, QString> GlobalString::getGlobalMap()
{
    QMap<QString, QString> globalMap;
    QSettings globalFile(getLanguageFile(), QSettings::IniFormat);
#if QT_VERSION < 0x060000
    globalFile.setIniCodec("UTF-8");
#endif
    globalFile.beginGroup("Global");
    for (const QString &globalStr : globalFile.childKeys()) {
        globalMap[globalStr] = globalFile.value(globalStr, globalStr).toString();
    }
    globalFile.endGroup();
    return globalMap;
}

QString GlobalString::getString(QString valueStr, bool *ok)
{
    QString globalString = valueStr;
    QSettings globalFile(getLanguageFile(), QSettings::IniFormat);
#if QT_VERSION < 0x060000
    globalFile.setIniCodec("UTF-8");
#endif
    globalFile.beginGroup("Global");
    QStringList globalStrList = globalFile.childKeys();
    if (globalStrList.contains(valueStr)) {
        if (ok != nullptr)
            *ok = true;
        globalString = globalFile.value(valueStr, valueStr).toString();
    }
    globalFile.endGroup();
    return globalString;
}

QString GlobalString::getLanguageFile()
{
    QString language = getLanguage();
    QString languageFile = ":/global/global." % language % ".ini";
#if QT_VERSION >= 0x050200
    if (!QFileInfo::exists(languageFile))
        languageFile = ":/global/global.en.ini";
#else
    if (!QFileInfo(languageFile).exists())
        languageFile = ":/global/global.en.ini";
#endif

    return languageFile;
}

QString GlobalString::getLanguage()
{
    return Translator->getCurrentAreaLanguage();
}
