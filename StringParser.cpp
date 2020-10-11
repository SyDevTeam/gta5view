/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
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

#include "StringParser.h"
#include <QTextDocument>
#include <QLibraryInfo>
#include <QByteArray>
#include <QFileInfo>
#include <QString>
#include <QList>
#include <QDir>

#ifdef GTA5SYNC_PROJECT
#include <QApplication>
#include "config.h"
#endif

StringParser::StringParser()
{

}

QString StringParser::escapeString(const QString &toEscape)
{
#if QT_VERSION >= 0x050000
    return toEscape.toHtmlEscaped();
#else
    return Qt::escape(toEscape);
#endif
}

#ifdef GTA5SYNC_PROJECT
QString StringParser::convertBuildedString(const QString &buildedStr)
{
    QString outputStr = buildedStr;
    outputStr.replace("APPNAME:", QString::fromUtf8(GTA5SYNC_APPSTR));
    outputStr.replace("SHAREDDIR:", QString::fromUtf8(GTA5SYNC_SHARE));
    outputStr.replace("RUNDIR:", QFileInfo(QApplication::applicationFilePath()).canonicalPath());
#if QT_VERSION >= 0x060000
    outputStr.replace("QCONFLANG:", QLibraryInfo::path(QLibraryInfo::TranslationsPath));
    outputStr.replace("QCONFPLUG:", QLibraryInfo::path(QLibraryInfo::PluginsPath));
#else
    outputStr.replace("QCONFLANG:", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    outputStr.replace("QCONFPLUG:", QLibraryInfo::location(QLibraryInfo::PluginsPath));
#endif
    outputStr.replace("SEPARATOR:", QDir::separator());
    return outputStr;
}
#endif
