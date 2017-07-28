/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2017 Syping
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

#ifndef UIMODWIDGET_H
#define UIMODWIDGET_H

#include <QMimeData>
#include <QWidget>
#include <QString>
#include <QSize>

class UiModWidget : public QWidget
{
    Q_OBJECT
public:
    UiModWidget(QWidget *parent = 0);
    void setFilesMode(bool enabled);
    ~UiModWidget();

protected:
    void dragEnterEvent(QDragEnterEvent *dragEnterEvent);
    void dropEvent(QDropEvent *dropEvent);
    void paintEvent(QPaintEvent *paintEvent);

private:
    bool filesMode;

signals:
    void dropped(const QMimeData *mimeData);
};

#endif // UIMODWIDGET_H
