/*****************************************************************************
* gta5sync-spv Grand Theft Auto Snapmatic Picture Viewer
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

#include "SnapmaticPicture.h"
#include "StringParser.h"
#include <QStringBuilder>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QVariantMap>
#include <QJsonArray>
#include <QFileInfo>
#include <QPainter>
#include <QString>
#include <QBuffer>
#include <QDebug>
#include <QImage>
#include <QSize>
#include <QFile>

// PARSER ALLOCATIONS
#define snapmaticHeaderLength 278
#define snapmaticUsefulLength 260
#define snapmaticFileMaxSize 528192
#define jpegHeaderLineDifStr 2
#define jpegPreHeaderLength 14
#define jpegPicStreamLength 524288
#define jsonStreamLength 3076
#define tideStreamLength 260

// EDITOR ALLOCATIONS
#define jpegStreamEditorBegin 292
#define jsonStreamEditorBegin 524588
#define jsonStreamEditorLength 3072
#define titlStreamEditorBegin 527668
#define titlStreamEditorLength 256
#define titlStreamCharacterMax 39

// IMAGES VALUES
#define snapmaticResolutionW 960
#define snapmaticResolutionH 536
#define snapmaticResolution QSize(snapmaticResolutionW, snapmaticResolutionH)

SnapmaticPicture::SnapmaticPicture(const QString &fileName, QObject *parent) : QObject(parent), picFilePath(fileName)
{
    reset();
}

SnapmaticPicture::~SnapmaticPicture()
{
}

void SnapmaticPicture::reset()
{
    // INIT PIC
    rawPicContent = "";
    cachePicture = QImage();
    jpegRawContentSizeE = 0;
    jpegRawContentSize = 0;
    picExportFileName = "";
    isCustomFormat = 0;
    isLoadedInRAM = 0;
    pictureHead = "";
    pictureStr = "";
    lowRamMode = 0;
    lastStep = "";
    sortStr = "";
    titlStr = "";
    descStr = "";
    picOk = 0;

    // INIT JSON
    jsonOk = 0;
    jsonStr = "";

    // SNAPMATIC PROPERTIES
    localSpJson = {};
}

bool SnapmaticPicture::preloadFile()
{
    QFile *picFile = new QFile(picFilePath);
    picFileName = QFileInfo(picFilePath).fileName();

    if (!picFile->open(QFile::ReadOnly))
    {
        lastStep = "1;/1,OpenFile," % StringParser::convertDrawStringForLog(picFilePath);
        delete picFile;
        return false;
    }
    if (picFilePath.right(4) != QLatin1String(".g5e"))
    {
        rawPicContent = picFile->read(snapmaticFileMaxSize);
        picFile->close();
        delete picFile;

        // Setting is values
        isCustomFormat = false;
        isLoadedInRAM = true;
    }
    else
    {
        QByteArray g5eContent = picFile->read(snapmaticFileMaxSize + 1024);
        picFile->close();
        delete picFile;

        // Set Custom Format
        isCustomFormat = true;

        // Reading g5e Content
        g5eContent.remove(0, 1);
        if (g5eContent.left(3) == QByteArray("G5E"))
        {
            g5eContent.remove(0, 3);
            if (g5eContent.left(2).toHex() == QByteArray("1000"))
            {
                g5eContent.remove(0, 2);
                if (g5eContent.left(3) == QByteArray("LEN"))
                {
                    g5eContent.remove(0, 3);
                    int fileNameLength = g5eContent.left(1).toHex().toInt();
                    g5eContent.remove(0, 1);
                    if (g5eContent.left(3) == QByteArray("FIL"))
                    {
                        g5eContent.remove(0, 3);
                        picFileName = g5eContent.left(fileNameLength);
                        g5eContent.remove(0, fileNameLength);
                        if (g5eContent.left(3) == QByteArray("COM"))
                        {
                            g5eContent.remove(0, 3);
                            rawPicContent = qUncompress(g5eContent);

                            // Setting is values
                            isLoadedInRAM = true;
                        }
                        else
                        {
                            lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",4,G5E_FORMATERROR";
                            return false;
                        }
                    }
                    else
                    {
                        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",3,G5E_FORMATERROR";
                        return false;
                    }
                }
                else
                {
                    lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",2,G5E_FORMATERROR";
                    return false;
                }
            }
            else
            {
                lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",1,G5E_NOTCOMPATIBLE";
                return false;
            }
        }
        else
        {
            lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",1,G5E_FORMATERROR";
            return false;
        }
    }
    emit preloaded();
    return true;
}

bool SnapmaticPicture::readingPicture(bool writeEnabled_, bool cacheEnabled_, bool fastLoad, bool lowRamMode_)
{
    // Start opening file
    // lastStep is like currentStep

    // Set boolean values
    writeEnabled = writeEnabled_;
    cacheEnabled = cacheEnabled_;
    lowRamMode = lowRamMode_;
    if (!writeEnabled) { lowRamMode = false; } // Low RAM Mode only works when writeEnabled is true

    QIODevice *picStream;

    if (!isLoadedInRAM) { preloadFile(); }

    picStream = new QBuffer(&rawPicContent);
    picStream->open(QIODevice::ReadWrite);

    // Reading Snapmatic Header
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",1,NOHEADER";
        picStream->close();
        delete picStream;
        return false;
    }
    QByteArray snapmaticHeaderLine = picStream->read(snapmaticHeaderLength);
    pictureHead = getSnapmaticHeaderString(snapmaticHeaderLine);

    // Reading JPEG Header Line
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",2,NOHEADER";
        picStream->close();
        delete picStream;
        return false;
    }
    QByteArray jpegHeaderLine = picStream->read(jpegPreHeaderLength);

    // Checking for JPEG
    jpegHeaderLine.remove(0, jpegHeaderLineDifStr);
    if (jpegHeaderLine.left(4) != QByteArray("JPEG"))
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",2,NOJPEG";
        picStream->close();
        delete picStream;
        return false;
    }

    // Read JPEG Stream
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",2,NOPIC";
        picStream->close();
        delete picStream;
        return false;
    }
    QByteArray jpegRawContent = picStream->read(jpegPicStreamLength);
    if (jpegRawContent.contains("\xFF\xD9"))
    {
        int jpegRawContentSizeT = jpegRawContent.indexOf("\xFF\xD9") + 2;
        jpegRawContentSizeE = jpegRawContentSizeT;
        jpegRawContentSize = jpegRawContentSizeT;
        if (jpegRawContent.contains("\xFF\x45\x4F\x49"))
        {
            jpegRawContentSizeT = jpegRawContent.indexOf("\xFF\x45\x4F\x49");
        }
        jpegRawContent = jpegRawContent.left(jpegRawContentSize);
        jpegRawContentSize = jpegRawContentSizeT;
    }
    if (cacheEnabled) picOk = cachePicture.loadFromData(jpegRawContent, "JPEG");
    if (!cacheEnabled)
    {
        QImage tempPicture;
        picOk = tempPicture.loadFromData(jpegRawContent, "JPEG");
    }
    else if (!fastLoad)
    {
        QImage tempPicture = QImage(snapmaticResolution, QImage::Format_RGB888);
        QPainter tempPainter(&tempPicture);
        if (cachePicture.size() == snapmaticResolution)
        {
            tempPainter.drawImage(0, 0, cachePicture);
        }
        else
        {
            tempPainter.drawImage(0, 0, cachePicture.scaled(snapmaticResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
        tempPainter.end();
        cachePicture = tempPicture;
    }

    // Read JSON Stream
    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",3,NOJSON";
        picStream->close();
        delete picStream;
        return false;
    }
    else if (picStream->read(4) != QByteArray("JSON"))
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",3,CTJSON";
        picStream->close();
        delete picStream;
        return false;
    }
    QByteArray jsonRawContent = picStream->read(jsonStreamLength);
    jsonStr = getSnapmaticJSONString(jsonRawContent);
    parseJsonContent(); // JSON parsing is own function

    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",4,NOTITL";
        picStream->close();
        delete picStream;
        return false;
    }
    else if (picStream->read(4) != QByteArray("TITL"))
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",4,CTTITL";
        picStream->close();
        delete picStream;
        return false;
    }
    QByteArray titlRawContent = picStream->read(tideStreamLength);
    titlStr = getSnapmaticTIDEString(titlRawContent);

    if (!picStream->isReadable())
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",5,NODESC";
        picStream->close();
        delete picStream;
        return picOk;
    }
    else if (picStream->read(4) != QByteArray("DESC"))
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",5,CTDESC";
        picStream->close();
        delete picStream;
        return false;
    }
    QByteArray descRawContent = picStream->read(tideStreamLength);
    descStr = getSnapmaticTIDEString(descRawContent);

    updateStrings();

    picStream->close();
    delete picStream;

    if (!writeEnabled) { rawPicContent.clear(); }
    else if (lowRamMode) { rawPicContent = qCompress(rawPicContent, 9); }

    emit loaded();
    return picOk;
}

QString SnapmaticPicture::getSnapmaticHeaderString(const QByteArray &snapmaticHeader)
{
    QByteArray snapmaticBytes = snapmaticHeader.left(snapmaticUsefulLength);
    QList<QByteArray> snapmaticBytesList = snapmaticBytes.split('\x01');
    snapmaticBytes = snapmaticBytesList.at(1);
    snapmaticBytesList.clear();
    return StringParser::parseTitleString(snapmaticBytes, snapmaticBytes.length());
}

QString SnapmaticPicture::getSnapmaticJSONString(const QByteArray &jsonBytes)
{
    QByteArray jsonUsefulBytes = jsonBytes;
    jsonUsefulBytes.replace('\x00', "");
    jsonUsefulBytes.replace('\x0c', "");
    return QString::fromUtf8(jsonUsefulBytes).trimmed();
}

QString SnapmaticPicture::getSnapmaticTIDEString(const QByteArray &tideBytes)
{
    QByteArray tideUsefulBytes = tideBytes;
    tideUsefulBytes.remove(0,4);
    QList<QByteArray> tideUsefulBytesList = tideUsefulBytes.split('\x00');
    return QString::fromUtf8(tideUsefulBytesList.at(0)).trimmed();
}

void SnapmaticPicture::updateStrings()
{
    QString cmpPicTitl = titlStr;
    cmpPicTitl.replace('\"', "''");
    cmpPicTitl.replace(' ', '_');
    cmpPicTitl.replace(':', '-');
    cmpPicTitl.remove('\\');
    cmpPicTitl.remove('{');
    cmpPicTitl.remove('}');
    cmpPicTitl.remove('/');
    cmpPicTitl.remove('<');
    cmpPicTitl.remove('>');
    cmpPicTitl.remove('*');
    cmpPicTitl.remove('?');
    cmpPicTitl.remove('.');
    pictureStr = tr("PHOTO - %1").arg(localSpJson.createdDateTime.toString("MM/dd/yy HH:mm:ss"));
    sortStr = localSpJson.createdDateTime.toString("yyMMddHHmmss") % QString::number(localSpJson.uid);
    picExportFileName = sortStr % "_" % cmpPicTitl;
}

bool SnapmaticPicture::readingPictureFromFile(const QString &fileName, bool writeEnabled_, bool cacheEnabled_, bool fastLoad, bool lowRamMode_)
{
    if (!fileName.isEmpty())
    {
        picFilePath = fileName;
        return readingPicture(writeEnabled_, cacheEnabled_, fastLoad, lowRamMode_);
    }
    else
    {
        return false;
    }
}

bool SnapmaticPicture::setImage(const QImage &picture) // dirty method
{
    if (writeEnabled)
    {
        QByteArray picByteArray;
        int comLvl = 100;
        bool saveSuccess = false;
        while (comLvl != 0 && !saveSuccess)
        {
            QByteArray picByteArrayT;
            QBuffer picStreamT(&picByteArrayT);
            picStreamT.open(QIODevice::WriteOnly);
            saveSuccess = picture.save(&picStreamT, "JPEG", comLvl);
            picStreamT.close();
            if (saveSuccess)
            {
                if (picByteArrayT.length() > jpegRawContentSize)
                {
                    comLvl--;
                    saveSuccess = false;
                }
                else
                {
                    picByteArray = picByteArrayT;
                }
            }
        }
        if (saveSuccess) return setPictureStream(picByteArray);
    }
    return false;
}

bool SnapmaticPicture::setPictureStream(const QByteArray &picByteArray_) // clean method
{
    if (writeEnabled)
    {
        bool customEOI = false;
        QByteArray picByteArray = picByteArray_;
        if (lowRamMode) { rawPicContent = qUncompress(rawPicContent); }
        QBuffer snapmaticStream(&rawPicContent);
        snapmaticStream.open(QIODevice::ReadWrite);
        if (!snapmaticStream.seek(jpegStreamEditorBegin)) return false;
        if (picByteArray.length() > jpegPicStreamLength) return false;
        if (picByteArray.length() < jpegRawContentSize && jpegRawContentSize + 4 < jpegPicStreamLength)
        {
            customEOI = true;
        }
        while (picByteArray.length() != jpegPicStreamLength)
        {
            picByteArray += '\x00';
        }
        if (customEOI)
        {
            picByteArray.replace(jpegRawContentSize, 4, "\xFF\x45\x4F\x49");
        }
        int result = snapmaticStream.write(picByteArray);
        snapmaticStream.close();
        if (result != 0)
        {
            if (cacheEnabled)
            {
                QImage replacedPicture;
                replacedPicture.loadFromData(picByteArray);
                cachePicture = replacedPicture;
            }
            if (lowRamMode) { rawPicContent = qCompress(rawPicContent, 9); }
            return true;
        }
        if (lowRamMode) { rawPicContent = qCompress(rawPicContent, 9); }
        return false;
    }
    return false;
}

bool SnapmaticPicture::setPictureTitl(const QString &newTitle_)
{
    if (writeEnabled)
    {
        QString newTitle = newTitle_;
        if (lowRamMode) { rawPicContent = qUncompress(rawPicContent); }
        QBuffer snapmaticStream(&rawPicContent);
        snapmaticStream.open(QIODevice::ReadWrite);
        if (!snapmaticStream.seek(titlStreamEditorBegin)) return false;
        if (newTitle.length() > titlStreamCharacterMax)
        {
            newTitle = newTitle.left(titlStreamCharacterMax);
        }
        QByteArray newTitleArray = newTitle.toUtf8();
        while (newTitleArray.length() != titlStreamEditorLength)
        {
            newTitleArray += '\x00';
        }
        int result = snapmaticStream.write(newTitleArray);
        snapmaticStream.close();
        if (result != 0)
        {
            titlStr = newTitle;
            if (lowRamMode) { rawPicContent = qCompress(rawPicContent, 9); }
            return true;
        }
        if (lowRamMode) { rawPicContent = qCompress(rawPicContent, 9); }
        return false;
    }
    return false;
}

QString SnapmaticPicture::getExportPictureFileName()
{
    return picExportFileName;
}

QString SnapmaticPicture::getPictureFileName()
{
    return picFileName;
}

QString SnapmaticPicture::getPictureFilePath()
{
    return picFilePath;
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

QString SnapmaticPicture::getPictureHead()
{
    return pictureHead;
}

QString SnapmaticPicture::getPictureStr()
{
    return pictureStr;
}

QString SnapmaticPicture::getLastStep()
{
    return lastStep;
}

QImage SnapmaticPicture::getImage()
{
    if (cacheEnabled)
    {
        return cachePicture;
    }
    else if (writeEnabled)
    {
        bool returnOk = 0;
        QImage tempPicture;
        QImage returnPicture(snapmaticResolution, QImage::Format_RGB888);

        if (lowRamMode) { rawPicContent = qUncompress(rawPicContent); }
        QBuffer snapmaticStream(&rawPicContent);
        snapmaticStream.open(QIODevice::ReadOnly);
        if (snapmaticStream.seek(jpegStreamEditorBegin))
        {
            QByteArray jpegRawContent = snapmaticStream.read(jpegPicStreamLength);
            returnOk = tempPicture.loadFromData(jpegRawContent, "JPEG");
        }
        snapmaticStream.close();
        if (lowRamMode) { rawPicContent = qCompress(rawPicContent, 9); }

        if (returnOk)
        {
            QPainter returnPainter(&returnPicture);
            if (tempPicture.size() == snapmaticResolution)
            {
                returnPainter.drawImage(0, 0, tempPicture);
            }
            else
            {
                returnPainter.drawImage(0, 0, tempPicture.scaled(snapmaticResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            }
            returnPainter.end();
            return returnPicture;
        }
    }
    else
    {
        bool returnOk = 0;
        QImage returnPicture;
        QIODevice *picStream;

        QFile *picFile = new QFile(picFilePath);
        if (!picFile->open(QFile::ReadOnly))
        {
            lastStep = "1;/1,OpenFile," % StringParser::convertDrawStringForLog(picFilePath);
            delete picFile;
            return QImage(0, 0, QImage::Format_RGB888);
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
    return QImage(0, 0, QImage::Format_RGB888);
}

int SnapmaticPicture::getContentMaxLength()
{
    return jpegRawContentSize;
}

bool SnapmaticPicture::isPicOk()
{
    return picOk;
}

void SnapmaticPicture::clearCache()
{
    cacheEnabled = false;
    cachePicture = QImage();
}

void SnapmaticPicture::emitUpdate()
{
    emit updated();
}

// JSON part

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

void SnapmaticPicture::parseJsonContent()
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonStr.toUtf8());
    QJsonObject jsonObject = jsonDocument.object();
    QVariantMap jsonMap = jsonObject.toVariantMap(); // backward compatibility

    if (jsonObject.contains("loc"))
    {
        QJsonObject locObject = jsonObject["loc"].toObject();
        if (locObject.contains("x")) { localSpJson.location.x = locObject["x"].toDouble(); }
        if (locObject.contains("y")) { localSpJson.location.y = locObject["y"].toDouble(); }
        if (locObject.contains("z")) { localSpJson.location.z = locObject["z"].toDouble(); }
    }
    if (jsonObject.contains("uid"))
    {
        localSpJson.uid = jsonObject["uid"].toInt();
    }
    if (jsonObject.contains("area"))
    {
        localSpJson.location.area = jsonObject["area"].toString();
    }
    if (jsonObject.contains("crewid"))
    {
        localSpJson.crewID = jsonObject["crewid"].toInt();
    }
    if (jsonObject.contains("creat"))
    {
        QDateTime createdTimestamp;
        localSpJson.createdTimestamp = jsonMap["creat"].toUInt();
        createdTimestamp.setTime_t(localSpJson.createdTimestamp);
        localSpJson.createdDateTime = createdTimestamp;
    }
    if (jsonObject.contains("plyrs"))
    {
        localSpJson.playersList = jsonMap["plyrs"].toStringList();
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

bool SnapmaticPicture::setSnapmaticProperties(SnapmaticProperties newSpJson)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonStr.toUtf8());
    QJsonObject jsonObject = jsonDocument.object();

    QJsonObject locObject;
    locObject["x"] = newSpJson.location.x;
    locObject["y"] = newSpJson.location.y;
    locObject["z"] = newSpJson.location.z;

    jsonObject["loc"] = locObject;
    jsonObject["uid"] = newSpJson.uid;
    jsonObject["area"] = newSpJson.location.area;
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
                jsonByteArray += '\x00';
            }
            if (lowRamMode) { rawPicContent = qUncompress(rawPicContent); }
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
                if (lowRamMode) { rawPicContent = qCompress(rawPicContent, 9); }
                return true;
            }
            else
            {
                if (lowRamMode) { rawPicContent = qCompress(rawPicContent, 9); }
                return false;
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

// FILE MANAGEMENT

bool SnapmaticPicture::exportPicture(const QString &fileName, const QString format)
{
    QFile *picFile = new QFile(fileName);
    if (picFile->open(QIODevice::WriteOnly))
    {
        if (format == QLatin1String("G5E"))
        {
            // Modern compressed export
            QByteArray stockFileNameUTF8 = picFileName.toUtf8();
            QByteArray numberLength = QByteArray::number(stockFileNameUTF8.length());
            if (numberLength.length() == 1)
            {
                numberLength.insert(0, "0");
            }
            else if (numberLength.length() != 2)
            {
                numberLength = "00";
            }
            QByteArray g5eHeader;
            g5eHeader.reserve(stockFileNameUTF8.length() + 16);
            g5eHeader += '\x00'; // First Null Byte
            g5eHeader += QByteArray("G5E"); // GTA 5 Export
            g5eHeader += '\x10'; g5eHeader += '\x00'; // 2 byte GTA 5 Export Version
            g5eHeader += QByteArray("LEN"); // Before Length
            g5eHeader += QByteArray::fromHex(numberLength); // Length in HEX before Compressed
            g5eHeader += QByteArray("FIL"); // Before File Name
            g5eHeader += stockFileNameUTF8; // File Name
            g5eHeader += QByteArray("COM"); // Before Compressed
            picFile->write(g5eHeader);
            if (!lowRamMode)
            {
                picFile->write(qCompress(rawPicContent, 9)); // Compressed Snapmatic
            }
            else
            {
                picFile->write(rawPicContent);
            }
            picFile->close();
            delete picFile;
        }
        else if (format == QLatin1String("JPG"))
        {
            // JPEG export
            QBuffer snapmaticStream(&rawPicContent);
            snapmaticStream.open(QIODevice::ReadOnly);
            if (snapmaticStream.seek(jpegStreamEditorBegin))
            {
                QByteArray jpegRawContent = snapmaticStream.read(jpegPicStreamLength);
                if (jpegRawContentSizeE != 0)
                {
                    jpegRawContent = jpegRawContent.left(jpegRawContentSizeE);
                }
                picFile->write(jpegRawContent);
            }
            picFile->close();
            delete picFile;
        }
        else
        {
            // Classic straight export
            if (!lowRamMode)
            {
                picFile->write(rawPicContent);
            }
            else
            {
                picFile->write(qUncompress(rawPicContent));
            }
            picFile->close();
            delete picFile;
        }
        return true;
    }
    else
    {
        delete picFile;
        return false;
    }
}

void SnapmaticPicture::setPicFileName(const QString &picFileName_)
{
    picFileName = picFileName_;
}

void SnapmaticPicture::setPicFilePath(const QString &picFilePath_)
{
    picFilePath = picFilePath_;
}

bool SnapmaticPicture::deletePicFile()
{
    if (!QFile::exists(picFilePath)) return true;
    if (QFile::remove(picFilePath)) return true;
    return false;
}

// VISIBILITY

bool SnapmaticPicture::isHidden()
{
    if (picFilePath.right(7) == QLatin1String(".hidden"))
    {
        return true;
    }
    return false;
}

bool SnapmaticPicture::setPictureHidden()
{
    if (isCustomFormat)
    {
        return false;
    }
    if (!isHidden())
    {
        QString newPicFilePath = QString(picFilePath % ".hidden");
        if (QFile::rename(picFilePath, newPicFilePath))
        {
            picFilePath = newPicFilePath;
            return true;
        }
        return false;
    }
    return true;
}

bool SnapmaticPicture::setPictureVisible()
{
    if (isCustomFormat)
    {
        return false;
    }
    if (isHidden())
    {
        QString newPicFilePath = QString(picFilePath).remove(picFilePath.length() - 7, 7);
        if (QFile::rename(picFilePath, newPicFilePath))
        {
            picFilePath = newPicFilePath;
            return true;
        }
        return false;
    }
    return true;
}

// PREDEFINED PROPERTIES

QSize SnapmaticPicture::getSnapmaticResolution()
{
    return snapmaticResolution;
}

// VERIFY CONTENT

bool SnapmaticPicture::verifyTitle(const QString &title)
{
    // VERIFY TITLE FOR BE A VALID SNAPMATIC TITLE
    if (title.length() <= titlStreamCharacterMax)
    {
        foreach(const QChar &titleChar, title)
        {
            if (!verifyTitleChar(titleChar)) return false;
        }
        return true;
    }
    return false;
}

bool SnapmaticPicture::verifyTitleChar(const QChar &titleChar)
{
    // VERIFY CHAR FOR BE A VALID SNAPMATIC CHARACTER
    if (titleChar.isLetterOrNumber() || titleChar.isPrint())
    {
        if (titleChar == '<' || titleChar == '>' || titleChar == '\\') return false;
        return true;
    }
    return false;
}
