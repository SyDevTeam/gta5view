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

#if QT_VERSION >= 0x050000
#include <QSaveFile>
#else
#include "StandardPaths.h"
#endif

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
    rawPicContent.clear();
    rawPicContent.squeeze();
    cachePicture = QImage();
    picExportFileName = QString();
    pictureHead = QString();
    pictureStr = QString();
    lastStep = QString();
    sortStr = QString();
    titlStr = QString();
    descStr = QString();

    // INIT PIC INTS
    jpegRawContentSizeE = 0;
    jpegRawContentSize = 0;

    // INIT PIC BOOLS
    isCustomFormat = false;
    isFormatSwitch = false;
    isLoadedInRAM = false;
    lowRamMode = false;
    picOk = false;

    // INIT JSON
    jsonOk = false;
    jsonStr = QString();

    // SNAPMATIC DEFAULTS
#ifdef GTA5SYNC_CSDF
    careSnapDefault = false;
#else
    careSnapDefault = true;
#endif

    // SNAPMATIC PROPERTIES
    localProperties = {};
}

bool SnapmaticPicture::preloadFile()
{
    QFile *picFile = new QFile(picFilePath);
    picFileName = QFileInfo(picFilePath).fileName();

    bool g5eMode = false;
    isFormatSwitch = false;

    if (!picFile->open(QFile::ReadOnly))
    {
        lastStep = "1;/1,OpenFile," % StringParser::convertDrawStringForLog(picFilePath);
        delete picFile;
        return false;
    }
    if (picFilePath.right(4) != QLatin1String(".g5e"))
    {
        rawPicContent = picFile->read(snapmaticFileMaxSize + 1024);
        picFile->close();
        delete picFile;

        if (rawPicContent.mid(1, 3) == QByteArray("G5E"))
        {
            isFormatSwitch = true;
        }
        else
        {
            isCustomFormat = false;
            isLoadedInRAM = true;
        }
    }
    else
    {
        g5eMode = true;
    }
    if (g5eMode || isFormatSwitch)
    {
        QByteArray g5eContent;
        if (!isFormatSwitch)
        {
            g5eContent = picFile->read(snapmaticFileMaxSize + 1024);
            picFile->close();
            delete picFile;
        }
        else
        {
            g5eContent = rawPicContent;
            rawPicContent.clear();
        }

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
    if (pictureHead == QLatin1String("MALFORMED"))
    {
        lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",1,MALFORMEDHEADER";
        picStream->close();
        delete picStream;
        return false;
    }

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
        if (careSnapDefault)
        {
            QImage tempPicture = QImage(snapmaticResolution, QImage::Format_RGB888);
            QPainter tempPainter(&tempPicture);
            if (cachePicture.size() != snapmaticResolution)
            {
                tempPainter.drawImage(0, 0, cachePicture.scaled(snapmaticResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            }
            else
            {
                tempPainter.drawImage(0, 0, cachePicture);
            }
            tempPainter.end();
            cachePicture = tempPicture;
        }
        else
        {
            QImage tempPicture = QImage(cachePicture.size(), QImage::Format_RGB888);
            QPainter tempPainter(&tempPicture);
            tempPainter.drawImage(0, 0, cachePicture);
            tempPainter.end();
            cachePicture = tempPicture;
        }
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
    QList<QByteArray> snapmaticBytesList = snapmaticHeader.left(snapmaticUsefulLength).split('\x01');
    if (snapmaticBytesList.length() < 2) { return QLatin1String("MALFORMED"); }
    QByteArray snapmaticBytes = snapmaticBytesList.at(1);
    return StringParser::parseTitleString(snapmaticBytes, snapmaticBytes.length());
}

QString SnapmaticPicture::getSnapmaticJSONString(const QByteArray &jsonBytes)
{
    QByteArray jsonUsefulBytes = jsonBytes;
    jsonUsefulBytes.replace('\x00', QString());
    jsonUsefulBytes.replace('\x0c', QString());
    return QString::fromUtf8(jsonUsefulBytes.trimmed());
}

QString SnapmaticPicture::getSnapmaticTIDEString(const QByteArray &tideBytes)
{
    QByteArray tideUsefulBytes = tideBytes;
    tideUsefulBytes.remove(0,4);
    QList<QByteArray> tideUsefulBytesList = tideUsefulBytes.split('\x00');
    return QString::fromUtf8(tideUsefulBytesList.at(0).trimmed());
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
    pictureStr = tr("PHOTO - %1").arg(localProperties.createdDateTime.toString("MM/dd/yy HH:mm:ss"));
    sortStr = localProperties.createdDateTime.toString("yyMMddHHmmss") % QString::number(localProperties.uid);
    QString exportStr = localProperties.createdDateTime.toString("yyyyMMdd") % "-" % QString::number(localProperties.uid);
    picExportFileName = exportStr % "_" % cmpPicTitl;
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

bool SnapmaticPicture::setImage(const QImage &picture)
{
    if (writeEnabled)
    {
        QImage altPicture;
        bool useAltPicture = false;
        if (picture.size() != snapmaticResolution && careSnapDefault)
        {
            altPicture = picture.scaled(snapmaticResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            useAltPicture = true;
        }
        QByteArray picByteArray;
        int comLvl = 100;
        bool saveSuccess = false;
        while (comLvl != 0 && !saveSuccess)
        {
            QByteArray picByteArrayT;
            QBuffer picStreamT(&picByteArrayT);
            picStreamT.open(QIODevice::WriteOnly);
            if (useAltPicture) { saveSuccess = altPicture.save(&picStreamT, "JPEG", comLvl); }
            else { saveSuccess = picture.save(&picStreamT, "JPEG", comLvl); }
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
        if (saveSuccess) { return setPictureStream(picByteArray); }
    }
    return false;
}

bool SnapmaticPicture::setPictureStream(const QByteArray &streamArray) // clean method
{
    if (writeEnabled)
    {
        bool customEOI = false;
        QByteArray picByteArray = streamArray;
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

QString SnapmaticPicture::getOriginalPictureFileName()
{
    QString newPicFileName = picFileName;
    if (picFileName.right(4) == ".bak")
    {
        newPicFileName = QString(picFileName).remove(picFileName.length() - 4, 4);
    }
    if (picFileName.right(7) == ".hidden")
    {
        newPicFileName = QString(picFileName).remove(picFileName.length() - 7, 7);
    }
    return newPicFileName;
}

QString SnapmaticPicture::getOriginalPictureFilePath()
{
    QString newPicFilePath = picFilePath;
    if (picFilePath.right(4) == ".bak")
    {
        newPicFilePath = QString(picFilePath).remove(picFilePath.length() - 4, 4);
    }
    if (picFilePath.right(7) == ".hidden")
    {
        newPicFilePath = QString(picFilePath).remove(picFilePath.length() - 7, 7);
    }
    return newPicFilePath;
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

QString SnapmaticPicture::getLastStep(bool readable)
{
    if (readable)
    {
        QStringList lastStepList = lastStep.split(";/");
        if (lastStepList.length() < 2) { return lastStep; }
        bool intOk;
        //int stepNumber = lastStepList.at(0).toInt(&intOk);
        //if (!intOk) { return lastStep; }
        QStringList descStepList = lastStepList.at(1).split(",");
        if (descStepList.length() < 1) { return lastStep; }
        int argsCount = descStepList.at(0).toInt(&intOk);
        if (!intOk) { return lastStep; }
        if (argsCount == 1)
        {
            QString currentAction = descStepList.at(1);
            QString actionFile = descStepList.at(2);
            if (currentAction == "OpenFile")
            {
                return tr("open file %1").arg(actionFile);
            }
        }
        else if (argsCount == 3 || argsCount == 4)
        {
            QString currentAction = descStepList.at(1);
            QString actionFile = descStepList.at(2);
            //QString actionStep = descStepList.at(3);
            QString actionError = descStepList.at(4);
            QString actionError2;
            if (argsCount == 4) { actionError2 = descStepList.at(5); }
            if (currentAction == "ReadingFile")
            {
                QString readableError = actionError;
                if (actionError == "NOHEADER")
                {
                    readableError = tr("header not exists");
                }
                else if (actionError == "MALFORMEDHEADER")
                {
                    readableError = tr("header is malformed");
                }
                else if (actionError == "NOJPEG" || actionError == "NOPIC")
                {
                    readableError = tr("picture not exists (%1)").arg(actionError);
                }
                else if (actionError == "NOJSON" || actionError == "CTJSON")
                {
                    readableError = tr("JSON not exists (%1)").arg(actionError);
                }
                else if (actionError == "NOTITL" || actionError == "CTTITL")
                {
                    readableError = tr("title not exists (%1)").arg(actionError);
                }
                else if (actionError == "NODESC" || actionError == "CTDESC")
                {
                    readableError = tr("description not exists (%1)").arg(actionError);
                }
                else if (actionError == "JSONINCOMPLETE" && actionError2 == "JSONERROR")
                {
                    readableError = tr("JSON is incomplete and malformed");
                }
                else if (actionError == "JSONINCOMPLETE")
                {
                    readableError = tr("JSON is incomplete");
                }
                else if (actionError == "JSONERROR")
                {
                    readableError = tr("JSON is malformed");
                }
                return tr("reading file %1 because of %2", "Example for %2: JSON is malformed error").arg(actionFile, readableError);
            }
            else
            {
                return lastStep;
            }
        }
        else
        {
            return lastStep;
        }
    }
    return lastStep;

}

QImage SnapmaticPicture::getImage(bool fastLoad)
{
    if (cacheEnabled)
    {
        return cachePicture;
    }
    else if (writeEnabled)
    {
        bool fastLoadU = fastLoad;
        if (!careSnapDefault) { fastLoadU = true; }

        bool returnOk = false;
        QImage tempPicture;
        QImage returnPicture;
        if (!fastLoadU)
        {
            returnPicture = QImage(snapmaticResolution, QImage::Format_RGB888);
        }

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
            if (!fastLoadU)
            {
                QPainter returnPainter(&returnPicture);
                if (tempPicture.size() != snapmaticResolution)
                {
                    returnPainter.drawImage(0, 0, tempPicture.scaled(snapmaticResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                }
                else
                {
                    returnPainter.drawImage(0, 0, tempPicture);
                }
                returnPainter.end();
                return returnPicture;
            }
            else
            {
                return tempPicture;
            }
        }
    }
    else
    {
        bool fastLoadU = fastLoad;
        if (!careSnapDefault) { fastLoadU = true; }

        bool returnOk = false;
        QImage tempPicture;
        QImage returnPicture;
        if (!fastLoadU)
        {
            returnPicture = QImage(snapmaticResolution, QImage::Format_RGB888);
        }
        QIODevice *picStream;

        QFile *picFile = new QFile(picFilePath);
        if (!picFile->open(QFile::ReadOnly))
        {
            lastStep = "1;/1,OpenFile," % StringParser::convertDrawStringForLog(picFilePath);
            delete picFile;
            return QImage();
        }
        rawPicContent = picFile->read(snapmaticFileMaxSize);
        picFile->close();
        delete picFile;

        picStream = new QBuffer(&rawPicContent);
        picStream->open(QIODevice::ReadWrite);
        if (picStream->seek(jpegStreamEditorBegin))
        {
            QByteArray jpegRawContent = picStream->read(jpegPicStreamLength);
            returnOk = tempPicture.loadFromData(jpegRawContent, "JPEG");
        }
        picStream->close();
        delete picStream;

        if (returnOk)
        {
            if (!fastLoadU)
            {
                QPainter returnPainter(&returnPicture);
                if (tempPicture.size() != snapmaticResolution)
                {
                    returnPainter.drawImage(0, 0, tempPicture.scaled(snapmaticResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                }
                else
                {
                    returnPainter.drawImage(0, 0, tempPicture);
                }
                returnPainter.end();
                return returnPicture;
            }
            else
            {
                return tempPicture;
            }
        }
    }
    return QImage();
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
    return localProperties;
}

void SnapmaticPicture::parseJsonContent()
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonStr.toUtf8());
    QJsonObject jsonObject = jsonDocument.object();
    QVariantMap jsonMap = jsonObject.toVariantMap();

    bool jsonIncomplete = false;
    bool jsonError = false;
    if (jsonObject.contains("loc"))
    {
        if (jsonObject["loc"].isObject())
        {
            QJsonObject locObject = jsonObject["loc"].toObject();
            if (locObject.contains("x"))
            {
                if (locObject["x"].isDouble()) { localProperties.location.x = locObject["x"].toDouble(); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
            if (locObject.contains("y"))
            {
                if (locObject["y"].isDouble()) { localProperties.location.y = locObject["y"].toDouble(); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
            if (locObject.contains("z"))
            {
                if (locObject["z"].isDouble()) { localProperties.location.z = locObject["z"].toDouble(); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
        }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("uid"))
    {
        bool uidOk;
        localProperties.uid = jsonMap["uid"].toInt(&uidOk);
        if (!uidOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("area"))
    {
        if (jsonObject["area"].isString()) { localProperties.location.area = jsonObject["area"].toString(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("crewid"))
    {
        bool crewIDOk;
        localProperties.crewID = jsonMap["crewid"].toInt(&crewIDOk);
        if (!crewIDOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("street"))
    {
        bool streetIDOk;
        localProperties.streetID = jsonMap["street"].toInt(&streetIDOk);
        if (!streetIDOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("creat"))
    {
        bool timestampOk;
        QDateTime createdTimestamp;
        localProperties.createdTimestamp = jsonMap["creat"].toUInt(&timestampOk);
        createdTimestamp.setTime_t(localProperties.createdTimestamp);
        localProperties.createdDateTime = createdTimestamp;
        if (!timestampOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("plyrs"))
    {
        if (jsonObject["plyrs"].isArray()) { localProperties.playersList = jsonMap["plyrs"].toStringList(); }
        else { jsonError = true; }
    }
    // else { jsonIncomplete = true; } // 2016 Snapmatic pictures left out plyrs when none are captured, so don't force exists on that one
    if (jsonObject.contains("meme"))
    {
        if (jsonObject["meme"].isBool()) { localProperties.isMeme = jsonObject["meme"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("mug"))
    {
        if (jsonObject["mug"].isBool()) { localProperties.isMug = jsonObject["mug"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("slf"))
    {
        if (jsonObject["slf"].isBool()) { localProperties.isSelfie = jsonObject["slf"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("drctr"))
    {
        if (jsonObject["drctr"].isBool()) { localProperties.isFromDirector = jsonObject["drctr"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("rsedtr"))
    {
        if (jsonObject["rsedtr"].isBool()) { localProperties.isFromRSEditor = jsonObject["rsedtr"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }

    if (!jsonIncomplete && !jsonError)
    {
        jsonOk = true;
    }
    else
    {
        if (jsonIncomplete && jsonError)
        {
            lastStep = "2;/4,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",3,JSONINCOMPLETE,JSONERROR";
        }
        else if (jsonIncomplete)
        {
            lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",3,JSONINCOMPLETE";
        }
        else if (jsonError)
        {
            lastStep = "2;/3,ReadingFile," % StringParser::convertDrawStringForLog(picFilePath) % ",3,JSONERROR";
        }
        jsonOk = false;
    }
}

bool SnapmaticPicture::setSnapmaticProperties(SnapmaticProperties properties)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonStr.toUtf8());
    QJsonObject jsonObject = jsonDocument.object();

    QJsonObject locObject;
    locObject["x"] = properties.location.x;
    locObject["y"] = properties.location.y;
    locObject["z"] = properties.location.z;

    jsonObject["loc"] = locObject;
    jsonObject["uid"] = properties.uid;
    jsonObject["area"] = properties.location.area;
    jsonObject["crewid"] = properties.crewID;
    jsonObject["street"] = properties.streetID;
    jsonObject["creat"] = QJsonValue::fromVariant(properties.createdTimestamp);
    jsonObject["plyrs"] = QJsonValue::fromVariant(properties.playersList);
    jsonObject["meme"] = properties.isMeme;
    jsonObject["mug"] = properties.isMug;
    jsonObject["slf"] = properties.isSelfie;
    jsonObject["drctr"] = properties.isFromDirector;
    jsonObject["rsedtr"] = properties.isFromRSEditor;

    jsonDocument.setObject(jsonObject);

    if (setJsonStr(QString::fromUtf8(jsonDocument.toJson(QJsonDocument::Compact))))
    {
        localProperties = properties;
        return true;
    }
    return false;
}

bool SnapmaticPicture::setJsonStr(const QString &newJsonStr, bool updateProperties)
{
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
                jsonStr = newJsonStr;
                if (lowRamMode) { rawPicContent = qCompress(rawPicContent, 9); }
                if (updateProperties) { parseJsonContent(); }
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
    return false;
}

// FILE MANAGEMENT

bool SnapmaticPicture::exportPicture(const QString &fileName, SnapmaticFormat format_)
{
    // Keep current format when Auto_Format is used
    SnapmaticFormat format = format_;
    if (format_ == SnapmaticFormat::Auto_Format)
    {
        if (isCustomFormat)
        {
            format = SnapmaticFormat::G5E_Format;
        }
        else
        {
            format = SnapmaticFormat::PGTA_Format;
        }
    }

    bool saveSuccess = false;
    bool writeFailure = false;
#if QT_VERSION >= 0x050000
    QSaveFile *picFile = new QSaveFile(fileName);
#else
    QFile *picFile = new QFile(StandardPaths::tempLocation() % "/" % QFileInfo(fileName).fileName() % ".tmp");
#endif
    if (picFile->open(QIODevice::WriteOnly))
    {
        if (format == SnapmaticFormat::G5E_Format)
        {
            // Modern compressed export
            QByteArray stockFileNameUTF8 = picFileName.toUtf8();
            QByteArray numberLength = QByteArray::number(stockFileNameUTF8.length());
            if (numberLength.length() == 1)
            {
                numberLength.insert(0, '0');
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
            if (picFile->write(g5eHeader) == -1) { writeFailure = true; }
            if (!lowRamMode)
            {
                if (picFile->write(qCompress(rawPicContent, 9)) == -1) { writeFailure = true; } // Compressed Snapmatic
            }
            else
            {
                if (picFile->write(rawPicContent) == -1) { writeFailure = true; }
            }
#if QT_VERSION >= 0x050000
            if (writeFailure) { picFile->cancelWriting(); }
            else { saveSuccess = picFile->commit(); }
#else
            if (!writeFailure) { saveSuccess = true; }
            picFile->close();
#endif
            delete picFile;
        }
        else if (format == SnapmaticFormat::JPEG_Format)
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
                if (picFile->write(jpegRawContent) == -1) { writeFailure = true; }
#if QT_VERSION >= 0x050000
                if (writeFailure) { picFile->cancelWriting(); }
                else { saveSuccess = picFile->commit(); }
#else
                if (!writeFailure) { saveSuccess = true; }
                picFile->close();
#endif
            }
            delete picFile;
        }
        else
        {
            // Classic straight export
            if (!lowRamMode)
            {
                if (picFile->write(rawPicContent) == -1) { writeFailure = true; }
            }
            else
            {
                if (picFile->write(qUncompress(rawPicContent)) == -1) { writeFailure = true; }
            }
#if QT_VERSION >= 0x050000
            if (writeFailure) { picFile->cancelWriting(); }
            else { saveSuccess = picFile->commit(); }
#else
            if (!writeFailure) { saveSuccess = true; }
            picFile->close();
#endif
            delete picFile;
        }
#if QT_VERSION <= 0x050000
        if (saveSuccess)
        {
            bool tempBakCreated = false;
            if (QFile::exists(fileName))
            {
                if (!QFile::rename(fileName, fileName % ".tmp"))
                {
                    QFile::remove(StandardPaths::tempLocation() % "/" % QFileInfo(fileName).fileName() % ".tmp");
                    return false;
                }
                tempBakCreated = true;
            }
            if (!QFile::rename(StandardPaths::tempLocation() % "/" % QFileInfo(fileName).fileName() % ".tmp", fileName))
            {
                QFile::remove(StandardPaths::tempLocation() % "/" % QFileInfo(fileName).fileName() % ".tmp");
                if (tempBakCreated) { QFile::rename(fileName % ".tmp", fileName); }
                return false;
            }
            if (tempBakCreated) { QFile::remove(fileName % ".tmp"); }
        }
#endif
        return saveSuccess;
    }
    else
    {
        delete picFile;
        return saveSuccess;
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

bool SnapmaticPicture::isVisible()
{
    if (picFilePath.right(7) == QLatin1String(".hidden"))
    {
        return false;
    }
    return true;
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

// SNAPMATIC DEFAULTS

bool SnapmaticPicture::isSnapmaticDefaultsEnforced()
{
    return careSnapDefault;
}

void SnapmaticPicture::setSnapmaticDefaultsEnforced(bool enforced)
{
    careSnapDefault = enforced;
}

// SNAPMATIC FORMAT

SnapmaticFormat SnapmaticPicture::getSnapmaticFormat()
{
    if (isCustomFormat)
    {
        return SnapmaticFormat::G5E_Format;
    }
    return SnapmaticFormat::PGTA_Format;
}

void SnapmaticPicture::setSnapmaticFormat(SnapmaticFormat format)
{
    if (format == SnapmaticFormat::G5E_Format)
    {
        isCustomFormat = true;
        return;
    }
    else if (format == SnapmaticFormat::PGTA_Format)
    {
        isCustomFormat = false;
        return;
    }
    qDebug() << "setSnapmaticFormat: Invalid SnapmaticFormat defined, valid SnapmaticFormats are G5E_Format and PGTA_Format";
}

bool SnapmaticPicture::isFormatSwitched()
{
    return isFormatSwitch;
}

// VERIFY CONTENT

bool SnapmaticPicture::verifyTitle(const QString &title)
{
    // VERIFY TITLE FOR BE A VALID SNAPMATIC TITLE
    if (title.length() <= titlStreamCharacterMax)
    {
        for (QChar titleChar : title)
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
