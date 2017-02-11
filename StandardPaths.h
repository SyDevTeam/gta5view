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

#ifndef STANDARDPATHS_H
#define STANDARDPATHS_H

#include <QString>

class StandardPaths
{
public:
    StandardPaths();
    static QString applicationsLocation();
    static QString cacheLocation();
    static QString dataLocation();
    static QString desktopLocation();
    static QString documentsLocation();
    static QString fontsLocation();
    static QString homeLocation();
    static QString moviesLocation();
    static QString picturesLocation();
    static QString musicLocation();
    static QString tempLocation();
};

#endif // STANDARDPATHS_H
