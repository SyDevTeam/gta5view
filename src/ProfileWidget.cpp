/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
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

#include "ProfileWidget.h"
#include <QDebug>

ProfileWidget::ProfileWidget(QWidget *parent) : QWidget(parent)
{
    contentMode = 0;
}

ProfileWidget::~ProfileWidget()
{
}

void ProfileWidget::retranslate()
{
    qDebug() << "ProfileWidget::retranslate got used without overwrite";
}

bool ProfileWidget::isSelected()
{
    qDebug() << "ProfileWidget::isSelected got used without overwrite";
    return false;
}

void ProfileWidget::setSelected(bool isSelected)
{
    qDebug() << "ProfileWidget::setSelected got used without overwrite, result" << isSelected;
}

void ProfileWidget::setSelectionMode(bool selectionMode)
{
    qDebug() << "ProfileWidget::setSelectionMode got used without overwrite, result:" << selectionMode;
}

QString ProfileWidget::getWidgetType()
{
    qDebug() << "ProfileWidget::getWidgetType got used without overwrite";
    return "ProfileWidget";
}

int ProfileWidget::getContentMode()
{
    return contentMode;
}

void ProfileWidget::setContentMode(int _contentMode)
{
    contentMode = _contentMode;
}
