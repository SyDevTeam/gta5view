/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping
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

#ifndef STANDARDPATHS_H
#define STANDARDPATHS_H

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#include <QObject>

class StandardPaths : public QStandardPaths
{
public:
    StandardPaths();
};
#else
#include <QDesktopServices>
#include <QObject>

class StandardPaths : public QDesktopServices
{
public:
    StandardPaths();
    static QString writableLocation(QDesktopServices::StandardLocation standardLocation);
};
#endif

#endif // STANDARDPATHS_H
