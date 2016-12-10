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
}

SnapmaticPicture::~SnapmaticPicture()
{
}

bool SnapmaticPicture::readingPicture(bool writeEnabled_, bool cacheEnabled_)
{
    // Start opening file
    // lastStep is like currentStep

    // Set boolean values
    writeEnabled = writeEnabled_;
    cacheEnabled = cacheEnabled_;

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
    picFile->close();
    delete picFile;

    picStream = new QBuffer(&rawPicContent);
    picStream->open(QIODevice::ReadWrite);

    // Reading Snapmatic Header
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",1,NOHEADER";
        picStream->close();
        picStream->deleteLater();
        delete picStream;
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
        delete picStream;
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
        delete picStream;
        return false;
    }

    // Read JPEG Stream
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",2,NOPIC";
        picStream->close();
        picStream->deleteLater();
        delete picStream;
        return false;
    }
    QByteArray jpegRawContent = picStream->read(jpegPicStreamLength);
    if (cacheEnabled) picOk = cachePicture.loadFromData(jpegRawContent, "JPEG");
    if (!cacheEnabled)
    {
        QImage tempPicture;
        picOk = tempPicture.loadFromData(jpegRawContent, "JPEG");
    }

    // Read JSON Stream
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",3,NOJSON";
        picStream->close();
        picStream->deleteLater();
        delete picStream;
        return picOk;
    }
    else if (picStream->read(4) != "JSON")
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",3,CTJSON";
        picStream->close();
        picStream->deleteLater();
        delete picStream;
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
        delete picStream;
        return picOk;
    }
    else if (picStream->read(4) != "TITL")
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",4,CTTITL";
        picStream->close();
        picStream->deleteLater();
        delete picStream;
        return picOk;
    }
    QByteArray titlRawContent = picStream->read(tideStreamLength);
    titlStr = getSnapmaticTIDEString(titlRawContent);

    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",5,NODESC";
        picStream->close();
        picStream->deleteLater();
        delete picStream;
        return picOk;
    }
    else if (picStream->read(4) != "DESC")
    {
        lastStep = "2;/3,ReadingFile," + StringParser::convertDrawStringForLog(picFileName) + ",5,CTDESC";
        picStream->close();
        picStream->deleteLater();
        delete picStream;
        return picOk;
    }
    QByteArray descRawContent = picStream->read(tideStreamLength);
    descStr = getSnapmaticTIDEString(descRawContent);

    parseSnapmaticExportAndSortString();

    picStream->close();
    picStream->deleteLater();
    delete picStream;
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

bool SnapmaticPicture::readingPictureFromFile(const QString &fileName, bool writeEnabled_, bool cacheEnabled_)
{
    if (fileName != "")
    {
        picFileName = fileName;
        return readingPicture(writeEnabled_, cacheEnabled_);
    }
    else
    {
        return false;
    }
}

bool SnapmaticPicture::setPicture(const QImage &picture)
{
    if (writeEnabled)
    {
        QByteArray picByteArray;
        QBuffer snapmaticStream(&rawPicContent);
        snapmaticStream.open(QIODevice::ReadWrite);
        if (snapmaticStream.seek(jpegStreamEditorBegin))
        {
            bool saveSuccess;
            QByteArray picByteArray1;
            QBuffer picStream1(&picByteArray1);
            picStream1.open(QIODevice::WriteOnly);
            saveSuccess = picture.save(&picStream1, "JPEG", 95);
            picStream1.close();

            if (picByteArray1.length() > jpegPicStreamLength)
            {
                QByteArray picByteArray2;
                QBuffer picStream2(&picByteArray2);
                picStream2.open(QIODevice::WriteOnly);
                saveSuccess = picture.save(&picStream2, "JPEG", 80);
                picStream2.close();
                if (picByteArray2.length() > jpegPicStreamLength)
                {
                    snapmaticStream.close();
                    return false;
                }
                picByteArray = picByteArray2;
            }
            else
            {
                picByteArray = picByteArray1;
            }
        }
        while (picByteArray.length() != jpegPicStreamLength)
        {
            picByteArray.append((char)0x00);
        }
        int result = snapmaticStream.write(picByteArray);
        if (result != 0)
        {
            if (cacheEnabled)
            {
                cachePicture = picture;
            }
            return true;
        }
        return false;
    }
    return false;
}

