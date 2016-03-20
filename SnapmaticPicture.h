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

class SnapmaticPicture : public QObject
{
    Q_OBJECT
public:
    explicit SnapmaticPicture(QObject *parent = 0);
    bool readingPictureFromFile(QString fileName);
    bool setPicture(QPixmap pixmap);
    QPixmap getPixmap();

private:
    QPixmap cachePicture;
    QString picDate;
    QString picTime;
    QString jsonStr;

signals:

public slots:
};

#endif // SNAPMATICPICTURE_H
