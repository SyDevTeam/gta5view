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
#include "SavegameData.h"
#include <QTextCodec>
#include <QByteArray>
#include <QDebug>
#include <QFile>

#define savegameHeaderLength 260
#define verificationValue QByteArray::fromHex("00000001")

SavegameData::SavegameData(const QString &fileName, QObject *parent) : QObject(parent), savegameFileName(fileName)
{
    // INIT SAVEGAME
    savegameStr = "";
    savegameOk = 0;
}

bool SavegameData::readingSavegame()
{
    // Start opening file
    // lastStep is like currentStep

    QFile *saveFile = new QFile(savegameFileName);
    if (!saveFile->open(QFile::ReadOnly))
    {
        lastStep = "1;/1,OpenFile," + StringParser::convertDrawStringForLog(savegameFileName);
        saveFile->deleteLater();
        delete saveFile;
        return false;
    }

    // Reading Savegame Header
    if (!saveFile->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(savegameFileName) + ",1,NOHEADER";
        saveFile->close();
        saveFile->deleteLater();
        delete saveFile;
        return false;
    }
    QByteArray savegameHeaderLine = saveFile->read(savegameHeaderLength);
    if (savegameHeaderLine.left(4) == verificationValue)
    {
        savegameStr = getSavegameDataString(savegameHeaderLine);
        if (savegameStr.length() >= 1)
        {
            savegameOk = true;
        }
    }
    saveFile->close();
    saveFile->deleteLater();
    delete saveFile;
    return savegameOk;
}

QString SavegameData::getSavegameDataString(const QByteArray &savegameHeader)
{
    QByteArray savegameBytes = savegameHeader.left(savegameHeaderLength);
    QList<QByteArray> savegameBytesList = savegameBytes.split(char(0x01));
    savegameBytes = savegameBytesList.at(1);
    savegameBytesList.clear();
    return StringParser::parseTitleString(savegameBytes, savegameBytes.length());
}

bool SavegameData::readingSavegameFromFile(const QString &fileName)
{
    if (fileName != "")
    {
        savegameFileName = fileName;
        return readingSavegame();
    }
    else
    {
        return false;
    }
}

bool SavegameData::isSavegameOk()
{
    return savegameOk;
}

QString SavegameData::getSavegameFileName()
{
    return savegameFileName;
}

QString SavegameData::getSavegameStr()
{
    return savegameStr;
}

QString SavegameData::getLastStep()
{
    return lastStep;
}

void SavegameData::setSavegameFileName(QString savegameFileName_)
{
    savegameFileName = savegameFileName_;
}
