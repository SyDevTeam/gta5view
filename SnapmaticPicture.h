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

#ifndef SNAPMATICPICTURE_H
#define SNAPMATICPICTURE_H

#include <QStringList>
#include <QObject>
#include <QString>
#include <QImage>
#include <QFile>

class SnapmaticPicture : public QObject
{
    Q_OBJECT
public:
    explicit SnapmaticPicture(QString fileName = "", QObject *parent = 0);
    bool readingPictureFromFile(QString fileName);
    bool readingPicture();
    void setPicture(QImage picture);
    bool isPicOk();
    QImage getPicture();
    QString getLastStep();
    QString getPictureStr();
    QString getPictureTitl();
    QString getPictureDesc();

    // JSON
    bool isJsonOk();
    int getCrewNumber();
    QString getJsonStr();
    double getLocationX();
    double getLocationY();
    double getLocationZ();
    QStringList getPlayers();

private:
    QString getSnapmaticPictureString(QByteArray snapmaticHeader);
    QString getSnapmaticJSONString(QByteArray jsonBytes);
    QString getSnapmaticTIDEString(QByteArray tideBytes);
    QString convertDrawStringForLog(QString inputStr);
    QString convertLogStringForDraw(QString inputStr);
    QImage cachePicture;
    QString picFileName;
    QString pictureStr;
    QString lastStep;
    QString titlStr;
    QString descStr;
    bool picOk;

    // PARSE INT
    int snapmaticHeaderLength;
    int snapmaticUsefulLength;
    int jpegHeaderLineDifStr;
    int jpegPreHeaderLength;
    int jpegPicStreamLength;
    int jsonStreamLength;
    int tideStreamLength;

    // JSON
    void parseJsonContent();
    bool jsonOk;
    int jsonCrewID;
    QString jsonStr;
    double jsonLocX;
    double jsonLocY;
    double jsonLocZ;
    QStringList jsonPlyrsList;

signals:

public slots:
};

#endif // SNAPMATICPICTURE_H
