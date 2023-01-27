/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2023 Syping
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

#if QT_VERSION < 0x060000
#include <QTextCodec>
#else
#include <QStringDecoder>
#endif
#include <QSaveFile>

// IMAGES VALUES
#define snapmaticResolutionW 960
#define snapmaticResolutionH 536
#define snapmaticResolution QSize(snapmaticResolutionW, snapmaticResolutionH)

// GTA5VIEW RELATED INTERNAL FUNCTIONS
inline quint32 gta5view_charToUInt32LE(char *x)
{
    return (static_cast<unsigned char>(x[3]) << 24 |
            static_cast<unsigned char>(x[2]) << 16 |
            static_cast<unsigned char>(x[1]) << 8 |
            static_cast<unsigned char>(x[0]));
}

inline void gta5view_uInt32ToCharLE(quint32 x, char *y)
{
    y[0] = x;
    y[1] = x >> 8;
    y[2] = x >> 16;
    y[3] = x >> 24;
}

inline bool gta5view_export_load(const QByteArray &fileData, RagePhoto *ragePhoto)
{
    QBuffer dataBuffer;
    dataBuffer.setData(fileData);
    if (!dataBuffer.open(QIODevice::ReadOnly))
        return false;
    dataBuffer.seek(4);

    char uInt32Buffer[4];
    qint64 size = dataBuffer.read(uInt32Buffer, 4);
    if (size != 4)
        return false;

    quint32 format = gta5view_charToUInt32LE(uInt32Buffer);
    if (format == G5EExportFormat::G5E3P) {
        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        quint32 compressedSize = gta5view_charToUInt32LE(uInt32Buffer);

        char *compressedPhotoHeader = static_cast<char*>(std::malloc(compressedSize));
        if (!compressedPhotoHeader)
            return false;
        size = dataBuffer.read(compressedPhotoHeader, compressedSize);
        if (size != compressedSize) {
            free(compressedPhotoHeader);
            return false;
        }
        QByteArray t_photoHeader = QByteArray::fromRawData(compressedPhotoHeader, compressedSize);
        t_photoHeader = qUncompress(t_photoHeader);
        free(compressedPhotoHeader);
        if (t_photoHeader.isEmpty())
            return false;

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        quint32 t_headerSum = gta5view_charToUInt32LE(uInt32Buffer);
        ragePhoto->setHeader(t_photoHeader.constData(), t_headerSum);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        quint32 t_photoBuffer = gta5view_charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        compressedSize = gta5view_charToUInt32LE(uInt32Buffer);

        char *compressedPhoto = static_cast<char*>(std::malloc(compressedSize));
        if (!compressedPhoto)
            return false;
        size = dataBuffer.read(compressedPhoto, compressedSize);
        if (size != compressedSize) {
            free(compressedPhoto);
            return false;
        }
        QByteArray t_photoData = QByteArray::fromRawData(compressedPhoto, compressedSize);
        t_photoData = qUncompress(t_photoData);
        free(compressedPhoto);
        ragePhoto->setPhoto(t_photoData.constData(), t_photoData.size(), t_photoBuffer);

        // JSON offset will be calculated later, offsets will be removed in G5E4P
        size = dataBuffer.skip(4);
        if (size != 4)
            return false;

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        quint32 t_jsonBuffer = gta5view_charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        compressedSize = gta5view_charToUInt32LE(uInt32Buffer);

        char *compressedJson = static_cast<char*>(std::malloc(compressedSize));
        if (!compressedJson)
            return false;
        size = dataBuffer.read(compressedJson, compressedSize);
        if (size != compressedSize) {
            free(compressedJson);
            return false;
        }
        QByteArray t_jsonData = QByteArray::fromRawData(compressedJson, compressedSize);
        t_jsonData = qUncompress(t_jsonData);
        free(compressedJson);
        if (t_jsonData.isEmpty())
            return false;
        ragePhoto->setJson(t_jsonData.constData(), t_jsonBuffer);

        // TITL offset will be calculated later, offsets will be removed in G5E4P
        size = dataBuffer.skip(4);
        if (size != 4)
            return false;

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        quint32 t_titlBuffer = gta5view_charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        compressedSize = gta5view_charToUInt32LE(uInt32Buffer);

        char *compressedTitl = static_cast<char*>(std::malloc(compressedSize));
        if (!compressedTitl)
            return false;
        size = dataBuffer.read(compressedTitl, compressedSize);
        if (size != compressedSize) {
            free(compressedTitl);
            return false;
        }
        QByteArray t_titlData = QByteArray::fromRawData(compressedTitl, compressedSize);
        t_titlData = qUncompress(t_titlData);
        free(compressedTitl);
        ragePhoto->setTitle(t_titlData.constData(), t_titlBuffer);

        // DESC offset will be calculated later, offsets will be removed in G5E4P
        size = dataBuffer.skip(4);
        if (size != 4)
            return false;

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        quint32 t_descBuffer = gta5view_charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        compressedSize = gta5view_charToUInt32LE(uInt32Buffer);

        char *compressedDesc = static_cast<char*>(std::malloc(compressedSize));
        if (!compressedDesc)
            return false;
        size = dataBuffer.read(compressedDesc, compressedSize);
        if (size != compressedSize) {
            free(compressedDesc);
            return false;
        }
        QByteArray t_descData = QByteArray::fromRawData(compressedDesc, compressedSize);
        t_descData  = qUncompress(t_descData);
        free(compressedDesc);
        ragePhoto->setDescription(t_descData.constData(), t_descBuffer);

        // EOF will be calculated later, EOF marker will be removed in G5E4P
        size = dataBuffer.skip(4);
        if (size != 4)
            return false;

        // libragephoto needs to know we gave it a GTA V Snapmatic
        ragePhoto->setFormat(RagePhoto::GTA5);

        return true;
    }
    else if (format == G5EExportFormat::G5E2P) {
        const QByteArray t_fileData = qUncompress(dataBuffer.readAll());
        if (t_fileData.isEmpty())
            return false;
        return ragePhoto->load(t_fileData.constData(), t_fileData.size());
    }
    else if (format == G5EExportFormat::G5E1P) {
        size = dataBuffer.skip(1);
        if (size != 1)
            return false;

        char length[1];
        size = dataBuffer.read(length, 1);
        if (size != 1)
            return false;
        int i_length = QByteArray::number(static_cast<int>(length[0]), 16).toInt() + 6;

        size = dataBuffer.skip(i_length);
        if (size != i_length)
            return false;

        const QByteArray t_fileData = qUncompress(dataBuffer.readAll());
        if (t_fileData.isEmpty())
            return false;
        return ragePhoto->load(t_fileData.constData(), t_fileData.size());
    }
    return false;
}

