/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping Gaming Team
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
#include <QPixmap>
#include <QString>
#include <QFile>

class SnapmaticPicture : public QObject
{
    Q_OBJECT
public:
    explicit SnapmaticPicture(QObject *parent = 0, QString fileName = "");
    bool readingPictureFromFile(QString fileName);
    bool readingPicture();
    void setPixmap(QPixmap pixmap);
    void resetValues();
    QPixmap getPixmap();
    QString getLastStep();
    QString getPictureStr();

    // JSON
    QString getJsonStr();
    double getLocationX();
    double getLocationY();
    double getLocationZ();
    QStringList getPlayers();

private:
    QString getSnapmaticPictureString(QByteArray snapmaticHeader);
    QString getSnapmaticJSONString(QByteArray jsonBytes);
    QString convertDrawStringForLog(QString inputStr);
    QString convertLogStringForDraw(QString inputStr);
    QPixmap cachePicture;
    QString picFileName;
    QString pictureStr;
    QString lastStep;

    // JSON
    void parseJsonContent();
    QString jsonStr;
    double jsonLocX;
    double jsonLocY;
    double jsonLocZ;
    QStringList jsonPlyrsList;

signals:

public slots:
};

#endif // SNAPMATICPICTURE_H
