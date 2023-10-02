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

#include "SnapmaticJson.h"
#ifdef RAGEPHOTO_USE_ABI_WRAPPER
#include <RagePhotoA>
typedef RagePhotoA RagePhoto;
#else
#include <RagePhoto>
#endif
#include <QStringList>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QImage>

enum class SnapmaticFormat : int32_t { Auto_Format = 0, PGTA5_Format = 1, PRDR3_Format = 2, JPEG_Format = 3, G5E_Format = 4, Unknown_Format = -1 };
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
    uint64_t uid;
    uint64_t crewID;
    uint64_t streetID;
    QStringList playersList;
    int64_t createdTimestamp;
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
    const QImage getImage();
    const QByteArray getPictureStream();
    const QSize getPictureResolution();
    const QString getLastStep(bool readable = true);
    const QString getPictureStr();
    const QString getPictureTitl();
    const QString getPictureSortStr();
    const QString getPictureFileName();
    const QString getPictureFilePath();
    const QString getExportPictureFileName();
    const QString getOriginalPictureFileName();
    const QString getOriginalPictureFilePath();
    void initialise(uint32_t photoFormat);
    bool setImage(const QImage &picture, bool eXtendMode = false);
    bool setPictureTitl(const QString &newTitle); // Please use setPictureTitle instead
    bool setPictureStream(const QByteArray &streamArray, int width, int height);
    void updateStrings();
    void emitUpdate();
    void emitCustomSignal(const QString &signal);

    // FILE MANAGEMENT
    bool exportPicture(const QString &fileName, SnapmaticFormat format = SnapmaticFormat::Auto_Format);
    void setPicFileName(const QString &picFileName);
    void setPicFilePath(const QString &picFilePath);
    bool deletePicFile();

    // JSON
    bool isJsonOk();
    const QString getJsonStr();
    const std::string getJsonStdStr();
    SnapmaticProperties getSnapmaticProperties();
    bool setSnapmaticProperties(SnapmaticProperties properties);
    bool setJsonStr(const std::string &json, bool updateProperties = false);
    bool setJsonStr(const QString &json, bool updateProperties = false);

    // VISIBILITY
    bool isHidden();
    bool isVisible();
    bool setPictureHidden();
    bool setPictureVisible();

    // ALTERNATIVES (MORE DEVELOPER FRIENDLY FUNCTION CALLS)
    inline QString getPictureJson() { return getJsonStr(); }
    inline QString getPictureTitle() { return getPictureTitl(); }
    inline uint32_t getPictureSize() { return ragePhoto()->jpegSize(); }
    inline QString getPictureString() { return getPictureStr(); }
    inline bool setPictureJson(const std::string &json, bool updateProperties = false) { return setJsonStr(json, updateProperties); }
    inline bool setPictureJson(const QString &json, bool updateProperties = false) { return setJsonStr(json, updateProperties); }
    inline bool setPictureTitle(const QString &title) { return setPictureTitl(title); }
    inline void setPictureFileName(const QString &fileName) { return setPicFileName(fileName); }
    inline void setPictureFilePath(const QString &filePath) { return setPicFilePath(filePath); }
    inline bool deletePictureFile() { return deletePicFile(); }
    inline bool isPictureOk() { return isPicOk(); }
    inline bool isPictureHidden() { return isHidden(); }
    inline bool isPictureVisible() { return isVisible(); }
    inline bool setHidden() { return setPictureHidden(); }
    inline bool setVisible() { return setPictureVisible(); }

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
    QSize picRes;
    bool picOk;
    bool cacheEnabled;
    bool isFormatSwitch;
    bool isPreLoaded;

    // JSON
    void parseJsonContent();
    bool jsonOk;
    SnapmaticProperties localProperties;
    SnapmaticJson snapmaticJson;

    // VERIFY CONTENT
    static bool verifyTitleChar(const QChar &titleChar);

    // RAGEPHOTO
    RagePhoto p_ragePhoto;

signals:
    void customSignal(QString signal);
    void preloaded();
    void updated();
    void loaded();
};

#endif // SNAPMATICPICTURE_H
