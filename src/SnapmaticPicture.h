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

#ifndef SNAPMATICPICTURE_H
#define SNAPMATICPICTURE_H

#ifdef RAGEPHOTO_USE_ABI_WRAPPER
#include <RagePhotoA.h>
typedef RagePhotoA RagePhoto;
#else
#include <RagePhoto.h>
#endif
#include <QStringList>
#include <QJsonObject>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QImage>
#include <QFile>

enum class SnapmaticFormat : int { Auto_Format = 0, PGTA_Format = 1, JPEG_Format = 2, G5E_Format = 3 };
enum G5EExportFormat : uint32_t {
    G5E1P = 0x454C0010UL,
    G5E2P = 0x01000032UL,
    G5E2S = 0x02000032UL,
    G5E3P = 0x01000033UL,
    G5E3S = 0x02000033UL,
};
enum G5EPhotoFormat : uint32_t {
    G5EX = 0x45354700UL,
};

struct SnapmaticProperties {
    struct SnapmaticLocation {
        QString area;
        double x;
        double y;
        double z;
        bool isCayoPerico;
    };
    int uid;
    int crewID;
    int streetID;
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
    bool readingPictureFromFile(const QString &fileName, bool cacheEnabled = false);
    bool readingPicture(bool cacheEnabled = false);
    bool isPicOk(); // Please use isPictureOk instead
    void clearCache();
    QImage getImage();
    QByteArray getPictureStream();
    QString getLastStep(bool readable = true);
    QString getPictureStr();
    QString getPictureTitl();
    QString getPictureSortStr();
    QString getPictureFileName();
    QString getPictureFilePath();
    QString getExportPictureFileName();
    QString getOriginalPictureFileName();
    QString getOriginalPictureFilePath();
    bool setImage(const QImage &picture, bool eXtendMode = false);
    bool setPictureTitl(const QString &newTitle); // Please use setPictureTitle instead
    bool setPictureStream(const QByteArray &streamArray);
    void updateStrings();
    void emitUpdate();
    void emitCustomSignal(const QString &signal);

    // FILE MANAGEMENT
    bool exportPicture(const QString &fileName, SnapmaticFormat format = SnapmaticFormat::Auto_Format);
    void setPicFileName(const QString &picFileName); // Please use setPictureFileName instead
    void setPicFilePath(const QString &picFilePath); // Please use setPictureFilePath instead
    bool deletePicFile(); // Please use deletePictureFile instead

    // JSON
    bool isJsonOk();
    QString getJsonStr(); // Please use getPictureJson instead
    SnapmaticProperties getSnapmaticProperties();
    bool setSnapmaticProperties(SnapmaticProperties properties);
    bool setJsonStr(const QString &jsonStr, bool updateProperties = false); // Please use setPictureJson instead

    // VISIBILITY
    bool isHidden(); // Please use isPictureHidden instead
    bool isVisible(); // Please use isPictureVisible instead
    bool setPictureHidden();
    bool setPictureVisible();

    // ALTERNATIVES (MORE DEVELOPER FRIENDLY FUNCTION CALLS)
    QString getJsonString() { return getJsonStr(); } // Please use getPictureJson instead
    QString getPictureJson() { return getJsonStr(); }
    QString getPictureTitle() { return getPictureTitl(); }
    QString getPictureString() { return getPictureStr(); }
    bool setJsonString(const QString &jsonString, bool updateProperties = false) { return setJsonStr(jsonString, updateProperties); } // Please use setPictureJson instead
    bool setPictureJson(const QString &json, bool updateProperties = false) { return setJsonStr(json, updateProperties); }
    bool setPictureTitle(const QString &title) { return setPictureTitl(title); }
    void setPictureFileName(const QString &fileName) { return setPicFileName(fileName); }
    void setPictureFilePath(const QString &filePath) { return setPicFilePath(filePath); }
    bool deletePictureFile() { return deletePicFile(); }
    bool isPictureOk() { return isPicOk(); }
    bool isPictureHidden() { return isHidden(); }
    bool isPictureVisible() { return isVisible(); }
    bool setHidden() { return setPictureHidden(); } // Please use setPictureHidden instead
    bool setVisible() { return setPictureVisible(); } // Please use setPictureVisible instead

    // PREDEFINED PROPERTIES
    static QSize getSnapmaticResolution();

    // SNAPMATIC FORMAT
    SnapmaticFormat getSnapmaticFormat();
    void setSnapmaticFormat(SnapmaticFormat format);
    bool isFormatSwitched();

    // VERIFY CONTENT
    static bool verifyTitle(const QString &title);

    // STRING OPERATIONS
    static QString parseTitleString(const QByteArray &commitBytes);
    static QString convertDrawStringForLog(const QString &inputStr);
    static QString convertLogStringForDraw(const QString &inputStr);

    // RAGEPHOTO
    RagePhoto* ragePhoto();

private:
    QImage cachePicture;
    QString picExportFileName;
    QString picFileName;
    QString picFilePath;
    QString pictureStr;
    QString lastStep;
    QString sortStr;
    uint32_t picFormat;
    bool picOk;
    bool cacheEnabled;
    bool isFormatSwitch;
    bool isPreLoaded;

    // JSON
    void parseJsonContent();
    bool jsonOk;
    SnapmaticProperties localProperties;
    QJsonObject jsonObject;

    // VERIFY CONTENT
    static bool verifyTitleChar(const QChar &titleChar);

    // RAGEPHOTO
    RagePhoto p_ragePhoto;

signals:
    void customSignal(QString signal);
    void preloaded();
    void updated();
    void loaded();

public slots:
};

#endif // SNAPMATICPICTURE_H
