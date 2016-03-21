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

#ifndef SNAPMATICPICTURE_H
#define SNAPMATICPICTURE_H

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

private:
    QString getSnapmaticPictureString(QByteArray snapmaticHeader);
    QString convertDrawStringForLog(QString inputStr);
    QString convertLogStringForDraw(QString inputStr);
    QPixmap cachePicture;
    QString picFileName;
    QString pictureStr;
    QString lastStep;
    QString jsonStr;

signals:

public slots:
};

#endif // SNAPMATICPICTURE_H