inline void gta5view_export_save(QIODevice *ioDevice, RagePhotoData *data)
{
    char uInt32Buffer[4];
    quint32 format = G5EPhotoFormat::G5EX;
    gta5view_uInt32ToCharLE(format, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);
    format = G5EExportFormat::G5E3P;
    gta5view_uInt32ToCharLE(format, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);

    QByteArray compressedData = qCompress(QByteArray::fromRawData(data->header, strlen(data->header)), 9);
    quint32 compressedSize = compressedData.size();
    gta5view_uInt32ToCharLE(compressedSize, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);
    ioDevice->write(compressedData);

    gta5view_uInt32ToCharLE(data->headerSum, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);

    gta5view_uInt32ToCharLE(data->photoBuffer, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);

    compressedData = qCompress(QByteArray::fromRawData(data->jpeg, data->jpegSize), 9);
    compressedSize = compressedData.size();
    gta5view_uInt32ToCharLE(compressedSize, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);
    ioDevice->write(compressedData);

    gta5view_uInt32ToCharLE(data->jsonOffset, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);

    gta5view_uInt32ToCharLE(data->jsonBuffer, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);

    compressedData = qCompress(QByteArray::fromRawData(data->json, strlen(data->json)), 9);
    compressedSize = compressedData.size();
    gta5view_uInt32ToCharLE(compressedSize, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);
    ioDevice->write(compressedData);

    gta5view_uInt32ToCharLE(data->titlOffset, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);

    gta5view_uInt32ToCharLE(data->titlBuffer, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);

    compressedData = qCompress(QByteArray::fromRawData(data->title, strlen(data->title)), 9);
    compressedSize = compressedData.size();
    gta5view_uInt32ToCharLE(compressedSize, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);
    ioDevice->write(compressedData);

    gta5view_uInt32ToCharLE(data->descOffset, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);

    gta5view_uInt32ToCharLE(data->descBuffer, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);

    compressedData = qCompress(QByteArray::fromRawData(data->description, strlen(data->description)), 9);
    compressedSize = compressedData.size();
    gta5view_uInt32ToCharLE(compressedSize, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);
    ioDevice->write(compressedData);

    gta5view_uInt32ToCharLE(data->endOfFile, uInt32Buffer);
    ioDevice->write(uInt32Buffer, 4);
}

