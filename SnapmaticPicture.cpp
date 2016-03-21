/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping Gaming Team
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/

#include "SnapmaticPicture.h"
#include <QPixmap>
#include <QString>
#include <QDebug>
#include <QFile>
int snapmaticHeaderLength = 278;
int snapmaticUsefulLength = 256;
int jpegPreHeaderLength = 14;
int jpegPicStreamLength = 524288;

SnapmaticPicture::SnapmaticPicture(QObject *parent, QString fileName) : QObject(parent)
{
    // Init
    cachePicture = QPixmap(0,0);
    picFileName = "";
    pictureStr = "";
    lastStep = "";
    jsonStr = "";

    // Set pic fileName
    if (fileName != "")
    {
        picFileName = fileName;
    }
}

bool SnapmaticPicture::readingPicture()
{
    // Start opening file
    // lastStep is like currentStep

    QFile *picFile = new QFile(picFileName);
    if (!picFile->open(QFile::ReadOnly))
    {
        lastStep = "1;/1,OpenFile," + convertDrawStringForLog(picFileName);
        return false;
    }

    // Reading Snapmatic Header
    if (!picFile->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + convertDrawStringForLog(picFileName) + ",1,NOHEADER";
        return false;
    }
    QByteArray snapmaticHeaderLine = picFile->read(snapmaticHeaderLength);
    pictureStr = getSnapmaticPictureString(snapmaticHeaderLine);

    // Reading JPEG Header Line
    if (!picFile->isReadable())
    {
        lastStep = "2;/3,ReadingFile," + convertDrawStringForLog(picFileName) + ",2,NOHEADER";
        return false;
    }
    QByteArray jpegHeaderLine = picFile->read(jpegPreHeaderLength);

    // Checking for JPEG
    jpegHeaderLine.remove(0,2);
    if (jpegHeaderLine.left(4) != "JPEG")
    {
        lastStep = "2;/3,ReadingFile," + convertDrawStringForLog(picFileName) + ",2,NOJPEG";
        return false;
    }

    lastStep = "3;/1,ReadedFile," + convertDrawStringForLog(picFileName);
    QByteArray jpegRawContent = picFile->read(jpegPicStreamLength);
    return cachePicture.loadFromData(jpegRawContent);
}

QString SnapmaticPicture::getSnapmaticPictureString(QByteArray snapmaticHeader)
{
    QByteArray snapmaticUsefulBytes = snapmaticHeader.left(snapmaticUsefulLength);
    snapmaticUsefulBytes.replace(QByteArray::fromHex("00"),"");
    snapmaticUsefulBytes.replace(QByteArray::fromHex("01"),"");
    return QString::fromAscii(snapmaticUsefulBytes);
}

bool SnapmaticPicture::readingPictureFromFile(QString fileName)
{
    if (fileName != "")
    {
        picFileName = fileName;
        return readingPicture();
    }
    else
    {
        return false;
    }
}

void SnapmaticPicture::setPixmap(QPixmap pixmap)
{
    cachePicture = pixmap;
}

QString SnapmaticPicture::getPictureStr()
{
    return pictureStr;
}

QString SnapmaticPicture::getLastStep()
{
    return lastStep;
}

QPixmap SnapmaticPicture::getPixmap()
{
    return cachePicture;
}

QString SnapmaticPicture::convertDrawStringForLog(QString inputStr)
{
    return inputStr.replace("&","&u;").replace(",","&c;");
}

QString SnapmaticPicture::convertLogStringForDraw(QString inputStr)
{
    return inputStr.replace("&c;",",").replace("&u;","&");
}
