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

#include "SnapmaticPicture.h"
#include "StringParser.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QVariantMap>
#include <QJsonArray>
#include <QString>
#include <QBuffer>
#include <QDebug>
#include <QImage>
#include <QFile>

SnapmaticPicture::SnapmaticPicture(const QString &fileName, QObject *parent) : QObject(parent), picFileName(fileName)
{
    // PARSE INT INIT - DO NOT CHANGE THIS VALUES
    snapmaticHeaderLength = 278;
    snapmaticUsefulLength = 260;
    snapmaticFileMaxSize = 528192;
    jpegHeaderLineDifStr = 2;
    jpegPreHeaderLength = 14;
    jpegPicStreamLength = 524288;
    jsonStreamLength = 3076;
    tideStreamLength = 260;

    // PARSE EDITOR INIT
    jpegStreamEditorBegin = 292;
    jsonStreamEditorBegin = 524588;
    jsonStreamEditorLength = 3072;
    rawPicContent = "";

    // INIT PIC
    cachePicture = QImage(0, 0, QImage::Format_RGB32);
    picExportFileName = "";
    pictureStr = "";
    lastStep = "";
    sortStr = "";
    titlStr = "";
    descStr = "";
    picOk = 0;

    // INIT JSON
    jsonOk = 0;
    jsonStr = "";
    jsonLocX = 0;
    jsonLocY = 0;
    jsonLocZ = 0;
    jsonCrewID = 0;
    jsonArea = "";
    jsonCreatedTimestamp = 0;
    jsonPlyrsList = QStringList();
}

bool SnapmaticPicture::readingPicture(bool writeEnabled)
{
    // Start opening file
    // lastStep is like currentStep

    QFile *picFile = new QFile(picFileName);
    QIODevice *picStream;

    if (!picFile->open(QFile::ReadOnly))
    {
        lastStep = "1;/1,OpenFile," + StringParser::convertDrawStringForLog(picFileName);
        picFile->deleteLater();
        delete picFile;
        return false;
    }
    rawPicContent = picFile->read(snapmaticFileMaxSize);
    picStream = new QBuffer(&rawPicContent);
    picStream->open(QIODevice::ReadWrite);

    // Reading Snapmatic Header
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",1,NOHEADER";
        picStream->close();
        picStream->deleteLater();
        delete picFile;
        return false;
    }
    QByteArray snapmaticHeaderLine = picStream->read(snapmaticHeaderLength);
    pictureStr = getSnapmaticPictureString(snapmaticHeaderLine);

    // Reading JPEG Header Line
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",2,NOHEADER";
        picStream->close();
        picStream->deleteLater();
        delete picFile;
        return false;
    }
    QByteArray jpegHeaderLine = picStream->read(jpegPreHeaderLength);

    // Checking for JPEG
    jpegHeaderLine.remove(0, jpegHeaderLineDifStr);
    if (jpegHeaderLine.left(4) != "JPEG")
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",2,NOJPEG";
        picStream->close();
        picStream->deleteLater();
        delete picFile;
        return false;
    }

    // Read JPEG Stream
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",2,NOPIC";
        picStream->close();
        picStream->deleteLater();
        delete picFile;
        return false;
    }
    QByteArray jpegRawContent = picStream->read(jpegPicStreamLength);
    picOk = cachePicture.loadFromData(jpegRawContent, "JPEG");

    // Read JSON Stream
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",3,NOJSON";
        picStream->close();
        picStream->deleteLater();
        delete picFile;
        return picOk;
    }
    else if (picStream->read(4) != "JSON")
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",3,CTJSON";
        picStream->close();
        picStream->deleteLater();
        delete picFile;
        return picOk;
    }
    QByteArray jsonRawContent = picStream->read(jsonStreamLength);
    jsonStr = getSnapmaticJSONString(jsonRawContent);
    parseJsonContent(); // JSON parsing is own function

    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",4,NOTITL";
        picStream->close();
        picStream->deleteLater();
        delete picFile;
        return picOk;
    }
    else if (picStream->read(4) != "TITL")
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",4,CTTITL";
        picStream->close();
        picStream->deleteLater();
        delete picFile;
        return picOk;
    }
    QByteArray titlRawContent = picStream->read(tideStreamLength);
    titlStr = getSnapmaticTIDEString(titlRawContent);

    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",5,NODESC";
        picStream->close();
        picStream->deleteLater();
        delete picFile;
        return picOk;
    }
    else if (picStream->read(4) != "DESC")
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",5,CTDESC";
        picStream->close();
        picStream->deleteLater();
        delete picFile;
        return picOk;
    }
    QByteArray descRawContent = picStream->read(tideStreamLength);
    descStr = getSnapmaticTIDEString(descRawContent);

    parseSnapmaticExportAndSortString();

    picStream->close();
    picStream->deleteLater();
    picFile->deleteLater();
    delete picStream;
    delete picFile;
    if (!writeEnabled) { rawPicContent.clear(); }
    return picOk;
}