// SNAPMATIC PICTURE CLASS
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
    p_ragePhoto.clear();
    cachePicture = QImage();
    picExportFileName = QString();
    pictureStr = QString();
    lastStep = QString();
    sortStr = QString();

    // INIT PIC FORMAT
    picFormat = 0;

    // INIT PIC BOOLS
    isFormatSwitch = false;
    isPreLoaded = false;
    picOk = false;

    // INIT JSON
    jsonOk = false;

    // SNAPMATIC PROPERTIES
    localProperties = {};

    // JSON OBJECT
    jsonObject = QJsonObject();
}

bool SnapmaticPicture::preloadFile()
{
    QFile picFile(picFilePath);
    picFileName = QFileInfo(picFilePath).fileName();
    isFormatSwitch = false;

    if (!picFile.open(QIODevice::ReadOnly)) {
        lastStep = "1;/1,OpenFile," % convertDrawStringForLog(picFilePath);
        return false;
    }

    const qint64 fileMaxSize = (1024 * 1024 * 64);
    const QByteArray fileData = picFile.read(fileMaxSize);

    bool ok = p_ragePhoto.load(fileData.constData(), fileData.size());
    picFormat = p_ragePhoto.format();

    // libragephoto doesn't support modules yet
    if (picFormat == G5EPhotoFormat::G5EX)
        ok = gta5view_export_load(fileData, &p_ragePhoto);

    if (!ok) {
        const RagePhoto::Error error = static_cast<RagePhoto::Error>(p_ragePhoto.error());
        switch (error) {
        case RagePhoto::Uninitialised:
            lastStep = "1;/1,OpenFile," % convertDrawStringForLog(picFilePath);
            break;
        case RagePhoto::NoFormatIdentifier:
        case RagePhoto::IncompleteHeader:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",1,NOHEADER";
            break;
        case RagePhoto::IncompatibleFormat:
            lastStep = "2;/4,ReadingFile," % convertDrawStringForLog(picFilePath) % ",1,MALFORMEDHEADER,LIBRAGEPHOTO_INCOMPATIBLE_FORMAT";
            break;
        case RagePhoto::UnicodeInitError:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",1,LIBRAGEPHOTO_UNICODE_ERROR";
            break;
        case RagePhoto::UnicodeHeaderError:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",1,MALFORMEDHEADER";
            break;
        case RagePhoto::HeaderMallocError:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",1,LIBRAGEPHOTO_MALLOC_ERROR";
            break;
        case RagePhoto::IncompleteChecksum:
        case RagePhoto::IncompleteEOF:
        case RagePhoto::IncompleteJsonOffset:
        case RagePhoto::IncompleteTitleOffset:
        case RagePhoto::IncompleteDescOffset:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",2,NOHEADER";
            break;
        case RagePhoto::IncompleteJpegMarker:
        case RagePhoto::IncorrectJpegMarker:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",2,NOJPEG";
            break;
        case RagePhoto::IncompletePhotoBuffer:
        case RagePhoto::IncompletePhotoSize:
        case RagePhoto::PhotoReadError:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",2,NOPIC";
            break;
        case RagePhoto::PhotoMallocError:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",2,LIBRAGEPHOTO_MALLOC_ERROR";
            break;
        case RagePhoto::IncompleteJsonMarker:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,NOJSON";
            break;
        case RagePhoto::IncorrectJsonMarker:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,CTJSON";
            break;
        case RagePhoto::IncompleteJsonBuffer:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,LIBRAGEPHOTO_INCOMPLETE_BUFFER_ERROR";
            break;
        case RagePhoto::JsonReadError:
            lastStep = "2;/4,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,NOJSON,LIBRAGEPHOTO_READ_ERROR";
            break;
        case RagePhoto::JsonMallocError:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,LIBRAGEPHOTO_MALLOC_ERROR";
            break;
        case RagePhoto::IncompleteTitleMarker:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",4,NOTITL";
            break;
        case RagePhoto::IncorrectTitleMarker:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",4,CTTITL";
            break;
        case RagePhoto::IncompleteTitleBuffer:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",4,LIBRAGEPHOTO_INCOMPLETE_BUFFER_ERROR";
            break;
        case RagePhoto::TitleReadError:
            lastStep = "2;/4,ReadingFile," % convertDrawStringForLog(picFilePath) % ",4,NOTITL,LIBRAGEPHOTO_READ_ERROR";
            break;
        case RagePhoto::TitleMallocError:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",4,LIBRAGEPHOTO_MALLOC_ERROR";
            break;
        case RagePhoto::IncompleteDescMarker:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",5,NODESC";
            break;
        case RagePhoto::IncorrectDescMarker:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",5,CTDESC";
            break;
        case RagePhoto::IncompleteDescBuffer:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",5,LIBRAGEPHOTO_INCOMPLETE_BUFFER_ERROR";
            break;
        case RagePhoto::DescReadError:
            lastStep = "2;/4,ReadingFile," % convertDrawStringForLog(picFilePath) % ",5,NODESC,LIBRAGEPHOTO_READ_ERROR";
            break;
        case RagePhoto::DescMallocError:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",5,LIBRAGEPHOTO_MALLOC_ERROR";
            break;
        case RagePhoto::IncompleteJendMarker:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",6,LIBRAGEPHOTO_INCOMPLETE_JEND_ERROR";
            break;
        case RagePhoto::IncorrectJendMarker:
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",6,LIBRAGEPHOTO_INCORRECT_JEND_ERROR";
            break;
        default:
            break;
        }
        return false;
    }

    const QJsonDocument t_jsonDocument = QJsonDocument::fromJson(p_ragePhoto.json());
    if (t_jsonDocument.isNull())
        return false;
    jsonObject = t_jsonDocument.object();

    if (!picFilePath.endsWith(".g5e", Qt::CaseInsensitive)) {
        if (picFormat == G5EPhotoFormat::G5EX)
            isFormatSwitch = true;
    }
    isPreLoaded = true;
    emit preloaded();
    return ok;
}

