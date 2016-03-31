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

#include <QTextCodec>
#include <QByteArray>
#include <QString>
#include <QList>
#include "StringParser.h"

StringParser::StringParser()
{

}

QString StringParser::parseTitleString(QByteArray commitBytes, int maxLength)
{
    QString finalString;
    int parsedBytes = 0;

    while (parsedBytes <= maxLength)
    {
        QList<QByteArray> parseByteList;
        parseByteList.append(commitBytes.mid(parsedBytes-1, 1));
        parseByteList.append(commitBytes.mid(parsedBytes-2, 1));
        if (parseByteList.at(0).toHex() == "00")
        {
            // Latin character
            finalString.append(QString::fromLatin1(parseByteList.at(1)));
        }
//      else if (parseByteList.at(0).toHex() == "30")
//      {
//          // Japanese character
//          QByteArray japaneseHex;
//          japaneseHex.append(QByteArray::fromHex("A5"));
//          japaneseHex.append(parseByteList.at(1));
//          finalString.append(QTextCodec::codecForName("EUC-JP")->toUnicode(japaneseHex));
//      }
        else
        {
            QByteArray unsupportedHex;
            unsupportedHex.append(parseByteList.at(0));
            unsupportedHex.append(parseByteList.at(1));
            finalString.append(QTextCodec::codecForName("UTF-16BE")->toUnicode(unsupportedHex));
        }
        parsedBytes = parsedBytes + 2;
        parseByteList.clear();
    }

    return finalString;
}
