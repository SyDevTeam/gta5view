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

#ifndef CONFIG_H
#define CONFIG_H

#ifndef GTA5SYNC_APPVENDOR
#define GTA5SYNC_APPVENDOR "Syping"
#endif

#ifndef GTA5SYNC_APPSTR
#define GTA5SYNC_APPSTR "gta5sync"
#endif

#ifndef GTA5SYNC_APPVER
#define GTA5SYNC_APPVER "1.0.0"
#endif

#ifndef GTA5SYNC_BUILDTYPE
#define GTA5SYNC_BUILDTYPE "Custom"
#endif

#ifndef GTA5SYNC_SHARE
#define GTA5SYNC_SHARE "$RUNDIR"
#endif

#ifndef GTA5SYNC_LANG
#define GTA5SYNC_LANG "$SHAREDIR$SEPARATORlang"
#endif

#ifndef GTA5SYNC_PLUG
#define GTA5SYNC_PLUG "$RUNDIR$SEPARATORplugins"
#endif

#ifdef GTA5SYNC_WINRT
#undef GTA5SYNC_WIN
#endif

#endif // CONFIG_H