bool SnapmaticPicture::exportPicture(const QString &fileName)
{
    QFile *picFile = new QFile(fileName);
    if (picFile->open(QIODevice::WriteOnly))
    {
        picFile->write(rawPicContent);
        picFile->close();
        picFile->deleteLater();
        return true;
    }
    else
    {
        return false;
    }
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
    if (cacheEnabled)
    {
        return cachePicture;
    }
    else if (writeEnabled)
    {
        bool returnOk;
        QImage returnPicture;

        QBuffer snapmaticStream(&rawPicContent);
        snapmaticStream.open(QIODevice::ReadOnly);
        if (snapmaticStream.seek(jpegStreamEditorBegin))
        {
            QByteArray jpegRawContent = snapmaticStream.read(jpegPicStreamLength);
            returnOk = returnPicture.loadFromData(jpegRawContent, "JPEG");
        }
        snapmaticStream.close();

        if (returnOk)
        {
            return returnPicture;
        }
    }
    else
    {
        bool returnOk;
        QImage returnPicture;
        QIODevice *picStream;

        QFile *picFile = new QFile(picFileName);
        if (!picFile->open(QFile::ReadOnly))
        {
            lastStep = "1;/1,OpenFile," + StringParser::convertDrawStringForLog(picFileName);
            picFile->deleteLater();
            delete picFile;
            return QImage(0, 0, QImage::Format_RGB32);
        }
        rawPicContent = picFile->read(snapmaticFileMaxSize);
        picFile->close();
        delete picFile;

        picStream = new QBuffer(&rawPicContent);
        picStream->open(QIODevice::ReadWrite);
        if (picStream->seek(jpegStreamEditorBegin))
        {
            QByteArray jpegRawContent = picStream->read(jpegPicStreamLength);
            returnOk = returnPicture.loadFromData(jpegRawContent, "JPEG");
        }
        picStream->close();
        delete picStream;

        if (returnOk)
        {
            return returnPicture;
        }
    }
    return QImage(0, 0, QImage::Format_RGB32);
}

bool SnapmaticPicture::isPicOk()
{
    return picOk;
}

void SnapmaticPicture::setPicFileName(QString picFileName_)
{
    picFileName = picFileName_;
}

void SnapmaticPicture::clearCache()
{
    cacheEnabled = false;
    cachePicture = QImage(0, 0, QImage::Format_RGB32);
}

// JSON part

void SnapmaticPicture::parseJsonContent()
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonStr.toUtf8());
    QJsonObject jsonObject = jsonDocument.object();

    if (jsonObject.contains("loc"))
    {
        QJsonObject locObject = jsonObject["loc"].toObject();
        if (locObject.contains("x")) { localSpJson.location.x = locObject["x"].toDouble(); }
        if (locObject.contains("y")) { localSpJson.location.y = locObject["y"].toDouble(); }
        if (locObject.contains("z")) { localSpJson.location.z = locObject["z"].toDouble(); }
    }
    if (jsonObject.contains("area"))
    {
        localSpJson.area = jsonObject["area"].toString();
    }
    if (jsonObject.contains("crewid"))
    {
        localSpJson.crewID = jsonObject["crewid"].toInt();
    }
    if (jsonObject.contains("creat"))
    {
        QDateTime createdTimestamp;
        localSpJson.createdTimestamp = jsonObject["creat"].toVariant().toUInt();
        createdTimestamp.setTime_t(localSpJson.createdTimestamp);
        localSpJson.createdDateTime = createdTimestamp;
    }
    if (jsonObject.contains("plyrs"))
    {
        localSpJson.playersList = jsonObject["plyrs"].toVariant().toStringList();
    }
    if (jsonObject.contains("meme"))
    {
        localSpJson.isMeme = jsonObject["meme"].toBool();
    }
    if (jsonObject.contains("mug"))
    {
        localSpJson.isMug = jsonObject["mug"].toBool();
    }
    if (jsonObject.contains("slf"))
    {
        localSpJson.isSelfie = jsonObject["slf"].toBool();
    }
    if (jsonObject.contains("drctr"))
    {
        localSpJson.isFromDirector = jsonObject["drctr"].toBool();
    }
    if (jsonObject.contains("rsedtr"))
    {
        localSpJson.isFromRSEditor = jsonObject["rsedtr"].toBool();
    }

    jsonOk = true;
}

