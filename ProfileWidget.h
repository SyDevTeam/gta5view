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

#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>

class ProfileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileWidget(QWidget *parent = 0);
    virtual void setSelectionMode(bool selectionMode);
    virtual void setContentMode(int contentMode);
    virtual void setSelected(bool isSelected);
    virtual bool isSelected();
    virtual QString getWidgetType();
    virtual int getContentMode();
    virtual void retranslate();
    ~ProfileWidget();

private:
    int contentMode;

signals:

public slots:
};

#endif // PROFILEWIDGET_H