bool SnapmaticPicture::readingPicture(bool cacheEnabled_)
{
    // Start opening file
    // lastStep is like currentStep

    // Set boolean values
    cacheEnabled = cacheEnabled_;

    bool ok = true;
    if (!isPreLoaded)
        ok = preloadFile();

    if (!ok)
        return false;

    if (cacheEnabled)
        picOk = cachePicture.loadFromData(QByteArray::fromRawData(p_ragePhoto.photoData(), p_ragePhoto.photoSize()), "JPEG");
    if (!cacheEnabled) {
        QImage tempPicture;
        picOk = tempPicture.loadFromData(QByteArray::fromRawData(p_ragePhoto.photoData(), p_ragePhoto.photoSize()), "JPEG");
    }

    parseJsonContent(); // JSON parsing is own function
    updateStrings();

    emit loaded();
    return picOk;
}

void SnapmaticPicture::updateStrings()
{
    QString cmpPicTitl(p_ragePhoto.title());
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
    const QString exportStr = localProperties.createdDateTime.toString("yyyyMMdd") % "-" % QString::number(localProperties.uid);
    if (getSnapmaticFormat() == SnapmaticFormat::G5E_Format)
        picFileName = "PGTA5" % QString::number(localProperties.uid);
    picExportFileName = exportStr % "_" % cmpPicTitl;
}

bool SnapmaticPicture::readingPictureFromFile(const QString &fileName, bool cacheEnabled_)
{
    if (!fileName.isEmpty()) {
        picFilePath = fileName;
        return readingPicture(cacheEnabled_);
    }
    else
        return false;
}

