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
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QImage>
#include <QFile>

class SnapmaticPicture : public QObject
{
    Q_OBJECT
public:
    explicit SnapmaticPicture(const QString &fileName = "", QObject *parent = 0);
    ~SnapmaticPicture();
    bool readingPictureFromFile(const QString &fileName, bool writeEnabled = true, bool cacheEnabled = true);
    bool readingPicture(bool writeEnabled = true, bool cacheEnabled = true);
    bool isPicOk();
    void clearCache();
    QImage getPicture();
    QString getLastStep();
    QString getPictureStr();
    QString getPictureTitl();
    QString getPictureDesc();
    QString getPictureSortStr();
    QString getPictureFileName();
    QString getExportPictureFileName();
    QDateTime getCreatedDateTime();
    bool setPicture(const QImage &picture);
    bool exportPicture(const QString &fileName);
    void setPicFileName(QString picFileName_);

    // JSON
    bool isJsonOk();
    QString getArea();
    int getCrewNumber();
    QString getJsonStr();
    double getLocationX();
    double getLocationY();
    double getLocationZ();
    QStringList getPlayers();

    // VISIBILITY
    bool isHidden();
    bool setPictureHidden();
    bool setPictureVisible();

private:
    QString getSnapmaticPictureString(const QByteArray &snapmaticHeader);
    QString getSnapmaticJSONString(const QByteArray &jsonBytes);
    QString getSnapmaticTIDEString(const QByteArray &tideBytes);
    void parseSnapmaticExportAndSortString();
    QImage cachePicture;
    QString picExportFileName;
    QString picFileName;
    QString pictureStr;
    QString lastStep;
    QString sortStr;
    QString titlStr;
    QString descStr;
    bool picOk;
    bool writeEnabled;
    bool cacheEnabled;

    // PARSE INT
    int snapmaticHeaderLength;
    int snapmaticUsefulLength;
    int snapmaticFileMaxSize;
    int jpegHeaderLineDifStr;
    int jpegPreHeaderLength;
    int jpegPicStreamLength;
    int jsonStreamLength;
    int tideStreamLength;

    // PARSE EDITOR
    int jpegStreamEditorBegin;
    int jsonStreamEditorBegin;
    int jsonStreamEditorLength;
    QByteArray rawPicContent;

    // JSON
    void parseJsonContent();
    bool jsonOk;
    int jsonCrewID;
    QString jsonStr;
    double jsonLocX;
    double jsonLocY;
    double jsonLocZ;
    QString jsonArea;
    QStringList jsonPlyrsList;
    uint jsonCreatedTimestamp;
    QDateTime jsonCreatedDateTime;
    bool jsonMeme;
    bool jsonMug;
    bool jsonSelfie;
    bool jsonDirector;
    bool jsonRockstarEditor;

signals:

public slots:
};

#endif // SNAPMATICPICTURE_H
