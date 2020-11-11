/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2020 Syping
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

#include "RagePhoto.h"
#include <QJsonDocument>
#include <QTextCodec>
#include <QBuffer>
#include <QFile>

RagePhoto::RagePhoto()
{
    p_photoFormat = PhotoFormat::Undefined;
    p_isLoaded = false;
    p_inputMode = -1;
}

RagePhoto::RagePhoto(const QByteArray &data) : p_fileData(data)
{
    p_photoFormat = PhotoFormat::Undefined;
    p_isLoaded = false;
    p_inputMode = 0;
}

RagePhoto::RagePhoto(const QString &filePath) : p_filePath(filePath)
{
    p_photoFormat = PhotoFormat::Undefined;
    p_isLoaded = false;
    p_inputMode = 1;
}

RagePhoto::RagePhoto(QIODevice *ioDevice) : p_ioDevice(ioDevice)
{
    p_photoFormat = PhotoFormat::Undefined;
    p_isLoaded = false;
    p_inputMode = 2;
}

bool RagePhoto::isLoaded()
{
    return p_isLoaded;
}

bool RagePhoto::load()
{
    if (p_inputMode == -1)
        return false;

    if (p_isLoaded)
        clear();

    if (p_inputMode == 1) {
        QFile pictureFile(p_filePath);
        if (pictureFile.open(QIODevice::ReadOnly)) {
            p_fileData = pictureFile.readAll();
        }
        pictureFile.close();
    }
    else if (p_inputMode == 2) {
        if (!p_ioDevice->isOpen()) {
            if (!p_ioDevice->open(QIODevice::ReadOnly))
                return false;
        }
        p_fileData = p_ioDevice->readAll();
    }

    QBuffer dataBuffer(&p_fileData);
    dataBuffer.open(QIODevice::ReadOnly);

    char uInt32Buffer[4];
    qint64 size = dataBuffer.read(uInt32Buffer, 4);
    if (size != 4)
        return false;
    quint32 format = charToUInt32LE(uInt32Buffer);

    if (format == static_cast<quint32>(PhotoFormat::GTA5)) {
        char photoHeader[256];
        size = dataBuffer.read(photoHeader, 256);
        if (size != 256)
            return false;
        for (const QChar &photoChar : QTextCodec::codecForName("UTF-16LE")->toUnicode(photoHeader, 256)) {
            if (photoChar.isNull())
                break;
            p_photoString += photoChar;
        }

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        p_headerSum = charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        p_endOfFile = charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        p_jsonOffset = charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        p_titlOffset = charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        p_descOffset = charToUInt32LE(uInt32Buffer);

        char markerBuffer[4];
        size = dataBuffer.read(markerBuffer, 4);
        if (size != 4)
            return false;
        if (strncmp(markerBuffer, "JPEG", 4) != 0)
            return false;

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        p_photoBuffer = charToUInt32LE(uInt32Buffer);

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        quint32 t_photoSize = charToUInt32LE(uInt32Buffer);

        char photoData[t_photoSize];
        size = dataBuffer.read(photoData, t_photoSize);
        if (size != t_photoSize)
            return false;
        p_photoData = QByteArray(photoData, t_photoSize);

        dataBuffer.seek(p_jsonOffset + 264);
        size = dataBuffer.read(markerBuffer, 4);
        if (size != 4)
            return false;
        if (strncmp(markerBuffer, "JSON", 4) != 0)
            return false;

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        p_jsonBuffer = charToUInt32LE(uInt32Buffer);

        char jsonBytes[p_jsonBuffer];
        size = dataBuffer.read(jsonBytes, p_jsonBuffer);
        if (size != p_jsonBuffer)
            return false;
        for (quint32 i = 0; i != p_jsonBuffer; i++) {
            if (jsonBytes[i] == '\x00')
                break;
            p_jsonData += jsonBytes[i];
        }
        QJsonDocument t_jsonDocument = QJsonDocument::fromJson(p_jsonData);
        if (t_jsonDocument.isNull())
            return false;
        p_jsonObject = t_jsonDocument.object();

        dataBuffer.seek(p_titlOffset + 264);
        size = dataBuffer.read(markerBuffer, 4);
        if (size != 4)
            return false;
        if (strncmp(markerBuffer, "TITL", 4) != 0)
            return false;

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        p_titlBuffer = charToUInt32LE(uInt32Buffer);

        char titlBytes[p_titlBuffer];
        size = dataBuffer.read(titlBytes, p_titlBuffer);
        if (size != p_titlBuffer)
            return false;
        for (const QChar &titlChar : QString::fromUtf8(titlBytes, p_titlBuffer)) {
            if (titlChar.isNull())
                break;
            p_titleString += titlChar;
        }

        dataBuffer.seek(p_descOffset + 264);
        size = dataBuffer.read(markerBuffer, 4);
        if (size != 4)
            return false;
        if (strncmp(markerBuffer, "DESC", 4) != 0)
            return false;

        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        p_descBuffer = charToUInt32LE(uInt32Buffer);

        char descBytes[p_descBuffer];
        size = dataBuffer.read(descBytes, p_descBuffer);
        if (size != p_descBuffer)
            return false;
        for (const QChar &descChar : QString::fromUtf8(descBytes, p_descBuffer)) {
            if (descChar.isNull())
                break;
            p_descriptionString += descChar;
        }

        dataBuffer.seek(p_endOfFile + 260);
        size = dataBuffer.read(markerBuffer, 4);
        if (size != 4)
            return false;
        if (strncmp(markerBuffer, "JEND", 4) != 0)
            return false;

        if (p_photoFormat != PhotoFormat::G5EX)
            p_photoFormat = PhotoFormat::GTA5;

        p_fileData.clear();
        p_isLoaded = true;
        return true;
    }
    else if (format == static_cast<quint32>(PhotoFormat::G5EX)) {
        size = dataBuffer.read(uInt32Buffer, 4);
        if (size != 4)
            return false;
        format = charToUInt32LE(uInt32Buffer);
        if (format == static_cast<quint32>(ExportFormat::G5E3P)) {
            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            quint32 compressedSize = charToUInt32LE(uInt32Buffer);

            char compressedPhotoHeader[compressedSize];
            size = dataBuffer.read(compressedPhotoHeader, compressedSize);
            if (size != compressedSize)
                return false;
            QByteArray t_photoHeader = QByteArray::fromRawData(compressedPhotoHeader, compressedSize);
            t_photoHeader = qUncompress(t_photoHeader);
            if (t_photoHeader.isEmpty())
                return false;
            p_photoString = QString::fromUtf8(t_photoHeader);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            p_headerSum = charToUInt32LE(uInt32Buffer);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            p_photoBuffer = charToUInt32LE(uInt32Buffer);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            compressedSize = charToUInt32LE(uInt32Buffer);

            char compressedPhoto[compressedSize];
            size = dataBuffer.read(compressedPhoto, compressedSize);
            if (size != compressedSize)
                return false;
            QByteArray t_photoData = QByteArray::fromRawData(compressedPhoto, compressedSize);
            p_photoData = qUncompress(t_photoData);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            p_jsonOffset = charToUInt32LE(uInt32Buffer);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            p_jsonBuffer = charToUInt32LE(uInt32Buffer);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            compressedSize = charToUInt32LE(uInt32Buffer);

            char compressedJson[compressedSize];
            size = dataBuffer.read(compressedJson, compressedSize);
            if (size != compressedSize)
                return false;
            QByteArray t_jsonBytes = QByteArray::fromRawData(compressedJson, compressedSize);
            p_jsonData = qUncompress(t_jsonBytes);
            if (p_jsonData.isEmpty())
                return false;
            QJsonDocument t_jsonDocument = QJsonDocument::fromJson(p_jsonData);
            if (t_jsonDocument.isNull())
                return false;
            p_jsonObject = t_jsonDocument.object();

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            p_titlOffset = charToUInt32LE(uInt32Buffer);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            p_titlBuffer = charToUInt32LE(uInt32Buffer);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            compressedSize = charToUInt32LE(uInt32Buffer);

            char compressedTitl[compressedSize];
            size = dataBuffer.read(compressedTitl, compressedSize);
            if (size != compressedSize)
                return false;
            QByteArray t_titlBytes = QByteArray::fromRawData(compressedTitl, compressedSize);
            t_titlBytes = qUncompress(t_titlBytes);
            p_titleString = QString::fromUtf8(t_titlBytes);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            p_descOffset = charToUInt32LE(uInt32Buffer);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            p_descBuffer = charToUInt32LE(uInt32Buffer);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            compressedSize = charToUInt32LE(uInt32Buffer);

            char compressedDesc[compressedSize];
            size = dataBuffer.read(compressedDesc, compressedSize);
            if (size != compressedSize)
                return false;
            QByteArray t_descBytes = QByteArray::fromRawData(compressedDesc, compressedSize);
            t_descBytes = qUncompress(t_descBytes);
            p_descriptionString = QString::fromUtf8(t_descBytes);

            size = dataBuffer.read(uInt32Buffer, 4);
            if (size != 4)
                return false;
            p_endOfFile = charToUInt32LE(uInt32Buffer);

            p_photoFormat = PhotoFormat::G5EX;

            p_fileData.clear();
            p_isLoaded = true;
            return true;
        }
        else if (format == static_cast<quint32>(ExportFormat::G5E2P)) {
            p_photoFormat = PhotoFormat::G5EX;
            p_fileData = qUncompress(dataBuffer.readAll());
            if (p_fileData.isEmpty())
                return false;
            p_inputMode = 0;
            return load();
        }
        else if (format == static_cast<quint32>(ExportFormat::G5E1P)) {
            size = dataBuffer.skip(1);
            if (size != 1)
                return false;

            char length[1];
            size = dataBuffer.read(length, 1);
            if (size != 1)
                return false;
            int i_length = QByteArray::number((int)length[0], 16).toInt() + 6;

            size = dataBuffer.skip(i_length);
            if (size != i_length)
                return false;

            p_photoFormat = PhotoFormat::G5EX;
            p_fileData = qUncompress(dataBuffer.readAll());
            if (p_fileData.isEmpty())
                return false;
            p_inputMode = 0;
            return load();
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

void RagePhoto::clear()
{
    p_photoFormat = PhotoFormat::Undefined;
    p_jsonObject = QJsonObject();
    p_descriptionString.clear();
    p_jsonData.clear();
    p_photoData.clear();
    p_photoString.clear();
    p_titleString.clear();
    p_headerSum = 0;
    p_isLoaded = false;
}

void RagePhoto::setDescription(const QString &description)
{
    p_descriptionString = description;
}

void RagePhoto::setFileData(const QByteArray &data)
{
    p_fileData = data;
    p_inputMode = 0;
}

void RagePhoto::setFilePath(const QString &filePath)
{
    p_filePath = filePath;
    p_inputMode = 1;
}

void RagePhoto::setIODevice(QIODevice *ioDevice)
{
    p_ioDevice = ioDevice;
    p_inputMode = 2;
}

bool RagePhoto::setJsonData(const QByteArray &data)
{
    QJsonDocument t_jsonDocument = QJsonDocument::fromJson(data);
    if (t_jsonDocument.isNull())
        return false;
    p_jsonData = t_jsonDocument.toJson(QJsonDocument::Compact);
    p_jsonObject = t_jsonDocument.object();
    return true;
}

bool RagePhoto::setPhotoData(const QByteArray &data)
{
    quint32 size = data.size();
    if (size > p_photoBuffer)
        return false;
    p_photoData = data;
    return true;
}

bool RagePhoto::setPhotoData(const char *data, int size)
{
    if ((quint32)size > p_photoBuffer)
        return false;
    p_photoData = QByteArray(data, size);
    return true;
}

void RagePhoto::setPhotoFormat(PhotoFormat photoFormat)
{
    p_photoFormat = photoFormat;
}

void RagePhoto::setTitle(const QString &title)
{
    p_titleString = title;
}

const QByteArray RagePhoto::jsonData(JsonFormat jsonFormat)
{
    if (jsonFormat == JsonFormat::Compact) {
        return QJsonDocument(p_jsonObject).toJson(QJsonDocument::Compact);
    }
    else if (jsonFormat == JsonFormat::Indented) {
        return QJsonDocument(p_jsonObject).toJson(QJsonDocument::Indented);
    }
    else {
        return p_jsonData;
    }
}

const QJsonObject RagePhoto::jsonObject()
{
    return p_jsonObject;
}

const QByteArray RagePhoto::photoData()
{
    return p_photoData;
}

const QString RagePhoto::description()
{
    return p_descriptionString;
}

const QString RagePhoto::photoString()
{
    return p_photoString;
}

const QString RagePhoto::title()
{
    return p_titleString;
}

quint32 RagePhoto::photoBuffer()
{
    return p_photoBuffer;
}

quint32 RagePhoto::photoSize()
{
    return p_photoData.size();
}

RagePhoto::PhotoFormat RagePhoto::photoFormat()
{
    return p_photoFormat;
}

QByteArray RagePhoto::save(PhotoFormat photoFormat)
{
    QByteArray data;
    QBuffer dataBuffer(&data);
    dataBuffer.open(QIODevice::WriteOnly);
    save(&dataBuffer, photoFormat);
    return data;
}

void RagePhoto::save(QIODevice *ioDevice, PhotoFormat photoFormat)
{
    if (photoFormat == PhotoFormat::G5EX) {
        char uInt32Buffer[4];
        quint32 format = static_cast<quint32>(PhotoFormat::G5EX);
        uInt32ToCharLE(&format, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);
        format = static_cast<quint32>(ExportFormat::G5E3P);
        uInt32ToCharLE(&format, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        QByteArray compressedData = qCompress(p_photoString.toUtf8(), 9);
        quint32 compressedSize = compressedData.size();
        uInt32ToCharLE(&compressedSize, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);
        ioDevice->write(compressedData);

        uInt32ToCharLE(&p_headerSum, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        uInt32ToCharLE(&p_photoBuffer, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        compressedData = qCompress(p_photoData, 9);
        compressedSize = compressedData.size();
        uInt32ToCharLE(&compressedSize, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);
        ioDevice->write(compressedData);

        uInt32ToCharLE(&p_jsonOffset, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        uInt32ToCharLE(&p_jsonBuffer, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        compressedData = qCompress(p_jsonData, 9);
        compressedSize = compressedData.size();
        uInt32ToCharLE(&compressedSize, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);
        ioDevice->write(compressedData);

        uInt32ToCharLE(&p_titlOffset, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        uInt32ToCharLE(&p_titlBuffer, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        compressedData = qCompress(p_titleString.toUtf8(), 9);
        compressedSize = compressedData.size();
        uInt32ToCharLE(&compressedSize, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);
        ioDevice->write(compressedData);

        uInt32ToCharLE(&p_descOffset, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        uInt32ToCharLE(&p_descBuffer, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        compressedData = qCompress(p_descriptionString.toUtf8(), 9);
        compressedSize = compressedData.size();
        uInt32ToCharLE(&compressedSize, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);
        ioDevice->write(compressedData);

        uInt32ToCharLE(&p_endOfFile, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        ioDevice->aboutToClose();
    }
    else if (photoFormat == PhotoFormat::GTA5) {
        char uInt32Buffer[4];
        quint32 format = static_cast<quint32>(PhotoFormat::GTA5);
        uInt32ToCharLE(&format, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        QByteArray photoHeader = QTextCodec::codecForName("UTF-16LE")->fromUnicode(p_photoString);
        if (photoHeader.left(2) == "\xFF\xFE") {
            photoHeader.remove(0, 2);
        }
        qint64 photoHeaderSize = photoHeader.size();
        if (photoHeaderSize > 256) {
            photoHeader = photoHeader.left(256);
            photoHeaderSize = 256;
        }
        ioDevice->write(photoHeader);
        for (qint64 size = photoHeaderSize; size < 256; size++) {
            ioDevice->write("\x00", 1);
        }

        uInt32ToCharLE(&p_headerSum, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        uInt32ToCharLE(&p_endOfFile, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        uInt32ToCharLE(&p_jsonOffset, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        uInt32ToCharLE(&p_titlOffset, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        uInt32ToCharLE(&p_descOffset, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        ioDevice->write("JPEG", 4);

        uInt32ToCharLE(&p_photoBuffer, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        quint32 t_photoSize = p_photoData.size();
        uInt32ToCharLE(&t_photoSize, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        ioDevice->write(p_photoData);
        for (qint64 size = t_photoSize; size < p_photoBuffer; size++) {
            ioDevice->write("\x00", 1);
        }

        ioDevice->seek(p_jsonOffset + 264);
        ioDevice->write("JSON", 4);

        uInt32ToCharLE(&p_jsonBuffer, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        qint64 dataSize = p_jsonData.size();
        ioDevice->write(p_jsonData);
        for (qint64 size = dataSize; size < p_jsonBuffer; size++) {
            ioDevice->write("\x00", 1);
        }

        ioDevice->seek(p_titlOffset + 264);
        ioDevice->write("TITL", 4);

        uInt32ToCharLE(&p_titlBuffer, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        QByteArray data = p_titleString.toUtf8();
        dataSize = data.size();
        ioDevice->write(data);
        for (qint64 size = dataSize; size < p_titlBuffer; size++) {
            ioDevice->write("\x00", 1);
        }

        ioDevice->seek(p_descOffset + 264);
        ioDevice->write("DESC", 4);

        uInt32ToCharLE(&p_descBuffer, uInt32Buffer);
        ioDevice->write(uInt32Buffer, 4);

        data = p_descriptionString.toUtf8();
        dataSize = data.size();
        ioDevice->write(data);
        for (qint64 size = dataSize; size < p_descBuffer; size++) {
            ioDevice->write("\x00", 1);
        }

        ioDevice->seek(p_endOfFile + 260);
        ioDevice->write("JEND", 4);
        ioDevice->aboutToClose();
    }
}

RagePhoto* RagePhoto::loadFile(const QString &filePath)
{
    RagePhoto *ragePhoto = new RagePhoto(filePath);
    ragePhoto->load();
    return ragePhoto;
}

quint32 RagePhoto::charToUInt32BE(char *x)
{
    return (((unsigned char)x[0] << 24) | ((unsigned char)x[1] << 16) | ((unsigned char)x[2] << 8) | ((unsigned char)x[3]));
}

quint32 RagePhoto::charToUInt32LE(char *x)
{
    return (((unsigned char)x[3] << 24) | ((unsigned char)x[2] << 16) | ((unsigned char)x[1] << 8) | ((unsigned char)x[0]));
}

void RagePhoto::uInt32ToCharBE(quint32 *x, char *y)
{
    y[0] = (*x >> 24) & 0xFF;
    y[1] = (*x >> 16) & 0xFF;
    y[2] = (*x >> 8) & 0xFF;
    y[3] = (*x) & 0xFF;
}

void RagePhoto::uInt32ToCharLE(quint32 *x, char *y)
{
    y[0] = (*x) & 0xFF;
    y[1] = (*x >> 8) & 0xFF;
    y[2] = (*x >> 16) & 0xFF;
    y[3] = (*x >> 24) & 0xFF;
}
