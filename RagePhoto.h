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

#ifndef RAGEPHOTO_H
#define RAGEPHOTO_H

#include <QJsonObject>
#include <QIODevice>
#include <QObject>

class RagePhoto : public QObject
{
    Q_OBJECT
public:
    enum ExportFormat {
        G5E2P = 0x01000032U,
        G5E2S = 0x02000032U,
        G5E3P = 0x01000033U,
        G5E3S = 0x02000033U,
    };
    enum PhotoFormat {
        G5EX = 0x45354700U,
        GTA5 = 0x01000000U,
        RDR2 = 0x04000000U,
    };
    explicit RagePhoto(const QByteArray &data);
    explicit RagePhoto(const QString &filePath = QString());
    explicit RagePhoto(QIODevice *device);
    bool isLoaded();
    bool load();
    void clear();
    void setDescription(const QString &description);
    void setFilePath(const QString &filePath);
    void setPhotoData(const QByteArray &data);
    void setPhotoData(const char *data, int size);
    void setTitle(const QString &title);
    const QByteArray photoData();
    const QString description();
    const QString photoString();
    const QString title();
    static RagePhoto* loadFile(const QString &filePath);

private:
    inline quint32 charToUInt32BE(char *x);
    inline quint32 charToUInt32LE(char *x);
    inline void uInt32ToCharBE(quint32 *x, char *y);
    inline void uInt32ToCharLE(quint32 *x, char *y);
    QJsonObject p_jsonObject;
    QByteArray p_fileData;
    QByteArray p_photoData;
    QIODevice *p_ioDevice;
    QString p_descriptionString;
    QString p_filePath;
    QString p_photoString;
    QString p_titleString;
    quint32 p_descOffset;
    quint32 p_endOfFile;
    quint32 p_headerSum;
    quint32 p_jpegBuffer;
    quint32 p_jsonOffset;
    quint32 p_photoSize;
    quint32 p_titlOffset;
    bool p_isLoaded;
    int p_inputMode;
};

#endif // RAGEPHOTO_H