bool SnapmaticPicture::isJsonOk()
{
    return jsonOk;
}

QString SnapmaticPicture::getJsonStr()
{
    return jsonStr;
}

SnapmaticProperties SnapmaticPicture::getSnapmaticProperties()
{
    return localSpJson;
}

bool SnapmaticPicture::setSnapmaticProperties(SnapmaticProperties newSpJson)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonStr.toUtf8());
    QJsonObject jsonObject = jsonDocument.object();

    QJsonObject locObject;
    locObject["x"] = newSpJson.location.x;
    locObject["y"] = newSpJson.location.y;
    locObject["z"] = newSpJson.location.z;

    jsonObject["loc"] = locObject;
    jsonObject["area"] = newSpJson.area;
    jsonObject["crewid"] = newSpJson.crewID;
    jsonObject["creat"] = QJsonValue::fromVariant(newSpJson.createdTimestamp);
    jsonObject["plyrs"] = QJsonValue::fromVariant(newSpJson.playersList);
    jsonObject["meme"] = newSpJson.isMeme;
    jsonObject["mug"] = newSpJson.isMug;
    jsonObject["slf"] = newSpJson.isSelfie;
    jsonObject["drctr"] = newSpJson.isFromDirector;
    jsonObject["rsedtr"] = newSpJson.isFromRSEditor;

    jsonDocument.setObject(jsonObject);

    QString newJsonStr = QString::fromUtf8(jsonDocument.toJson(QJsonDocument::Compact));
    if (newJsonStr.length() < jsonStreamEditorLength)
    {
        if (writeEnabled)
        {
            QByteArray jsonByteArray = newJsonStr.toUtf8();
            while (jsonByteArray.length() != jsonStreamEditorLength)
            {
                jsonByteArray.append((char)0x00);
            }
            QBuffer snapmaticStream(&rawPicContent);
            snapmaticStream.open(QIODevice::ReadWrite);
            if (!snapmaticStream.seek(jsonStreamEditorBegin))
            {
                snapmaticStream.close();
                return false;
            }
            int result = snapmaticStream.write(jsonByteArray);
            snapmaticStream.close();
            if (result != 0)
            {
                localSpJson = newSpJson;
                jsonStr = newJsonStr;
                return true;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

// VISIBILITY

bool SnapmaticPicture::isHidden()
{
    if (picFileName.right(7) == ".hidden")
    {
        return true;
    }
    return false;
}

bool SnapmaticPicture::setPictureHidden()
{
    if (!isHidden())
    {
        QString newPicFileName = QString(picFileName + ".hidden");
        if (QFile::rename(picFileName, newPicFileName))
        {
            picFileName = newPicFileName;
            return true;
        }
        return false;
    }
    return true;
}

bool SnapmaticPicture::setPictureVisible()
{
    if (isHidden())
    {
        QString newPicFileName = QString(picFileName).remove(picFileName.length() - 7, 7);
        if (QFile::rename(picFileName, newPicFileName))
        {
            picFileName = newPicFileName;
            return true;
        }
        return false;
    }
    return true;
}
