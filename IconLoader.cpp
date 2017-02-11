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

#include "IconLoader.h"
#include <QIcon>

IconLoader::IconLoader()
{

}

QIcon IconLoader::loadingAppIcon()
{
    QIcon appIcon;
    appIcon.addFile(":/img/5sync-16.png", QSize(16, 16));
    appIcon.addFile(":/img/5sync-24.png", QSize(24, 24));
    appIcon.addFile(":/img/5sync-32.png", QSize(32, 32));
    appIcon.addFile(":/img/5sync-40.png", QSize(40, 40));
    appIcon.addFile(":/img/5sync-48.png", QSize(48, 48));
    appIcon.addFile(":/img/5sync-64.png", QSize(64, 64));
    appIcon.addFile(":/img/5sync-96.png", QSize(96, 96));
    appIcon.addFile(":/img/5sync-128.png", QSize(128, 128));
    appIcon.addFile(":/img/5sync-256.png", QSize(256, 256));
    return appIcon;
}
