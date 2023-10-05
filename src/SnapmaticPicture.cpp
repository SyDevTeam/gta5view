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
#include "pcg_basic.h"
#include <QStringBuilder>
#include <QStringList>
#include <QVariantMap>
#include <QFileInfo>
#include <QDateTime>
#include <QString>
#include <cstring>
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

bool gta5view_export_load(RagePhotoData *rp_data, const char *data, size_t length)
{
    const QByteArray fileData = QByteArray::fromRawData(data, length);

    QBuffer dataBuffer;
    dataBuffer.setData(fileData);
    if (!dataBuffer.open(QIODevice::ReadOnly))
        return false;
    dataBuffer.seek(4);

    char uInt32Buffer[4];
    size_t size = dataBuffer.read(uInt32Buffer, 4);
    if (size != 4) {
        rp_data->error = RagePhoto::NoFormatIdentifier;
        return false;
    }

    uint32_t format = gta5view_charToUInt32LE(uInt32Buffer);
    if (format == G5EExportFormat::G5E3P) {
        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteHeader;
            return false;
        }
        quint32 compressedSize = gta5view_charToUInt32LE(uInt32Buffer);

        char *compressedPhotoHeader = static_cast<char*>(std::malloc(compressedSize));
        if (!compressedPhotoHeader) {
            rp_data->error = RagePhoto::HeaderMallocError;
            return false;
        }
        size = dataBuffer.read(compressedPhotoHeader, compressedSize);
        if (size != compressedSize) {
            std::free(compressedPhotoHeader);
            rp_data->error = RagePhoto::UnicodeHeaderError;
            return false;
        }
        QByteArray t_photoHeader = QByteArray::fromRawData(compressedPhotoHeader, compressedSize);
        t_photoHeader = qUncompress(t_photoHeader);
        std::free(compressedPhotoHeader);
        if (t_photoHeader.isEmpty()) {
            rp_data->error = RagePhoto::IncompleteHeader;
            return false;
        }

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteHeader;
            return false;
        }
        rp_data->headerSum = gta5view_charToUInt32LE(uInt32Buffer);
        rp_data->header = static_cast<char*>(std::malloc(t_photoHeader.size() + 1));
        if (!rp_data->header) {
            rp_data->error = RagePhoto::HeaderMallocError;
            return false;
        }
        std::memcpy(rp_data->header, t_photoHeader.constData(), t_photoHeader.size() + 1);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompletePhotoBuffer;
            return false;
        }
        rp_data->jpegBuffer = gta5view_charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompletePhotoBuffer;
            return false;
        }
        compressedSize = gta5view_charToUInt32LE(uInt32Buffer);

        char *compressedPhoto = static_cast<char*>(std::malloc(compressedSize));
        if (!compressedPhoto) {
            rp_data->error = RagePhoto::PhotoMallocError;
            return false;
        }
        size = dataBuffer.read(compressedPhoto, compressedSize);
        if (size != compressedSize) {
            std::free(compressedPhoto);
            rp_data->error = RagePhoto::PhotoReadError;
            return false;
        }
        QByteArray t_photoData = QByteArray::fromRawData(compressedPhoto, compressedSize);
        t_photoData = qUncompress(t_photoData);
        std::free(compressedPhoto);
        rp_data->jpeg = static_cast<char*>(std::malloc(t_photoData.size()));
        if (!rp_data->jpeg) {
            rp_data->error = RagePhoto::PhotoMallocError;
            return false;
        }
        std::memcpy(rp_data->jpeg, t_photoData.constData(), t_photoData.size());
        rp_data->jpegSize = t_photoData.size();

        // JSON offset will be calculated later, offsets will be removed in G5E4P
        size = dataBuffer.skip(4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteJsonOffset;
            return false;
        }

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteJsonBuffer;
            return false;
        }
        rp_data->jsonBuffer = gta5view_charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteJsonBuffer;
            return false;
        }
        compressedSize = gta5view_charToUInt32LE(uInt32Buffer);

        char *compressedJson = static_cast<char*>(std::malloc(compressedSize));
        if (!compressedJson) {
            rp_data->error = RagePhoto::JsonMallocError;
            return false;
        }
        size = dataBuffer.read(compressedJson, compressedSize);
        if (size != compressedSize) {
            std::free(compressedJson);
            rp_data->error = RagePhoto::JsonReadError;
            return false;
        }
        QByteArray t_jsonData = QByteArray::fromRawData(compressedJson, compressedSize);
        t_jsonData = qUncompress(t_jsonData);
        std::free(compressedJson);
        if (t_jsonData.isEmpty()) {
            rp_data->error = RagePhoto::JsonReadError;
            return false;
        }
        rp_data->json = static_cast<char*>(std::malloc(t_jsonData.size() + 1));
        if (!rp_data->json) {
            rp_data->error = RagePhoto::JsonMallocError;
            return false;
        }
        std::memcpy(rp_data->json, t_jsonData.constData(), t_jsonData.size() + 1);

        // TITL offset will be calculated later, offsets will be removed in G5E4P
        size = dataBuffer.skip(4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteTitleOffset;
            return false;
        }

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteTitleBuffer;
            return false;
        }
        rp_data->titlBuffer = gta5view_charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteTitleBuffer;
            return false;
        }
        compressedSize = gta5view_charToUInt32LE(uInt32Buffer);

        char *compressedTitl = static_cast<char*>(std::malloc(compressedSize));
        if (!compressedTitl) {
            rp_data->error = RagePhoto::TitleMallocError;
            return false;
        }
        size = dataBuffer.read(compressedTitl, compressedSize);
        if (size != compressedSize) {
            std::free(compressedTitl);
            rp_data->error = RagePhoto::TitleReadError;
            return false;
        }
        QByteArray t_titlData = QByteArray::fromRawData(compressedTitl, compressedSize);
        t_titlData = qUncompress(t_titlData);
        std::free(compressedTitl);
        rp_data->title = static_cast<char*>(std::malloc(t_titlData.size() + 1));
        if (!rp_data->title) {
            rp_data->error = RagePhoto::TitleMallocError;
            return false;
        }
        std::memcpy(rp_data->title, t_titlData.constData(), t_titlData.size() + 1);

        // DESC offset will be calculated later, offsets will be removed in G5E4P
        size = dataBuffer.skip(4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteDescOffset;
            return false;
        }

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteDescBuffer;
            return false;
        }
        rp_data->descBuffer = gta5view_charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteDescBuffer;
            return false;
        }
        compressedSize = gta5view_charToUInt32LE(uInt32Buffer);

        char *compressedDesc = static_cast<char*>(std::malloc(compressedSize));
        if (!compressedDesc) {
            rp_data->error = RagePhoto::DescMallocError;
            return false;
        }
        size = dataBuffer.read(compressedDesc, compressedSize);
        if (size != compressedSize) {
            std::free(compressedDesc);
            rp_data->error = RagePhoto::DescReadError;
            return false;
        }
        QByteArray t_descData = QByteArray::fromRawData(compressedDesc, compressedSize);
        t_descData = qUncompress(t_descData);
        std::free(compressedDesc);
        rp_data->description = static_cast<char*>(std::malloc(t_descData.size() + 1));
        if (!rp_data->description) {
            rp_data->error = RagePhoto::DescMallocError;
            return false;
        }
        std::memcpy(rp_data->description, t_descData.constData(), t_descData.size() + 1);

        // EOF will be calculated later, EOF marker will be removed in G5E4P
        size = dataBuffer.skip(4);
        if (size != 4) {
            rp_data->error = RagePhoto::IncompleteEOF;
            return false;
        }

        rp_data->error = RagePhoto::NoError;
        RagePhoto::setBufferOffsets(rp_data);

        return true;
    }
    else if (format == G5EExportFormat::G5E2P) {
        const QByteArray t_fileData = qUncompress(dataBuffer.readAll());
        if (t_fileData.isEmpty()) {
            rp_data->error = RagePhoto::IncompatibleFormat;
            return false;
        }
        const bool ok = RagePhoto::load(t_fileData.constData(), t_fileData.size(), rp_data, nullptr);
        rp_data->photoFormat = G5EPhotoFormat::G5EX;
        return ok;
    }
    else if (format == G5EExportFormat::G5E1P) {
        size = dataBuffer.skip(1);
        if (size != 1) {
            rp_data->error = RagePhoto::IncompatibleFormat;
            return false;
        }

        char length[1];
        size = dataBuffer.read(length, 1);
        if (size != 1) {
            rp_data->error = RagePhoto::IncompatibleFormat;
            return false;
        }
        int i_length = QByteArray::number(static_cast<int>(length[0]), 16).toInt() + 6;

        size = dataBuffer.skip(i_length);
        if (size != i_length) {
            rp_data->error = RagePhoto::IncompatibleFormat;
            return false;
        }

        const QByteArray t_fileData = qUncompress(dataBuffer.readAll());
        if (t_fileData.isEmpty()) {
            rp_data->error = RagePhoto::IncompatibleFormat;
            return false;
        }

        const bool ok = RagePhoto::load(t_fileData.constData(), t_fileData.size(), rp_data, nullptr);
        rp_data->photoFormat = G5EPhotoFormat::G5EX;
        return ok;
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

    gta5view_uInt32ToCharLE(data->jpegBuffer, uInt32Buffer);
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

inline bool gta5view_isGTAVFormat(uint32_t photoFormat)
{
    return (photoFormat == G5EPhotoFormat::G5EX || photoFormat == RagePhoto::PhotoFormat::GTA5);
}

inline bool gta5view_isRDR2Format(uint32_t photoFormat)
{
    return (photoFormat == RagePhoto::PhotoFormat::RDR2);
}

// SNAPMATIC PICTURE CLASS
SnapmaticPicture::SnapmaticPicture(const QString &fileName, QObject *parent) : QObject(parent), picFilePath(fileName)
{
    RagePhotoFormatParser g5eParser[1]{};
    g5eParser[0].photoFormat = G5EPhotoFormat::G5EX;
    g5eParser[0].funcLoad = &gta5view_export_load;
    p_ragePhoto.addParser(&g5eParser[0]);

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

    // INIT PIC BOOLS
    isFormatSwitch = false;
    isPreLoaded = false;
    picOk = false;

    // INIT JSON
    jsonOk = false;

    // SNAPMATIC PROPERTIES
    localProperties = {};

    // JSON VALUE
    snapmaticJson.jsonObject = boost::json::object();
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

    const qint64 fileSize = picFile.size();
    const QByteArray fileData = picFile.read(fileSize);

    bool ok = p_ragePhoto.load(fileData.constData(), fileData.size());

    if (!ok) {
        const int32_t error = p_ragePhoto.error();
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

    if (!picFilePath.endsWith(".g5e", Qt::CaseInsensitive) && p_ragePhoto.format() == G5EPhotoFormat::G5EX)
        isFormatSwitch = true;

    boost::json::error_code ec;
    const boost::json::value jsonValue = boost::json::parse(p_ragePhoto.json(), ec);
    if (ec)
        return false;
    if (!jsonValue.is_object())
        return false;
    snapmaticJson.jsonObject = jsonValue.get_object();

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

    if (cacheEnabled) {
        picOk = cachePicture.loadFromData(QByteArray::fromRawData(p_ragePhoto.jpegData(), p_ragePhoto.jpegSize()), "JPEG");
        picRes = cachePicture.size();
    }
    else {
        QImage tempPicture;
        picOk = tempPicture.loadFromData(QByteArray::fromRawData(p_ragePhoto.jpegData(), p_ragePhoto.jpegSize()), "JPEG");
        picRes = tempPicture.size();
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
    quint32 photoFormat = p_ragePhoto.format();
    quint32 jpegPicStreamLength = 0;
    if (gta5view_isGTAVFormat(photoFormat)) {
        jpegPicStreamLength = RagePhoto::DEFAULT_GTA5_PHOTOBUFFER;
    }
    else if (gta5view_isRDR2Format(photoFormat)) {
        jpegPicStreamLength = RagePhoto::DEFAULT_RDR2_PHOTOBUFFER;
    }
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
        return setPictureStream(picByteArray, picture.width(), picture.height());
    return false;
}

bool SnapmaticPicture::setPictureStream(const QByteArray &streamArray, int width, int height)
{
    quint32 photoFormat = p_ragePhoto.format();
    quint32 jpegPicStreamLength = 0;
    if (gta5view_isGTAVFormat(photoFormat)) {
        jpegPicStreamLength = RagePhoto::DEFAULT_GTA5_PHOTOBUFFER;
    }
    else if (gta5view_isRDR2Format(photoFormat)) {
        jpegPicStreamLength = RagePhoto::DEFAULT_RDR2_PHOTOBUFFER;
    }
    if (streamArray.size() > jpegPicStreamLength)
        jpegPicStreamLength = streamArray.size();
#ifdef GTA5SYNC_COMPACT_PHOTOBUFFER // Experiment to save less than the default photo buffer
    if (streamArray.size() < jpegPicStreamLength)
        jpegPicStreamLength = streamArray.size();
#endif
    bool success = p_ragePhoto.setJpeg(streamArray.constData(), streamArray.size(), jpegPicStreamLength);
    if (success) {
        // Update JPEG signature
        if (gta5view_isGTAVFormat(photoFormat)) {
            snapmaticJson.jsonObject["sign"] = p_ragePhoto.jpegSign(RagePhoto::PhotoFormat::GTA5);
            const std::string json = SnapmaticJson::serialize(snapmaticJson.jsonObject);
            p_ragePhoto.setJson(json.c_str());
        }
        else if (gta5view_isRDR2Format(photoFormat)) {
            snapmaticJson.jsonObject["sign"] = p_ragePhoto.jpegSign(RagePhoto::PhotoFormat::RDR2);
            snapmaticJson.jsonObject["size"] = streamArray.size();
            snapmaticJson.jsonObject["width"] = width;
            snapmaticJson.jsonObject["height"] = height;
            const std::string json = SnapmaticJson::serialize(snapmaticJson.jsonObject);
            p_ragePhoto.setJson(json.c_str());
        }

        // Update resolution
        picRes = QSize(width, height);

        // Update cache
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
        newTitle.resize(39);
    p_ragePhoto.setTitle(newTitle.toUtf8().constData());
    return true;
}

const QString SnapmaticPicture::getExportPictureFileName()
{
    return picExportFileName;
}

const QString SnapmaticPicture::getOriginalPictureFileName()
{
    QString newPicFileName = picFileName;
    if (picFileName.endsWith(".bak", Qt::CaseInsensitive))
        newPicFileName.resize(newPicFileName.length() - 4);
    if (picFileName.endsWith(".hidden", Qt::CaseInsensitive))
        newPicFileName.resize(newPicFileName.length() - 7);
    return newPicFileName;
}

const QString SnapmaticPicture::getOriginalPictureFilePath()
{
    QString newPicFilePath = picFilePath;
    if (picFileName.endsWith(".bak", Qt::CaseInsensitive))
        newPicFilePath.resize(newPicFilePath.length() - 4);
    if (picFileName.endsWith(".hidden", Qt::CaseInsensitive))
        newPicFilePath.resize(newPicFilePath.length() - 7);
    return newPicFilePath;
}

const QString SnapmaticPicture::getPictureFileName()
{
    return picFileName;
}

const QString SnapmaticPicture::getPictureFilePath()
{
    return picFilePath;
}

const QString SnapmaticPicture::getPictureSortStr()
{
    return sortStr;
}

const QString SnapmaticPicture::getPictureTitl()
{
    return QString::fromUtf8(p_ragePhoto.title());
}

const QString SnapmaticPicture::getPictureStr()
{
    return pictureStr;
}

const QString SnapmaticPicture::getLastStep(bool readable)
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

void SnapmaticPicture::initialise(uint32_t photoFormat)
{
    switch (photoFormat) {
    case RagePhoto::PhotoFormat::GTA5:
    case G5EPhotoFormat::G5EX:
    {
        p_ragePhoto.setFormat(photoFormat);
        p_ragePhoto.setHeader("PHOTO - 09/20/23 04:41:35", 0x97D5BDBDUL, 0x00000000UL);
        p_ragePhoto.setJpeg(std::string(), RagePhoto::DefaultSize::DEFAULT_GTA5_PHOTOBUFFER);

        boost::json::object t_jsonObject;
        t_jsonObject["area"] = "SANAND";
        t_jsonObject["crewid"] = 0;
        t_jsonObject["cv"] = true;
        t_jsonObject["drctr"] = false;

        boost::json::object t_locObject;
        t_locObject["x"] = 0;
        t_locObject["y"] = 0;
        t_locObject["z"] = 0;

        t_jsonObject["loc"] = t_locObject;
        t_jsonObject["meme"] = false;
        t_jsonObject["mid"] = "";
        t_jsonObject["mode"] = "FREEMODE";
        t_jsonObject["mug"] = false;
        t_jsonObject["nm"] = "";
        t_jsonObject["rds"] = "";
        t_jsonObject["rsedtr"] = false;
        t_jsonObject["scr"] = 1;
        t_jsonObject["sid"] = "0x0";
        t_jsonObject["slf"] = true;
        t_jsonObject["street"] = 0;

        pcg32_random_t rng;
        pcg32_srandom_r(&rng, QDateTime::currentMSecsSinceEpoch(), (intptr_t)&rng);
        uint32_t secondsInYear = pcg32_boundedrand_r(&rng, 31535999UL);
        uint32_t timestamp = 1356998400UL + secondsInYear;
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp, Qt::UTC);

        boost::json::object t_timeObject;
        t_timeObject["day"] = dateTime.date().day();
        t_timeObject["hour"] = dateTime.time().hour();
        t_timeObject["minute"] = dateTime.time().minute();
        t_timeObject["month"] = dateTime.date().month();
        t_timeObject["second"] = dateTime.time().second();
        t_timeObject["year"] = dateTime.date().year();

        t_jsonObject["time"] = t_timeObject;

        const std::string json = SnapmaticJson::serialize(t_jsonObject, false);
        setJsonStr(json, true);
        jsonOk = true;

        p_ragePhoto.setDescription("");

        isPreLoaded = true;
        picOk = true; // TODO: the picture is still "not ok", but soon after it will be, but we should not assume it
    }
        break;
    case RagePhoto::PhotoFormat::RDR2:
    {
        p_ragePhoto.setFormat(photoFormat);
        p_ragePhoto.setHeader("PHOTO - 09/20/23 04:39:16", 0x0F5B0A65UL, 0xDF91D3D2UL);
        p_ragePhoto.setJpeg(std::string(), RagePhoto::DefaultSize::DEFAULT_RDR2_PHOTOBUFFER);

        boost::json::object t_jsonObject;
        t_jsonObject["advanced"] = false;
        t_jsonObject["crewid"] = 0;
        t_jsonObject["districtname"] = 0;
        t_jsonObject["drctr"] = false;
        t_jsonObject["inphotomode"] = true;

        boost::json::object t_locObject;
        t_locObject["x"] = 0;
        t_locObject["y"] = 0;
        t_locObject["z"] = 0;

        t_jsonObject["loc"] = t_locObject;
        t_jsonObject["meme"] = false;
        t_jsonObject["mid"] = "";
        t_jsonObject["mode"] = "SP";
        t_jsonObject["mug"] = false;
        t_jsonObject["nm"] = "";
        t_jsonObject["regionname"] = 0;
        t_jsonObject["rsedtr"] = false;
        t_jsonObject["sid"] = "0x0";
        t_jsonObject["slf"] = false;
        t_jsonObject["statename"] = 0;

        pcg32_random_t rng;
        pcg32_srandom_r(&rng, QDateTime::currentMSecsSinceEpoch(), (intptr_t)&rng);
        uint32_t secondsInYear = pcg32_boundedrand_r(&rng, 31535999UL);
        int64_t timestamp = -2240524800L + secondsInYear;
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp, Qt::UTC);

        boost::json::object t_timeObject;
        t_timeObject["day"] = dateTime.date().day();
        t_timeObject["hour"] = dateTime.time().hour();
        t_timeObject["minute"] = dateTime.time().minute();
        t_timeObject["month"] = dateTime.date().month();
        t_timeObject["second"] = dateTime.time().second();
        t_timeObject["year"] = dateTime.date().year();

        t_jsonObject["time"] = t_timeObject;

        const std::string json = SnapmaticJson::serialize(t_jsonObject, false);
        setJsonStr(json, true);
        jsonOk = true;

        p_ragePhoto.setDescription("");

        isPreLoaded = true;
        picOk = true; // TODO: the picture is still "not ok", but soon after it will be, but we should not assume it
    }
        break;
    }
}

const QImage SnapmaticPicture::getImage()
{
    if (cacheEnabled)
        return cachePicture;
    else
        return QImage::fromData(QByteArray::fromRawData(p_ragePhoto.jpegData(), p_ragePhoto.jpegSize()), "JPEG");
    return QImage();
}

const QByteArray SnapmaticPicture::getPictureStream()
{
    return QByteArray::fromRawData(p_ragePhoto.jpegData(), p_ragePhoto.jpegSize());
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

const QString SnapmaticPicture::getJsonStr()
{
    return QString::fromUtf8(p_ragePhoto.json());
}

const std::string SnapmaticPicture::getJsonStdStr()
{
    return std::string(p_ragePhoto.json());
}

SnapmaticProperties SnapmaticPicture::getSnapmaticProperties()
{
    return localProperties;
}

void SnapmaticPicture::parseJsonContent()
{
    const uint32_t format = p_ragePhoto.format();
    const boost::json::object &t_jsonObject = snapmaticJson.jsonObject;

    bool jsonIncomplete = false;
    bool jsonError = false;
    if (t_jsonObject.contains("loc")) {
        if (t_jsonObject.at("loc").is_object()) {
            const boost::json::object locObject = t_jsonObject.at("loc").get_object();
            if (locObject.contains("x")) {
                if (locObject.at("x").is_double()) { localProperties.location.x = locObject.at("x").get_double(); }
                else if (locObject.at("x").is_int64()) { localProperties.location.x = static_cast<double>(locObject.at("x").get_int64()); }
                else if (locObject.at("x").is_uint64()) { localProperties.location.x = static_cast<double>(locObject.at("x").get_uint64()); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
            if (locObject.contains("y")) {
                if (locObject.at("y").is_double()) { localProperties.location.y = locObject.at("y").get_double(); }
                else if (locObject.at("y").is_int64()) { localProperties.location.x = static_cast<double>(locObject.at("y").get_int64()); }
                else if (locObject.at("y").is_uint64()) { localProperties.location.x = static_cast<double>(locObject.at("y").get_uint64()); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
            if (locObject.contains("z")) {
                if (locObject.at("z").is_double()) { localProperties.location.z = locObject.at("z").get_double(); }
                else if (locObject.at("z").is_int64()) { localProperties.location.x = static_cast<double>(locObject.at("z").get_int64()); }
                else if (locObject.at("z").is_uint64()) { localProperties.location.x = static_cast<double>(locObject.at("z").get_uint64()); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
        }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (t_jsonObject.contains("uid")) {
        if (t_jsonObject.at("uid").is_uint64()) { localProperties.uid = t_jsonObject.at("uid").get_uint64(); }
        else if (t_jsonObject.at("uid").is_int64()) { localProperties.uid = static_cast<uint64_t>(t_jsonObject.at("uid").get_int64()); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (t_jsonObject.contains("area")) {
        if (t_jsonObject.at("area").is_string()) {
            localProperties.location.area = QString::fromUtf8(t_jsonObject.at("area").get_string().c_str());
        }
        else { jsonError = true; }
    }
    else if (gta5view_isGTAVFormat(format)) { jsonIncomplete = true; }
    if (t_jsonObject.contains("crewid")) {
        if (t_jsonObject.at("crewid").is_uint64()) { localProperties.crewID = t_jsonObject.at("crewid").get_uint64(); }
        else if (t_jsonObject.at("crewid").is_int64()) { localProperties.crewID = static_cast<uint64_t>(t_jsonObject.at("crewid").get_int64()); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (t_jsonObject.contains("street")) {
        if (t_jsonObject.at("street").is_uint64()) { localProperties.streetID = t_jsonObject.at("street").get_uint64(); }
        else if (t_jsonObject.at("street").is_int64()) { localProperties.streetID = static_cast<uint64_t>(t_jsonObject.at("street").get_int64()); }
        else { jsonError = true; }
    }
    else if (gta5view_isGTAVFormat(format)) { jsonIncomplete = true; }
    if (t_jsonObject.contains("creat")) {
        if (t_jsonObject.at("creat").is_int64()) {
            QDateTime createdTimestamp;
            localProperties.createdTimestamp = t_jsonObject.at("creat").get_int64();
            createdTimestamp.setSecsSinceEpoch(localProperties.createdTimestamp);
            localProperties.createdDateTime = createdTimestamp;
        }
        else if (t_jsonObject.at("creat").is_uint64()) {
            QDateTime createdTimestamp;
            localProperties.createdTimestamp = static_cast<int64_t>(t_jsonObject.at("creat").get_uint64());
            createdTimestamp.setSecsSinceEpoch(localProperties.createdTimestamp);
            localProperties.createdDateTime = createdTimestamp;
        }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (t_jsonObject.contains("plyrs")) {
        if (t_jsonObject.at("plyrs").is_array()) {
            const boost::json::array plyrsArray = t_jsonObject.at("plyrs").get_array();
            QStringList playersList;
            for (const boost::json::value &plyrVal : plyrsArray) {
                if (plyrVal.is_string()) {
                    playersList << QString::fromUtf8(plyrVal.get_string().c_str());
                }
            }
            localProperties.playersList = playersList;
        }
        else { jsonError = true; }
    }
    if (t_jsonObject.contains("meme")) {
        if (t_jsonObject.at("meme").is_bool()) { localProperties.isMeme = t_jsonObject.at("meme").get_bool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (t_jsonObject.contains("mug")) {
        if (t_jsonObject.at("mug").is_bool()) { localProperties.isMug = t_jsonObject.at("mug").get_bool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (t_jsonObject.contains("slf")) {
        if (t_jsonObject.at("slf").is_bool()) { localProperties.isSelfie = t_jsonObject.at("slf").get_bool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (t_jsonObject.contains("drctr")) {
        if (t_jsonObject.at("drctr").is_bool()) { localProperties.isFromDirector = t_jsonObject.at("drctr").get_bool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (t_jsonObject.contains("rsedtr")) {
        if (t_jsonObject.at("rsedtr").is_bool()) { localProperties.isFromRSEditor = t_jsonObject.at("rsedtr").get_bool(); }
        else { jsonError = true; }
    }
    else { localProperties.isFromRSEditor = false; }
    if (t_jsonObject.contains("onislandx")) {
        if (t_jsonObject.at("onislandx").is_bool()) { localProperties.location.isCayoPerico = t_jsonObject.at("onislandx").get_bool(); }
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
    boost::json::object t_jsonObject = snapmaticJson.jsonObject;

    boost::json::object locObject;
    locObject["x"] = properties.location.x;
    locObject["y"] = properties.location.y;
    locObject["z"] = properties.location.z;

    boost::json::array plyrsArray;
    for (const QString &player : properties.playersList) {
        plyrsArray.push_back(player.toUtf8().constData());
    }

    t_jsonObject["loc"] = locObject;
    t_jsonObject["uid"] = properties.uid;
    t_jsonObject["crewid"] = properties.crewID;
    t_jsonObject["creat"] = properties.createdTimestamp;
    t_jsonObject["plyrs"] = plyrsArray;
    t_jsonObject["meme"] = properties.isMeme;
    t_jsonObject["mug"] = properties.isMug;
    t_jsonObject["slf"] = properties.isSelfie;
    t_jsonObject["drctr"] = properties.isFromDirector;
    t_jsonObject["rsedtr"] = properties.isFromRSEditor;
    if (gta5view_isGTAVFormat(p_ragePhoto.format())) {
        t_jsonObject["area"] = properties.location.area.toUtf8().constData();
        t_jsonObject["street"] = properties.streetID;
        t_jsonObject["onislandx"] = properties.location.isCayoPerico;
    }

    const std::string json = SnapmaticJson::serialize(t_jsonObject);
    if (setJsonStr(QString::fromUtf8(json.c_str(), json.size()))) {
        localProperties = properties;
        return true;
    }
    return false;
}

bool SnapmaticPicture::setJsonStr(const QString &json, bool updateProperties)
{
    return setJsonStr(json.toStdString(), updateProperties);
}

bool SnapmaticPicture::setJsonStr(const std::string &json, bool updateProperties)
{
    boost::json::error_code ec;
    const boost::json::value t_jsonValue = boost::json::parse(json, ec);
    if (ec)
        return false;
    if (!t_jsonValue.is_object())
        return false;
    const std::string t_json = SnapmaticJson::serialize(t_jsonValue);
    snapmaticJson.jsonObject = t_jsonValue.get_object();

    p_ragePhoto.setJson(t_json.c_str());
    if (updateProperties)
        parseJsonContent();
    return true;
}

// FILE MANAGEMENT

bool SnapmaticPicture::exportPicture(const QString &fileName, SnapmaticFormat format)
{
    if (format == SnapmaticFormat::Auto_Format) {
        if (p_ragePhoto.format() == G5EPhotoFormat::G5EX)
            format = SnapmaticFormat::G5E_Format;
        else if (p_ragePhoto.format() == RagePhoto::PhotoFormat::GTA5)
            format = SnapmaticFormat::PGTA5_Format;
        else if (p_ragePhoto.format() == RagePhoto::PhotoFormat::RDR2)
            format = SnapmaticFormat::PRDR3_Format;
        else
            format = SnapmaticFormat::Unknown_Format;
    }

    bool saveSuccess = false;
    QSaveFile picFile(fileName);
    if (picFile.open(QIODevice::WriteOnly)) {
        if (format == SnapmaticFormat::G5E_Format) {
            gta5view_export_save(&picFile, p_ragePhoto.data());
            saveSuccess = picFile.commit();
        }
        else if (format == SnapmaticFormat::JPEG_Format) {
            picFile.write(QByteArray::fromRawData(p_ragePhoto.jpegData(), p_ragePhoto.jpegSize()));
            saveSuccess = picFile.commit();
        }
        else {
            bool ok;
            std::string photo;
            if (format == SnapmaticFormat::PGTA5_Format)
                photo = p_ragePhoto.save(RagePhoto::PhotoFormat::GTA5, &ok);
            else if (format == SnapmaticFormat::PRDR3_Format)
                photo = p_ragePhoto.save(RagePhoto::PhotoFormat::RDR2, &ok);
            else
                photo = p_ragePhoto.save(&ok);
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
        const QString picBakPath = QString(picFilePath).chopped(7) % ".bak";
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
    if (picFilePath.endsWith(".hidden", Qt::CaseInsensitive))
        return true;
    return false;
}

bool SnapmaticPicture::isVisible()
{
    if (picFilePath.endsWith(".hidden", Qt::CaseInsensitive))
        return false;
    return true;
}

bool SnapmaticPicture::setPictureHidden()
{
    if (p_ragePhoto.format() == G5EPhotoFormat::G5EX) {
        return false;
    }
    if (isVisible()) {
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
    if (p_ragePhoto.format() == G5EPhotoFormat::G5EX)
        return false;
    if (isHidden()) {
        QString newPicFilePath = QString(picFilePath).chopped(7);
        if (QFile::rename(picFilePath, newPicFilePath)) {
            picFilePath = newPicFilePath;
            return true;
        }
        return false;
    }
    return true;
}

const QSize SnapmaticPicture::getPictureResolution()
{
    return picRes;
}

// SNAPMATIC FORMAT

SnapmaticFormat SnapmaticPicture::getSnapmaticFormat()
{
    if (p_ragePhoto.format() == G5EPhotoFormat::G5EX)
        return SnapmaticFormat::G5E_Format;
    else if (p_ragePhoto.format() == RagePhoto::PhotoFormat::GTA5)
        return SnapmaticFormat::PGTA5_Format;
    else if (p_ragePhoto.format() == RagePhoto::PhotoFormat::RDR2)
        return SnapmaticFormat::PRDR3_Format;
    else
        return SnapmaticFormat::Unknown_Format;
}

void SnapmaticPicture::setSnapmaticFormat(SnapmaticFormat format)
{
    if (format == SnapmaticFormat::G5E_Format) {
        p_ragePhoto.setFormat(G5EPhotoFormat::G5EX);
        return;
    }
    else if (format == SnapmaticFormat::PGTA5_Format) {
        p_ragePhoto.setFormat(RagePhoto::PhotoFormat::GTA5);
        return;
    }
    else if (format == SnapmaticFormat::PRDR3_Format) {
        p_ragePhoto.setFormat(RagePhoto::PhotoFormat::RDR2);
        return;
    }
    qDebug() << "setSnapmaticFormat: Invalid SnapmaticFormat defined, valid SnapmaticFormats are G5E_Format, PGTA5_Format and PRDR3_Format";
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
            if (!verifyTitleChar(titleChar))
                return false;
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
