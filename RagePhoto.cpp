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

    char formatHeader[4];
    qint64 size = dataBuffer.read(formatHeader, 4);
    if (size != 4)
        return false;
    quint32 format = charToUInt32LE(formatHeader);

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

        char checksum[4];
        size = dataBuffer.read(checksum, 4);
        if (size != 4)
            return false;
        p_headerSum = charToUInt32LE(checksum);

        char endOfFile[4];
        size = dataBuffer.read(endOfFile, 4);
        if (size != 4)
            return false;
        p_endOfFile = charToUInt32LE(endOfFile);

        char jsonOffset[4];
        size = dataBuffer.read(jsonOffset, 4);
        if (size != 4)
            return false;
        p_jsonOffset = charToUInt32LE(jsonOffset);

        char titleOffset[4];
        size = dataBuffer.read(titleOffset, 4);
        if (size != 4)
            return false;
        p_titlOffset = charToUInt32LE(titleOffset);

        char descOffset[4];
        size = dataBuffer.read(descOffset, 4);
        if (size != 4)
            return false;
        p_descOffset = charToUInt32LE(descOffset);

        char jpegMarker[4];
        size = dataBuffer.read(jpegMarker, 4);
        if (size != 4)
            return false;
        if (strncmp(jpegMarker, "JPEG", 4) != 0)
            return false;

        char jpegBuffer[4];
        size = dataBuffer.read(jpegBuffer, 4);
        if (size != 4)
            return false;
        p_jpegBuffer = charToUInt32LE(jpegBuffer);

        char photoSize[4];
        size = dataBuffer.read(photoSize, 4);
        if (size != 4)
            return false;
        quint32 t_photoSize = charToUInt32LE(photoSize);

        char photoData[t_photoSize];
        size = dataBuffer.read(photoData, t_photoSize);
        if (size != t_photoSize)
            return false;
        p_photoData = QByteArray(photoData, t_photoSize);

        dataBuffer.seek(p_jsonOffset + 264);
        char jsonMarker[4];
        size = dataBuffer.read(jsonMarker, 4);
        if (size != 4)
            return false;
        if (strncmp(jsonMarker, "JSON", 4) != 0)
            return false;

        char jsonSize[4];
        size = dataBuffer.read(jsonSize, 4);
        if (size != 4)
            return false;
        p_jsonSize = charToUInt32LE(jsonSize);

        char jsonBytes[p_jsonSize];
        size = dataBuffer.read(jsonBytes, p_jsonSize);
        if (size != p_jsonSize)
            return false;
        for (quint32 i = 0; i != p_jsonSize; i++) {
            if (jsonBytes[i] == '\x00')
                break;
            p_jsonData += jsonBytes[i];
        }
        QJsonDocument t_jsonDocument = QJsonDocument::fromJson(p_jsonData);
        if (t_jsonDocument.isNull())
            return false;
        p_jsonObject = t_jsonDocument.object();

        dataBuffer.seek(p_titlOffset + 264);
        char titlMarker[4];
        size = dataBuffer.read(titlMarker, 4);
        if (size != 4)
            return false;
        if (strncmp(titlMarker, "TITL", 4) != 0)
            return false;

        char titlSize[4];
        size = dataBuffer.read(titlSize, 4);
        if (size != 4)
            return false;
        p_titlSize = charToUInt32LE(titlSize);

        char titlBytes[p_titlSize];
        size = dataBuffer.read(titlBytes, p_titlSize);
        if (size != p_titlSize)
            return false;
        for (const QChar &titlChar : QString::fromUtf8(titlBytes, p_titlSize)) {
            if (titlChar.isNull())
                break;
            p_titleString += titlChar;
        }

        dataBuffer.seek(p_descOffset + 264);
        char descMarker[4];
        size = dataBuffer.read(descMarker, 4);
        if (size != 4)
            return false;
        if (strncmp(descMarker, "DESC", 4) != 0)
            return false;

        char descSize[4];
        size = dataBuffer.read(descSize, 4);
        if (size != 4)
            return false;
        p_descSize = charToUInt32LE(descSize);

        char descBytes[p_descSize];
        size = dataBuffer.read(descBytes, p_descSize);
        if (size != p_descSize)
            return false;
        for (const QChar &descChar : QString::fromUtf8(descBytes, p_descSize)) {
            if (descChar.isNull())
                break;
            p_descriptionString += descChar;
        }

        dataBuffer.seek(p_endOfFile + 260);
        char jendMarker[4];
        size = dataBuffer.read(jendMarker, 4);
        if (size != 4)
            return false;
        if (strncmp(jendMarker, "JEND", 4) != 0)
            return false;

        if (p_photoFormat != PhotoFormat::G5EX)
            p_photoFormat = PhotoFormat::GTA5;

        p_fileData.clear();
        p_isLoaded = true;
        return true;
    }
    else if (format == static_cast<quint32>(PhotoFormat::G5EX)) {
        char formatHeader[4];
        size = dataBuffer.read(formatHeader, 4);
        if (size != 4)
            return false;
        quint32 format = charToUInt32LE(formatHeader);
        if (format == static_cast<quint32>(ExportFormat::G5E3P)) {
            char photoHeaderSize[4];
            size = dataBuffer.peek(photoHeaderSize, 4);
            if (size != 4)
                return false;
            quint32 i_photoHeaderSize = charToUInt32BE(photoHeaderSize) + 4;

            char compressedPhotoHeader[i_photoHeaderSize];
            size = dataBuffer.read(compressedPhotoHeader, i_photoHeaderSize);
            if (size != i_photoHeaderSize)
                return false;
            QByteArray t_photoHeaderBytes = QByteArray::fromRawData(compressedPhotoHeader, i_photoHeaderSize);
            t_photoHeaderBytes = qUncompress(t_photoHeaderBytes);
            p_photoString = QString::fromUtf8(t_photoHeaderBytes);

            char checksum[4];
            size = dataBuffer.read(checksum, 4);
            if (size != 4)
                return false;
            p_headerSum = charToUInt32LE(checksum);

            char jpegBuffer[4];
            size = dataBuffer.read(jpegBuffer, 4);
            if (size != 4)
                return false;
            p_jpegBuffer = charToUInt32LE(jpegBuffer);

            char photoSize[4];
            size = dataBuffer.peek(photoSize, 4);
            if (size != 4)
                return false;
            quint32 i_photoSize = charToUInt32BE(photoSize) + 4;

            char compressedPhoto[i_photoSize];
            size = dataBuffer.read(compressedPhoto, i_photoSize);
            if (size != i_photoSize)
                return false;
            QByteArray t_photoData = QByteArray::fromRawData(compressedPhoto, i_photoSize);
            p_photoData = qUncompress(t_photoData);

            char jsonOffset[4];
            size = dataBuffer.read(jsonOffset, 4);
            if (size != 4)
                return false;
            p_jsonOffset = charToUInt32LE(jsonOffset);

            char jsonSize[4];
            size = dataBuffer.peek(jsonSize, 4);
            if (size != 4)
                return false;
            p_jsonSize = charToUInt32BE(jsonSize) + 4;

            char compressedJson[p_jsonSize];
            size = dataBuffer.read(compressedJson, p_jsonSize);
            if (size != p_jsonSize)
                return false;
            QByteArray t_jsonBytes = QByteArray::fromRawData(compressedJson, p_jsonSize);
            p_jsonData = qUncompress(t_jsonBytes);
            QJsonDocument t_jsonDocument = QJsonDocument::fromJson(p_jsonData);
            if (t_jsonDocument.isNull())
                return false;
            p_jsonObject = t_jsonDocument.object();

            char titleOffset[4];
            size = dataBuffer.read(titleOffset, 4);
            if (size != 4)
                return false;
            p_titlOffset = charToUInt32LE(titleOffset);

            char titlSize[4];
            size = dataBuffer.peek(titlSize, 4);
            if (size != 4)
                return false;
            p_titlSize = charToUInt32BE(titlSize) + 4;

            char compressedTitl[p_titlSize];
            size = dataBuffer.read(compressedTitl, p_titlSize);
            if (size != p_titlSize)
                return false;
            QByteArray t_titlBytes = QByteArray::fromRawData(compressedTitl, p_titlSize);
            t_titlBytes = qUncompress(t_titlBytes);
            p_titleString = QString::fromUtf8(t_titlBytes);

            char descOffset[4];
            size = dataBuffer.read(descOffset, 4);
            if (size != 4)
                return false;
            p_descOffset = charToUInt32LE(descOffset);

            char descSize[4];
            size = dataBuffer.peek(descSize, 4);
            if (size != 4)
                return false;
            p_descSize = charToUInt32BE(descSize) + 4;

            char compressedDesc[p_descSize];
            size = dataBuffer.read(compressedDesc, p_descSize);
            if (size != p_descSize)
                return false;
            QByteArray t_descBytes = QByteArray::fromRawData(compressedDesc, p_descSize);
            t_descBytes = qUncompress(t_descBytes);
            p_descriptionString = QString::fromUtf8(t_descBytes);

            char endOfFile[4];
            size = dataBuffer.read(endOfFile, 4);
            if (size != 4)
                return false;
            p_endOfFile = charToUInt32LE(endOfFile);

            p_photoFormat = PhotoFormat::G5EX;

            p_fileData.clear();
            p_isLoaded = true;
            return true;
        }
        else if (format == static_cast<quint32>(ExportFormat::G5E2P)) {
            p_photoFormat = PhotoFormat::G5EX;
            p_fileData = qUncompress(dataBuffer.readAll());
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
    p_jsonObject = t_jsonDocument.object();
    p_jsonData = data;
    return true;
}

bool RagePhoto::setPhotoData(const QByteArray &data)
{
    quint32 size = data.size();
    if (size > p_jpegBuffer)
        return false;
    p_photoData = data;
    return true;
}

bool RagePhoto::setPhotoData(const char *data, int size)
{
    if ((quint32)size > p_jpegBuffer)
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

const QByteArray RagePhoto::jsonData()
{
    return p_jsonData;
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
    return p_jpegBuffer;
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
    if (photoFormat == PhotoFormat::GTA5) {
        char formatHeader[4];
        quint32 format = (quint32)PhotoFormat::GTA5;
        uInt32ToCharLE(&format, formatHeader);
        ioDevice->write(formatHeader, 4);

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

        char checksum[4];
        uInt32ToCharLE(&p_headerSum, checksum);
        ioDevice->write(checksum, 4);

        char endOfFile[4];
        uInt32ToCharLE(&p_endOfFile, endOfFile);
        ioDevice->write(endOfFile, 4);

        char jsonOffset[4];
        uInt32ToCharLE(&p_jsonOffset, jsonOffset);
        ioDevice->write(jsonOffset, 4);

        char titlOffset[4];
        uInt32ToCharLE(&p_titlOffset, titlOffset);
        ioDevice->write(titlOffset, 4);

        char descOffset[4];
        uInt32ToCharLE(&p_descOffset, descOffset);
        ioDevice->write(descOffset, 4);

        ioDevice->write("JPEG");

        char jpegBuffer[4];
        uInt32ToCharLE(&p_jpegBuffer, jpegBuffer);
        ioDevice->write(jpegBuffer, 4);

        quint32 t_photoSize = p_photoData.size();
        char photoSize[4];
        uInt32ToCharLE(&t_photoSize, photoSize);
        ioDevice->write(photoSize, 4);

        ioDevice->write(p_photoData);
        for (qint64 size = t_photoSize; size < p_jpegBuffer; size++) {
            ioDevice->write("\x00", 1);
        }

        ioDevice->seek(p_jsonOffset + 264);
        ioDevice->write("JSON");

        char jsonSize[4];
        uInt32ToCharLE(&p_jsonSize, jsonSize);
        ioDevice->write(jsonSize, 4);

        qint64 dataSize = p_jsonData.size();
        ioDevice->write(p_jsonData);
        for (qint64 size = dataSize; size < p_jsonSize; size++) {
            ioDevice->write("\x00", 1);
        }

        ioDevice->seek(p_titlOffset + 264);
        ioDevice->write("TITL");

        char titlSize[4];
        uInt32ToCharLE(&p_titlSize, titlSize);
        ioDevice->write(titlSize, 4);

        QByteArray data = p_titleString.toUtf8();
        dataSize = data.size();
        ioDevice->write(data);
        for (qint64 size = dataSize; size < p_titlSize; size++) {
            ioDevice->write("\x00", 1);
        }

        ioDevice->seek(p_descOffset + 264);
        ioDevice->write("DESC");

        char descSize[4];
        uInt32ToCharLE(&p_descSize, descSize);
        ioDevice->write(descSize, 4);

        data = p_descriptionString.toUtf8();
        dataSize = data.size();
        ioDevice->write(data);
        for (qint64 size = dataSize; size < p_descSize; size++) {
            ioDevice->write("\x00", 1);
        }

        ioDevice->seek(p_endOfFile + 260);
        ioDevice->write("JEND");
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
