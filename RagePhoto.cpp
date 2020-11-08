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
#include <QDebug>
#include <QFile>

RagePhoto::RagePhoto(const QString &filePath) : p_filePath(filePath)
{
    p_inputMode = 0;
    p_isLoaded = false;
}

RagePhoto::RagePhoto(QIODevice *ioDevice) : p_ioDevice(ioDevice)
{
    p_inputMode = 1;
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

    QByteArray readData;
    if (p_inputMode == 0) {
        QFile pictureFile(p_filePath);
        if (pictureFile.open(QIODevice::ReadOnly)) {
            readData = pictureFile.readAll();
        }
        pictureFile.close();
    }
    else if (p_inputMode == 1) {
        if (!p_ioDevice->isOpen()) {
            if (!p_ioDevice->open(QIODevice::ReadOnly))
                return false;
            readData = p_ioDevice->readAll();
        }
    }
    else {
        return false;
    }

    QBuffer dataBuffer(&readData);
    dataBuffer.open(QIODevice::ReadOnly);

    char formatHeader[4];
    qint64 size = dataBuffer.read(formatHeader, 4);
    if (size != 4)
        return false;
    quint32 format = charToUInt32(formatHeader);
    if (format != PhotoFormat::GTA5)
        return false;

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
    p_headerCRC = charToUInt32(checksum);

    char endOfFile[4];
    size = dataBuffer.read(endOfFile, 4);
    if (size != 4)
        return false;
    p_endOfFile = charToUInt32(endOfFile);

    char jsonOffset[4];
    size = dataBuffer.read(jsonOffset, 4);
    if (size != 4)
        return false;
    p_jsonOffset = charToUInt32(jsonOffset);

    char titleOffset[4];
    size = dataBuffer.read(titleOffset, 4);
    if (size != 4)
        return false;
    p_titlOffset = charToUInt32(titleOffset);

    char descOffset[4];
    size = dataBuffer.read(descOffset, 4);
    if (size != 4)
        return false;
    p_descOffset = charToUInt32(descOffset);

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
    p_jpegBuffer = charToUInt32(jpegBuffer);

    char photoSize[4];
    size = dataBuffer.read(photoSize, 4);
    if (size != 4)
        return false;
    p_photoSize = charToUInt32(photoSize);

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
    quint32 i_jsonSize = charToUInt32(jsonSize);

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
    quint32 i_titlSize = charToUInt32(titlSize);

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
    quint32 i_descSize = charToUInt32(descSize);

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

    p_isLoaded = true;
    return true;
}

void RagePhoto::clear()
{
    p_jsonObject = QJsonObject();
    p_descriptionString.clear();
    p_photoData.clear();
    p_photoString.clear();
    p_titleString.clear();
    p_headerCRC = 0;
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

quint32 RagePhoto::charToUInt32(char *x)
{
    return (((unsigned char)x[3] << 24) | ((unsigned char)x[2] << 16) | ((unsigned char)x[1] << 8) | ((unsigned char)x[0]));
}
