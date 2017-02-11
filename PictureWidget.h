/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
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

#ifndef PICTUREWIDGET_H
#define PICTUREWIDGET_H

#include "UiModLabel.h"
#include <QHBoxLayout>
#include <QDialog>
#include <QWidget>
#include <QEvent>

class PictureWidget : public QDialog
{
    Q_OBJECT
public:
    explicit PictureWidget(QWidget *parent = 0);
    void setImage(QImage image, QRect rec);
    ~PictureWidget();

public slots:
    void setImage(QImage image);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private:
    QHBoxLayout *widgetLayout;
    UiModLabel *pictureLabel;
    QImage image;

private slots:
    void pictureDoubleClicked(Qt::MouseButton button);
    void updateWindowSize(int screenID);

signals:
    void nextPictureRequested();
    void previousPictureRequested();
};

#endif // PICTUREWIDGET_H