QString SnapmaticPicture::getSnapmaticPictureString(const QByteArray &snapmaticHeader)
{
    QByteArray snapmaticBytes = snapmaticHeader.left(snapmaticUsefulLength);
    QList<QByteArray> snapmaticBytesList = snapmaticBytes.split(char(0x01));
    snapmaticBytes = snapmaticBytesList.at(1);
    snapmaticBytesList.clear();
    return StringParser::parseTitleString(snapmaticBytes, snapmaticBytes.length());
}

QString SnapmaticPicture::getSnapmaticJSONString(const QByteArray &jsonBytes)
{
    QByteArray jsonUsefulBytes = jsonBytes;
    jsonUsefulBytes.replace((char)0x00, "");
    jsonUsefulBytes.replace((char)0x0c, "");
    return QString::fromUtf8(jsonUsefulBytes).trimmed();
}

QString SnapmaticPicture::getSnapmaticTIDEString(const QByteArray &tideBytes)
{
    QByteArray tideUsefulBytes = tideBytes;
    tideUsefulBytes.remove(0,4);
    QList<QByteArray> tideUsefulBytesList = tideUsefulBytes.split(char(0x00));
    return QString::fromUtf8(tideUsefulBytesList.at(0)).trimmed();
}

void SnapmaticPicture::parseSnapmaticExportAndSortString()
{
    QStringList pictureStrList = pictureStr.split(" - ");
    if (pictureStrList.length() <= 2)
    {
        QString dtStr = pictureStrList.at(1);
        QStringList dtStrList = dtStr.split(" ");
        if (dtStrList.length() <= 2)
        {
            QString dayStr;
            QString yearStr;
            QString monthStr;
            QString dateStr = dtStrList.at(0);
            QString timeStr = dtStrList.at(1);
            timeStr.replace(":","");
            QStringList dateStrList = dateStr.split("/");
            if (dateStrList.length() <= 3)
            {
                dayStr = dateStrList.at(1);
                yearStr = dateStrList.at(2);
                monthStr = dateStrList.at(0);
            }
            QString cmpPicTitl = titlStr;
            cmpPicTitl.replace("\"", "''");
            cmpPicTitl.replace(" ", "_");
            cmpPicTitl.replace(":", "-");
            cmpPicTitl.replace("\\", "");
            cmpPicTitl.replace("/", "");
            cmpPicTitl.replace("<", "");
            cmpPicTitl.replace(">", "");
            cmpPicTitl.replace("*", "");
            cmpPicTitl.replace("?", "");
            cmpPicTitl.replace(".", "");
            sortStr = yearStr + monthStr + dayStr + timeStr;
            picExportFileName = sortStr + "_" + cmpPicTitl +  ".jpg";
        }
    }
}

bool SnapmaticPicture::readingPictureFromFile(const QString &fileName, bool writeEnabled)
{
    if (fileName != "")
    {
        picFileName = fileName;
        return readingPicture(writeEnabled);
    }
    else
    {
        return false;
    }
}

void SnapmaticPicture::setPicture(const QImage &picture)
{
    cachePicture = picture;
}

QString SnapmaticPicture::getExportPictureFileName()
{
    return picExportFileName;
}

QString SnapmaticPicture::getPictureFileName()
{
    return picFileName;
}

QString SnapmaticPicture::getPictureSortStr()
{
    return sortStr;
}

QString SnapmaticPicture::getPictureDesc()
{
    return descStr;
}

QString SnapmaticPicture::getPictureTitl()
{
    return titlStr;
}

QString SnapmaticPicture::getPictureStr()
{
    return pictureStr;
}

QString SnapmaticPicture::getLastStep()
{
    return lastStep;
}

QImage SnapmaticPicture::getPicture()
{
    return cachePicture;
}

bool SnapmaticPicture::isPicOk()
{
    return picOk;
}

void SnapmaticPicture::setPicFileName(QString picFileName_)
{
    picFileName = picFileName_;
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
    if (jsonMap.contains("area"))
    {
        jsonArea = jsonMap["area"].toString();
    }
    if (jsonMap.contains("crewid"))
    {
        jsonCrewID = jsonMap["crewid"].toInt();
    }
    if (jsonMap.contains("creat"))
    {
        QDateTime createdTimestamp;
        jsonCreatedTimestamp = jsonMap["creat"].toUInt();
        createdTimestamp.setTime_t(jsonCreatedTimestamp);
        jsonCreatedDateTime = createdTimestamp;
    }
    if (jsonMap.contains("plyrs"))
    {
        jsonPlyrsList = jsonMap["plyrs"].toStringList();
    }

    jsonOk = true;
}

bool SnapmaticPicture::isJsonOk()
{
    return jsonOk;
}

QString SnapmaticPicture::getArea()
{
    return jsonArea;
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

QDateTime SnapmaticPicture::getCreatedDateTime()
{
    return jsonCreatedDateTime;
}
