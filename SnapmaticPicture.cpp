/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping Gaming Team
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

#include "SnapmaticPicture.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>
#include <QPixmap>
#include <QString>
#include <QDebug>
#include <QFile>

SnapmaticPicture::SnapmaticPicture(QObject *parent, QString fileName) : QObject(parent)
{
    // PARSE INT INIT - DO NOT CHANGE THIS VALUES
    snapmaticHeaderLength = 278;
    snapmaticUsefulLength = 256;
    jpegHeaderLineDifStr = 2;
    jpegPreHeaderLength = 14;
    jpegPicStreamLength = 524288;
    jsonStreamLength = 3076;

    // INIT PIC
    cachePicture = QPixmap(0,0);
    picFileName = "";
    pictureStr = "";
    lastStep = "";

    // INIT JSON
    jsonStr = "";
    jsonLocX = 0;
    jsonLocY = 0;
    jsonLocZ = 0;
    jsonCrewID = 0;
    jsonPlyrsList = QStringList();

    // SET PIC FILENAME
    if (fileName != "")
    {
        picFileName = fileName;
    }
}

bool SnapmaticPicture::readingPicture()
{
    // Start opening file
    // lastStep is like currentStep

    QFile *picFile = new QFile(picFileName);
    if (!picFile->open(QFile::ReadOnly))
    {
        lastStep = "1;/1,OpenFile," + convertDrawStringForLog(picFileName);
        return false;
    }

    // Reading Snapmatic Header
    if (!picFile->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + convertDrawStringForLog(picFileName) + ",1,NOHEADER";
        return false;
    }
    QByteArray snapmaticHeaderLine = picFile->read(snapmaticHeaderLength);
    pictureStr = getSnapmaticPictureString(snapmaticHeaderLine);

    // Reading JPEG Header Line
    if (!picFile->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + convertDrawStringForLog(picFileName) + ",2,NOHEADER";
        return false;
    }
    QByteArray jpegHeaderLine = picFile->read(jpegPreHeaderLength);

    // Checking for JPEG
    jpegHeaderLine.remove(0, jpegHeaderLineDifStr);
    if (jpegHeaderLine.left(4) != "JPEG")
    {
        lastStep = "2;/3,ReadingFile," + convertDrawStringForLog(picFileName) + ",2,NOJPEG";
        return false;
    }

    // Read JPEG Stream
    if (!picFile->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + convertDrawStringForLog(picFileName) + ",3,NOPIC";
        return false;
    }
    QByteArray jpegRawContent = picFile->read(jpegPicStreamLength);

    // Read JSON Stream
    if (!picFile->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + convertDrawStringForLog(picFileName) + ",3,NOJSON";
    }
    else if (picFile->read(4) != "JSON")
    {
        lastStep = "2;/3,ReadingFile," + convertDrawStringForLog(picFileName) + ",3,CTJSON";
    }
    QByteArray jsonRawContent = picFile->read(jsonStreamLength);
    jsonStr = getSnapmaticJSONString(jsonRawContent);
    parseJsonContent(); // JSON parsing is own function

    return cachePicture.loadFromData(jpegRawContent);
}

QString SnapmaticPicture::getSnapmaticPictureString(QByteArray snapmaticHeader)
{
    QByteArray snapmaticUsefulBytes = snapmaticHeader.left(snapmaticUsefulLength);
    snapmaticUsefulBytes.replace(QByteArray::fromHex("00"),"");
    snapmaticUsefulBytes.replace(QByteArray::fromHex("01"),"");
    return QString::fromLatin1(snapmaticUsefulBytes);
}

QString SnapmaticPicture::getSnapmaticJSONString(QByteArray jsonBytes)
{
    QByteArray jsonUsefulBytes = jsonBytes;
    jsonUsefulBytes.replace(QByteArray::fromHex("00"),"");
    jsonUsefulBytes.replace(QByteArray::fromHex("0C"),"");
    return QString::fromLatin1(jsonUsefulBytes);
}

bool SnapmaticPicture::readingPictureFromFile(QString fileName)
{
    if (fileName != "")
    {
        picFileName = fileName;
        return readingPicture();
    }
    else
    {
        return false;
    }
}

void SnapmaticPicture::setPixmap(QPixmap pixmap)
{
    cachePicture = pixmap;
}

QString SnapmaticPicture::getPictureStr()
{
    return pictureStr;
}

QString SnapmaticPicture::getLastStep()
{
    return lastStep;
}

QPixmap SnapmaticPicture::getPixmap()
{
    return cachePicture;
}

QString SnapmaticPicture::convertDrawStringForLog(QString inputStr)
{
    return inputStr.replace("&","&u;").replace(",","&c;");
}

QString SnapmaticPicture::convertLogStringForDraw(QString inputStr)
{
    return inputStr.replace("&c;",",").replace("&u;","&");
}

// JSON part

void SnapmaticPicture::parseJsonContent()
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonStr.toLatin1());
    QJsonObject jsonObject = jsonDocument.object();
    QVariantMap jsonMap = jsonObject.toVariantMap();

    if (jsonMap.contains("loc"))
    {
        QJsonObject locObject = jsonObject["loc"].toObject();
        QVariantMap locMap = locObject.toVariantMap();
        if (locMap.contains("x")) { jsonLocX = locMap["x"].toDouble(); }
        if (locMap.contains("y")) { jsonLocY = locMap["y"].toDouble(); }
        if (locMap.contains("z")) { jsonLocZ = locMap["z"].toDouble(); }
    }
    if (jsonMap.contains("crewid"))
    {
        jsonCrewID = jsonMap["crewid"].toInt();
    }
    if (jsonMap.contains("plyrs"))
    {
        jsonPlyrsList = jsonMap["plyrs"].toStringList();
    }
}

QString SnapmaticPicture::getJsonStr()
{
    return jsonStr;
}

int SnapmaticPicture::getCrewNumber()
{
    return jsonCrewID;
}

double SnapmaticPicture::getLocationX()
{
    return jsonLocX;
}

double SnapmaticPicture::getLocationY()
{
    return jsonLocY;
}

double SnapmaticPicture::getLocationZ()
{
    return jsonLocZ;
}

QStringList SnapmaticPicture::getPlayers()
{
    return jsonPlyrsList;
}
