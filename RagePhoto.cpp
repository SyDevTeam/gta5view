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
    p_inputMode = 0;
    p_isLoaded = false;
}

RagePhoto::RagePhoto(const QString &filePath) : p_filePath(filePath)
{
    p_inputMode = 1;
    p_isLoaded = false;
}

RagePhoto::RagePhoto(QIODevice *ioDevice) : p_ioDevice(ioDevice)
{
    p_inputMode = 2;
    p_isLoaded = false;
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

    if (format == PhotoFormat::GTA5) {
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
        p_photoSize = charToUInt32LE(photoSize);

        char photoData[p_photoSize];
        size = dataBuffer.read(photoData, p_photoSize);
        if (size != p_photoSize)
            return false;
        p_photoData = QByteArray::fromRawData(photoData, p_photoSize);

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
        quint32 i_jsonSize = charToUInt32LE(jsonSize);

        char jsonBytes[i_jsonSize];
        size = dataBuffer.read(jsonBytes, i_jsonSize);
        if (size != i_jsonSize)
            return false;
        QByteArray t_jsonBytes;
        for (quint32 i = 0; i != i_jsonSize; i++) {
            if (jsonBytes[i] == '\x00')
                break;
            t_jsonBytes += jsonBytes[i];
        }
        QJsonDocument t_jsonDocument = QJsonDocument::fromJson(t_jsonBytes);
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
        quint32 i_titlSize = charToUInt32LE(titlSize);

        char titlBytes[i_titlSize];
        size = dataBuffer.read(titlBytes, i_titlSize);
        if (size != i_titlSize)
            return false;
        for (const QChar &titlChar : QString::fromUtf8(titlBytes, i_titlSize)) {
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
        quint32 i_descSize = charToUInt32LE(descSize);

        char descBytes[i_descSize];
        size = dataBuffer.read(descBytes, i_descSize);
        if (size != i_descSize)
            return false;
        for (const QChar &descChar : QString::fromUtf8(descBytes, i_descSize)) {
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

        p_fileData.clear();
        p_isLoaded = true;
        return true;
    }
    else if (format == PhotoFormat::G5EX) {
        char formatHeader[4];
        size = dataBuffer.read(formatHeader, 4);
        if (size != 4)
            return false;
        quint32 format = charToUInt32LE(formatHeader);
        if (format == ExportFormat::G5E3P) {
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
            p_photoSize = p_photoData.size();

            char jsonOffset[4];
            size = dataBuffer.read(jsonOffset, 4);
            if (size != 4)
                return false;
            p_jsonOffset = charToUInt32LE(jsonOffset);

            char jsonSize[4];
            size = dataBuffer.peek(jsonSize, 4);
            if (size != 4)
                return false;
            quint32 i_jsonSize = charToUInt32BE(jsonSize) + 4;

            char compressedJson[i_jsonSize];
            size = dataBuffer.read(compressedJson, i_jsonSize);
            if (size != i_jsonSize)
                return false;
            QByteArray t_jsonBytes = QByteArray::fromRawData(compressedJson, i_jsonSize);
            t_jsonBytes = qUncompress(t_jsonBytes);
            QJsonDocument t_jsonDocument = QJsonDocument::fromJson(t_jsonBytes);
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
            quint32 i_titlSize = charToUInt32BE(titlSize) + 4;

            char compressedTitl[i_titlSize];
            size = dataBuffer.read(compressedTitl, i_titlSize);
            if (size != i_titlSize)
                return false;
            QByteArray t_titlBytes = QByteArray::fromRawData(compressedTitl, i_titlSize);
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
            quint32 i_descSize = charToUInt32BE(descSize) + 4;

            char compressedDesc[i_descSize];
            size = dataBuffer.read(compressedDesc, i_descSize);
            if (size != i_descSize)
                return false;
            QByteArray t_descBytes = QByteArray::fromRawData(compressedDesc, i_descSize);
            t_descBytes = qUncompress(t_descBytes);
            p_descriptionString = QString::fromUtf8(t_descBytes);

            char endOfFile[4];
            size = dataBuffer.read(endOfFile, 4);
            if (size != 4)
                return false;
            p_endOfFile = charToUInt32LE(endOfFile);

            p_fileData.clear();
            p_isLoaded = true;
            return true;
        }
        else if (format == ExportFormat::G5E2P) {
            p_fileData = dataBuffer.readAll();
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
    p_jsonObject = QJsonObject();
    p_descriptionString.clear();
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

void RagePhoto::setFilePath(const QString &filePath)
{
    p_filePath = filePath;
    p_inputMode = 0;
}

void RagePhoto::setPhotoData(const QByteArray &data)
{
    p_photoData = data;
}

void RagePhoto::setPhotoData(const char *data, int size)
{
    p_photoData = QByteArray::fromRawData(data, size);
}

void RagePhoto::setTitle(const QString &title)
{
    p_titleString = title;
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