bool SnapmaticPicture::setImage(const QImage &picture, bool eXtendMode)
{
#ifdef GTA5SYNC_DYNAMIC_PHOTOBUFFER // It's not properly implemented yet, please don't define
    quint32 jpegPicStreamLength = p_ragePhoto.data()->photoBuffer();
#else
    quint32 jpegPicStreamLength = RagePhoto::DEFAULT_GTA5_PHOTOBUFFER;
#endif
    QByteArray picByteArray;
    int comLvl = 100;
    bool saveSuccess = false;
    while (comLvl != 0 && !saveSuccess) {
        QByteArray picByteArrayT;
        QBuffer picStreamT(&picByteArrayT);
        picStreamT.open(QIODevice::WriteOnly);
        saveSuccess = picture.save(&picStreamT, "JPEG", comLvl);
        picStreamT.close();
        if (saveSuccess) {
            quint32 size = picByteArrayT.length();
            if (size > jpegPicStreamLength) {
                if (!eXtendMode) {
                    comLvl--;
                    saveSuccess = false;
                }
                else
                    picByteArray = picByteArrayT;
            }
            else
                picByteArray = picByteArrayT;
        }
    }
    if (saveSuccess)
        return setPictureStream(picByteArray);
    return false;
}

bool SnapmaticPicture::setPictureStream(const QByteArray &streamArray) // clean method
{
#ifdef GTA5SYNC_DYNAMIC_PHOTOBUFFER // It's not properly implemented yet, please don't define
    quint32 jpegPicStreamLength = p_ragePhoto.data()->photoBuffer();
#else
    quint32 jpegPicStreamLength = RagePhoto::DEFAULT_GTA5_PHOTOBUFFER;
#endif
    if (streamArray.size() > jpegPicStreamLength)
        jpegPicStreamLength = streamArray.size();
#ifdef GTA5SYNC_COMPACT_PHOTOBUFFER // Experiment to save less than the default photo buffer
    if (streamArray.size() < jpegPicStreamLength)
        jpegPicStreamLength = streamArray.size();
#endif
    bool success = p_ragePhoto.setPhoto(streamArray.data(), streamArray.size(), jpegPicStreamLength);
    if (success) {
        if (cacheEnabled) {
            QImage replacedPicture;
            replacedPicture.loadFromData(streamArray);
            cachePicture = replacedPicture;
        }
        return true;
    }
    else
        return false;
}

bool SnapmaticPicture::setPictureTitl(const QString &newTitle_)
{
    QString newTitle = newTitle_;
    if (newTitle.length() > 39)
        newTitle = newTitle.left(39);
    p_ragePhoto.setTitle(newTitle.toStdString().c_str());
    return true;
}

QString SnapmaticPicture::getExportPictureFileName()
{
    return picExportFileName;
}

QString SnapmaticPicture::getOriginalPictureFileName()
{
    QString newPicFileName = picFileName;
    if (picFileName.right(4) == ".bak")
        newPicFileName = QString(picFileName).remove(picFileName.length() - 4, 4);
    if (picFileName.right(7) == ".hidden")
        newPicFileName = QString(picFileName).remove(picFileName.length() - 7, 7);
    return newPicFileName;
}

