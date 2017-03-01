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

#ifndef UIMODLABEL_H
#define UIMODLABEL_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QSize>

class UiModLabel : public QLabel
{
    Q_OBJECT
public:
    UiModLabel(const QString &text, QWidget *parent = 0);
    UiModLabel(QWidget *parent, const QString &text);
    UiModLabel(QWidget *parent = 0);
    ~UiModLabel();

protected:
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseDoubleClickEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void resizeEvent(QResizeEvent *ev);

signals:
    void mouseMoved();
    void mousePressed(Qt::MouseButton button);
    void mouseReleased(Qt::MouseButton button);
    void mouseDoubleClicked(Qt::MouseButton button);
    void labelPainted();
    void resized(QSize newSize);
};

#endif // UIMODLABEL_H
