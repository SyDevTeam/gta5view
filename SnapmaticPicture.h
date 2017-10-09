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

#ifndef SNAPMATICPICTURE_H
#define SNAPMATICPICTURE_H

#include <QStringList>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QImage>
#include <QFile>

enum class SnapmaticFormat : int { Auto_Format = 0, PGTA_Format = 1, JPEG_Format = 2, G5E_Format = 3 };

struct SnapmaticProperties {
    struct SnapmaticLocation {
        QString area;
        double x;
        double y;
        double z;
    };
    int uid;
    int crewID;
    QStringList playersList;
    uint createdTimestamp;
    QDateTime createdDateTime;
    bool isMeme;
    bool isMug;
    bool isSelfie;
    bool isFromDirector;
    bool isFromRSEditor;
    SnapmaticLocation location;
};

class SnapmaticPicture : public QObject
{
    Q_OBJECT
public:
    explicit SnapmaticPicture(const QString &fileName = "", QObject *parent = 0);
    ~SnapmaticPicture();
    void reset();
    bool preloadFile();
    bool readingPictureFromFile(const QString &fileName, bool writeEnabled = true, bool cacheEnabled = false, bool fastLoad = true, bool lowRamMode = false);
    bool readingPicture(bool writeEnabled = true, bool cacheEnabled = false, bool fastLoad = true, bool lowRamMode = false);
    bool isPicOk();
    void clearCache();
    QImage getImage(bool fastLoad = false);
    QString getLastStep();
    QString getPictureStr();
    QString getPictureHead();
    QString getPictureTitl();
    QString getPictureDesc();
    QString getPictureSortStr();
    QString getPictureFileName();
    QString getPictureFilePath();
    QString getExportPictureFileName();
    int getContentMaxLength();
    bool setImage(const QImage &picture);
    bool setPictureTitl(const QString &newTitle);
    bool setPictureStream(const QByteArray &picByteArray);
    void updateStrings();
    void emitUpdate();

    // FILE MANAGEMENT
    bool exportPicture(const QString &fileName, SnapmaticFormat format = SnapmaticFormat::Auto_Format);
    void setPicFileName(const QString &picFileName);
    void setPicFilePath(const QString &picFilePath);
    bool deletePicFile();

    // ALTERNATIVES
    QString getPictureTitle() { return getPictureTitl(); }
    QString getPictureString() { return getPictureStr(); }
    QString getPictureDescription() { return getPictureDesc(); }
    bool setPictureTitle(const QString &newTitle) { return setPictureTitl(newTitle); }

    // JSON
    bool isJsonOk();
    QString getJsonStr();
    SnapmaticProperties getSnapmaticProperties();
    bool setSnapmaticProperties(SnapmaticProperties newSpJson);
    bool setJsonStr(const QString &jsonStr);

    // VISIBILITY
    bool isHidden();
    bool setPictureHidden();
    bool setPictureVisible();

    // PREDEFINED PROPERTIES
    QSize getSnapmaticResolution();

    // SNAPMATIC DEFAULTS
    bool isSnapmaticDefaultsEnforced();
    void setSnapmaticDefaultsEnforced(bool enforced);

    // VERIFY CONTENT
    static bool verifyTitle(const QString &title);

private:
    QString getSnapmaticHeaderString(const QByteArray &snapmaticHeader);
    QString getSnapmaticJSONString(const QByteArray &jsonBytes);
    QString getSnapmaticTIDEString(const QByteArray &tideBytes);
    QImage cachePicture;
    QString picExportFileName;
    QString picFileName;
    QString picFilePath;
    QString pictureHead;
    QString pictureStr;
    QString lastStep;
    QString sortStr;
    QString titlStr;
    QString descStr;
    bool picOk;
    bool lowRamMode;
    bool writeEnabled;
    bool cacheEnabled;
    bool isLoadedInRAM;
    bool isCustomFormat;
    bool careSnapDefault;
    int jpegRawContentSize;
    int jpegRawContentSizeE;

    // PICTURE STREAM
    QByteArray rawPicContent;

    // JSON
    void parseJsonContent();
    bool jsonOk;
    QString jsonStr;
    SnapmaticProperties localSpJson;

    // VERIFY CONTENT
    static bool verifyTitleChar(const QChar &titleChar);

signals:
    void preloaded();
    void updated();
    void loaded();

public slots:
};

#endif // SNAPMATICPICTURE_H