QString SnapmaticPicture::getOriginalPictureFilePath()
{
    QString newPicFilePath = picFilePath;
    if (picFilePath.right(4) == ".bak")
        newPicFilePath = QString(picFilePath).remove(picFilePath.length() - 4, 4);
    if (picFilePath.right(7) == ".hidden")
        newPicFilePath = QString(picFilePath).remove(picFilePath.length() - 7, 7);
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

QString SnapmaticPicture::getPictureTitl()
{
    return p_ragePhoto.title();
}

QString SnapmaticPicture::getPictureStr()
{
    return pictureStr;
}

QString SnapmaticPicture::getLastStep(bool readable)
{
    if (readable) {
        QStringList lastStepList = lastStep.split(";/");
        if (lastStepList.length() < 2)
            return lastStep;
        bool intOk;
        QStringList descStepList = lastStepList.at(1).split(",");
        if (descStepList.length() < 1)
            return lastStep;
        int argsCount = descStepList.at(0).toInt(&intOk);
        if (!intOk)
            return lastStep;
        if (argsCount == 1) {
            QString currentAction = descStepList.at(1);
            QString actionFile = descStepList.at(2);
            if (currentAction == "OpenFile")
                return tr("open file %1").arg(actionFile);
        }
        else if (argsCount == 3 || argsCount == 4) {
            QString currentAction = descStepList.at(1);
            QString actionFile = descStepList.at(2);
            QString actionError = descStepList.at(4);
            QString actionError2;
            if (argsCount == 4) { actionError2 = descStepList.at(5); }
            if (currentAction == "ReadingFile") {
                QString readableError = actionError;
                if (actionError == "NOHEADER")
                    readableError = tr("header not exists");
                else if (actionError == "MALFORMEDHEADER")
                    readableError = tr("header is malformed");
                else if (actionError == "NOJPEG" || actionError == "NOPIC")
                    readableError = tr("picture not exists (%1)").arg(actionError);
                else if (actionError == "NOJSON" || actionError == "CTJSON")
                    readableError = tr("JSON not exists (%1)").arg(actionError);
                else if (actionError == "NOTITL" || actionError == "CTTITL")
                    readableError = tr("title not exists (%1)").arg(actionError);
                else if (actionError == "NODESC" || actionError == "CTDESC")
                    readableError = tr("description not exists (%1)").arg(actionError);
                else if (actionError == "JSONINCOMPLETE" && actionError2 == "JSONERROR")
                    readableError = tr("JSON is incomplete and malformed");
                else if (actionError == "JSONINCOMPLETE")
                    readableError = tr("JSON is incomplete");
                else if (actionError == "JSONERROR")
                    readableError = tr("JSON is malformed");
                return tr("reading file %1 because of %2", "Example for %2: JSON is malformed error").arg(actionFile, readableError);
            }
            else
                return lastStep;
        }
        else
            return lastStep;
    }
    return lastStep;

}

QImage SnapmaticPicture::getImage()
{
    if (cacheEnabled)
        return cachePicture;
    else
        return QImage::fromData(QByteArray::fromRawData(p_ragePhoto.photoData(), p_ragePhoto.photoSize()), "JPEG");
    return QImage();
}

QByteArray SnapmaticPicture::getPictureStream()
{
    return QByteArray::fromRawData(p_ragePhoto.photoData(), p_ragePhoto.photoSize());
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

void SnapmaticPicture::emitCustomSignal(const QString &signal)
{
    emit customSignal(signal);
}

// JSON part

bool SnapmaticPicture::isJsonOk()
{
    return jsonOk;
}

QString SnapmaticPicture::getJsonStr()
{
    return QString::fromUtf8(p_ragePhoto.json());
}

SnapmaticProperties SnapmaticPicture::getSnapmaticProperties()
{
    return localProperties;
}

void SnapmaticPicture::parseJsonContent()
{
    QVariantMap jsonMap = jsonObject.toVariantMap();

    bool jsonIncomplete = false;
    bool jsonError = false;
    if (jsonObject.contains("loc")) {
        if (jsonObject["loc"].isObject()) {
            QJsonObject locObject = jsonObject["loc"].toObject();
            if (locObject.contains("x")) {
                if (locObject["x"].isDouble()) { localProperties.location.x = locObject["x"].toDouble(); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
            if (locObject.contains("y")) {
                if (locObject["y"].isDouble()) { localProperties.location.y = locObject["y"].toDouble(); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
            if (locObject.contains("z")) {
                if (locObject["z"].isDouble()) { localProperties.location.z = locObject["z"].toDouble(); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
        }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("uid")) {
        bool uidOk;
        localProperties.uid = jsonMap["uid"].toInt(&uidOk);
        if (!uidOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("area")) {
        if (jsonObject["area"].isString()) { localProperties.location.area = jsonObject["area"].toString(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("crewid")) {
        bool crewIDOk;
        localProperties.crewID = jsonMap["crewid"].toInt(&crewIDOk);
        if (!crewIDOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("street")) {
        bool streetIDOk;
        localProperties.streetID = jsonMap["street"].toInt(&streetIDOk);
        if (!streetIDOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("creat")) {
        bool timestampOk;
        QDateTime createdTimestamp;
        localProperties.createdTimestamp = jsonMap["creat"].toUInt(&timestampOk);
#if QT_VERSION >= 0x060000
        createdTimestamp.setSecsSinceEpoch(localProperties.createdTimestamp);
#else
        createdTimestamp.setTime_t(localProperties.createdTimestamp);
#endif
        localProperties.createdDateTime = createdTimestamp;
        if (!timestampOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("plyrs")) {
        if (jsonObject["plyrs"].isArray()) { localProperties.playersList = jsonMap["plyrs"].toStringList(); }
        else { jsonError = true; }
    }
    // else { jsonIncomplete = true; } // 2016 Snapmatic pictures left out plyrs when none are captured, so don't force exists on that one
    if (jsonObject.contains("meme")) {
        if (jsonObject["meme"].isBool()) { localProperties.isMeme = jsonObject["meme"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("mug")) {
        if (jsonObject["mug"].isBool()) { localProperties.isMug = jsonObject["mug"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("slf")) {
        if (jsonObject["slf"].isBool()) { localProperties.isSelfie = jsonObject["slf"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("drctr")) {
        if (jsonObject["drctr"].isBool()) { localProperties.isFromDirector = jsonObject["drctr"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("rsedtr")) {
        if (jsonObject["rsedtr"].isBool()) { localProperties.isFromRSEditor = jsonObject["rsedtr"].toBool(); }
        else { jsonError = true; }
    }
    else { localProperties.isFromRSEditor = false; }
    if (jsonObject.contains("onislandx")) {
        if (jsonObject["onislandx"].isBool()) { localProperties.location.isCayoPerico = jsonObject["onislandx"].toBool(); }
        else { jsonError = true; }
    }
    else { localProperties.location.isCayoPerico = false; }

    if (!jsonIncomplete && !jsonError) {
        jsonOk = true;
    }
    else {
        if (jsonIncomplete && jsonError) {
            lastStep = "2;/4,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,JSONINCOMPLETE,JSONERROR";
        }
        else if (jsonIncomplete) {
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,JSONINCOMPLETE";
        }
        else if (jsonError) {
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,JSONERROR";
        }
        jsonOk = false;
    }
}

bool SnapmaticPicture::setSnapmaticProperties(SnapmaticProperties properties)
{
    QJsonObject t_jsonObject = jsonObject;

    QJsonObject locObject;
    locObject["x"] = properties.location.x;
    locObject["y"] = properties.location.y;
    locObject["z"] = properties.location.z;

    t_jsonObject["loc"] = locObject;
    t_jsonObject["uid"] = properties.uid;
    t_jsonObject["area"] = properties.location.area;
    t_jsonObject["crewid"] = properties.crewID;
    t_jsonObject["street"] = properties.streetID;
    t_jsonObject["creat"] = QJsonValue::fromVariant(properties.createdTimestamp);
    t_jsonObject["plyrs"] = QJsonValue::fromVariant(properties.playersList);
    t_jsonObject["meme"] = properties.isMeme;
    t_jsonObject["mug"] = properties.isMug;
    t_jsonObject["slf"] = properties.isSelfie;
    t_jsonObject["drctr"] = properties.isFromDirector;
    t_jsonObject["rsedtr"] = properties.isFromRSEditor;
    t_jsonObject["onislandx"] = properties.location.isCayoPerico;

    const QJsonDocument jsonDocument(t_jsonObject);
    if (setJsonStr(QString::fromUtf8(jsonDocument.toJson(QJsonDocument::Compact)))) {
        localProperties = properties;
        return true;
    }
    return false;
}

bool SnapmaticPicture::setJsonStr(const QString &newJsonStr, bool updateProperties)
{
    const QJsonDocument t_jsonDocument = QJsonDocument::fromJson(newJsonStr.toStdString().c_str());
    if (t_jsonDocument.isNull())
        return false;
    const QByteArray t_jsonData = t_jsonDocument.toJson(QJsonDocument::Compact);
    jsonObject = t_jsonDocument.object();

    p_ragePhoto.setJson(t_jsonData.constData());
    if (updateProperties)
        parseJsonContent();
    return true;
}

// FILE MANAGEMENT

bool SnapmaticPicture::exportPicture(const QString &fileName, SnapmaticFormat format_)
{
    // Keep current format when Auto_Format is used
    SnapmaticFormat format = format_;
    if (format_ == SnapmaticFormat::Auto_Format) {
        if (p_ragePhoto.format() == G5EPhotoFormat::G5EX) {
            format = SnapmaticFormat::G5E_Format;
        }
        else {
            format = SnapmaticFormat::PGTA_Format;
        }
    }

    bool saveSuccess = false;
    QSaveFile picFile(fileName);
    if (picFile.open(QIODevice::WriteOnly)) {
        if (format == SnapmaticFormat::G5E_Format) {
            gta5view_export_save(&picFile, p_ragePhoto.data());
            saveSuccess = picFile.commit();
        }
        else if (format == SnapmaticFormat::JPEG_Format) {
            picFile.write(QByteArray::fromRawData(p_ragePhoto.photoData(), p_ragePhoto.photoSize()));
            saveSuccess = picFile.commit();
        }
        else {
            bool ok;
            const std::string photo = p_ragePhoto.save(&ok);
            if (ok)
                picFile.write(photo.data(), photo.size());
            saveSuccess = picFile.commit();
        }
        return saveSuccess;
    }
    else
        return saveSuccess;
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
    bool success = false;
    if (!QFile::exists(picFilePath))
        success = true;
    else if (QFile::remove(picFilePath))
        success = true;
    if (isHidden()) {
        const QString picBakPath = QString(picFilePath).remove(picFilePath.length() - 7, 7) % ".bak";
        if (QFile::exists(picBakPath))
            QFile::remove(picBakPath);
    }
    else {
        const QString picBakPath = picFilePath % ".bak";
        if (QFile::exists(picBakPath))
            QFile::remove(picBakPath);
    }
    return success;
}

// VISIBILITY

bool SnapmaticPicture::isHidden()
{
    if (picFilePath.right(7) == QLatin1String(".hidden"))
        return true;
    return false;
}

bool SnapmaticPicture::isVisible()
{
    if (picFilePath.right(7) == QLatin1String(".hidden"))
        return false;
    return true;
}

bool SnapmaticPicture::setPictureHidden()
{
    if (picFormat == G5EPhotoFormat::G5EX) {
        return false;
    }
    if (!isHidden()) {
        QString newPicFilePath = QString(picFilePath % ".hidden");
        if (QFile::rename(picFilePath, newPicFilePath)) {
            picFilePath = newPicFilePath;
            return true;
        }
        return false;
    }
    return true;
}

bool SnapmaticPicture::setPictureVisible()
{
    if (picFormat == G5EPhotoFormat::G5EX)
        return false;
    if (isHidden()) {
        QString newPicFilePath = QString(picFilePath).remove(picFilePath.length() - 7, 7);
        if (QFile::rename(picFilePath, newPicFilePath)) {
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

// SNAPMATIC FORMAT

SnapmaticFormat SnapmaticPicture::getSnapmaticFormat()
{
    if (picFormat == G5EPhotoFormat::G5EX)
        return SnapmaticFormat::G5E_Format;
    return SnapmaticFormat::PGTA_Format;
}

void SnapmaticPicture::setSnapmaticFormat(SnapmaticFormat format)
{
    if (format == SnapmaticFormat::G5E_Format) {
        picFormat = G5EPhotoFormat::G5EX;
        return;
    }
    else if (format == SnapmaticFormat::PGTA_Format) {
        picFormat = RagePhoto::PhotoFormat::GTA5;
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
    if (title.length() <= 39 && title.length() > 0) {
        for (const QChar &titleChar : title) {
            if (!verifyTitleChar(titleChar)) return false;
        }
        return true;
    }
    return false;
}

bool SnapmaticPicture::verifyTitleChar(const QChar &titleChar)
{
    // VERIFY CHAR FOR BE A VALID SNAPMATIC CHARACTER
    if (titleChar.isLetterOrNumber() || titleChar.isPrint()) {
        if (titleChar == '<' || titleChar == '>' || titleChar == '\\')
            return false;
        return true;
    }
    return false;
}

// STRING OPERATIONS

QString SnapmaticPicture::parseTitleString(const QByteArray &commitBytes)
{
#if QT_VERSION >= 0x060000
    QStringDecoder strDecoder = QStringDecoder(QStringDecoder::Utf16LE);
    QString retStr = strDecoder(commitBytes);
    retStr = retStr.trimmed();
#else
    QString retStr = QTextCodec::codecForName("UTF-16LE")->toUnicode(commitBytes).trimmed();
#endif
    retStr.remove(QChar('\x00'));
    return retStr;
}

QString SnapmaticPicture::convertDrawStringForLog(const QString &inputStr)
{
    QString outputStr = inputStr;
    return outputStr.replace("&","&u;").replace(",", "&c;");
}

QString SnapmaticPicture::convertLogStringForDraw(const QString &inputStr)
{
    QString outputStr = inputStr;
    return outputStr.replace("&c;",",").replace("&u;", "&");
}

// RAGEPHOTO

RagePhoto* SnapmaticPicture::ragePhoto()
{
    return &p_ragePhoto;
}
