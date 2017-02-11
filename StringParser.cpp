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

#include "StringParser.h"
#include "config.h"
#include <QApplication>
#include <QTextCodec>
#include <QByteArray>
#include <QFileInfo>
#include <QString>
#include <QList>
#include <QDir>

StringParser::StringParser()
{

}

QString StringParser::parseTitleString(const QByteArray &commitBytes, int maxLength)
{
    Q_UNUSED(maxLength)
    QString retStr = QTextCodec::codecForName("UTF-16LE")->toUnicode(commitBytes).trimmed();
    retStr.remove(QChar((char)0x00));
    return retStr;
}

QString StringParser::convertDrawStringForLog(const QString &inputStr)
{
    QString outputStr = inputStr;
    return outputStr.replace("&","&u;").replace(",","&c;");
}

QString StringParser::convertLogStringForDraw(const QString &inputStr)
{
    QString outputStr = inputStr;
    return outputStr.replace("&c;",",").replace("&u;","&");
}

QString StringParser::convertBuildedString(const QString &buildedStr)
{
    QString outputStr = buildedStr;
    QByteArray sharePath = GTA5SYNC_SHARE;
    outputStr.replace("$SHAREDIR", QString::fromUtf8(sharePath));
    outputStr.replace("$RUNDIR", QFileInfo(qApp->applicationFilePath()).absoluteDir().absolutePath());
    outputStr.replace("$SEPARATOR", QDir::separator());
    return outputStr;
}
