/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016-2018 Syping
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
#include <QString>

#ifndef GTA5SYNC_APPVENDOR
#define GTA5SYNC_APPVENDOR "Syping"
#endif

#ifndef GTA5SYNC_APPVENDORLINK
#define GTA5SYNC_APPVENDORLINK "https://github.com/Syping/"
#endif

#ifndef GTA5SYNC_DISABLED
#define GTA5SYNC_ENABLED
#endif

#ifndef GTA5SYNC_APPSTR
#ifdef GTA5SYNC_ENABLED
#define GTA5SYNC_APPSTR "gta5sync"
#else
#define GTA5SYNC_APPSTR "gta5view"
#endif
#endif

#ifndef GTA5SYNC_APPDES
#define GTA5SYNC_APPDES "INSERT YOUR APPLICATION DESCRIPTION HERE"
#endif

#ifndef GTA5SYNC_COPYRIGHT
#define GTA5SYNC_COPYRIGHT "2016-2018"
#endif

#ifndef GTA5SYNC_APPVER
#define GTA5SYNC_APPVER "1.5.0-rc1"
#endif

#ifdef GTA5SYNC_BUILDTYPE_REL
#ifndef GTA5SYNC_BUILDTYPE
#define GTA5SYNC_BUILDTYPE QT_TRANSLATE_NOOP("AboutDialog", "Release")
#endif
#endif

#ifdef GTA5SYNC_BUILDTYPE_RC
#ifndef GTA5SYNC_BUILDTYPE
#define GTA5SYNC_BUILDTYPE QT_TRANSLATE_NOOP("AboutDialog", "Release Candidate")
#endif
#endif

#ifdef GTA5SYNC_BUILDTYPE_DAILY
#ifndef GTA5SYNC_BUILDTYPE
#define GTA5SYNC_BUILDTYPE QT_TRANSLATE_NOOP("AboutDialog", "Daily Build")
#endif
#endif

#ifdef GTA5SYNC_BUILDTYPE_DEV
#ifndef GTA5SYNC_BUILDTYPE
#define GTA5SYNC_BUILDTYPE QT_TRANSLATE_NOOP("AboutDialog", "Developer")
#endif
#endif

#ifdef GTA5SYNC_BUILDTYPE_BETA
#ifndef GTA5SYNC_BUILDTYPE
#define GTA5SYNC_BUILDTYPE QT_TRANSLATE_NOOP("AboutDialog", "Beta")
#endif
#endif

#ifdef GTA5SYNC_BUILDTYPE_ALPHA
#ifndef GTA5SYNC_BUILDTYPE
#define GTA5SYNC_BUILDTYPE QT_TRANSLATE_NOOP("AboutDialog", "Alpha")
#endif
#endif

#ifndef GTA5SYNC_BUILDTYPE
#define GTA5SYNC_BUILDTYPE QT_TRANSLATE_NOOP("AboutDialog", "Custom")
#endif

#ifdef GTA5SYNC_QCONF
#ifndef GTA5SYNC_SHARE
#define GTA5SYNC_SHARE "RUNDIR:SEPARATOR:..SEPARATOR:share"
#endif
#ifndef GTA5SYNC_LANG
#define GTA5SYNC_LANG "QCONFLANG:"
#endif
#ifndef GTA5SYNC_PLUG
#define GTA5SYNC_PLUG "QCONFPLUG:"
#endif
#ifdef GTA5SYNC_QCONF_IN
#ifndef GTA5SYNC_INLANG
#define GTA5SYNC_INLANG ":/tr"
#endif
#endif
#endif

#ifndef GTA5SYNC_SHARE
#define GTA5SYNC_SHARE "RUNDIR:"
#endif

#ifndef GTA5SYNC_LANG
#define GTA5SYNC_LANG "SHAREDDIR:SEPARATOR:lang"
#endif

#ifndef GTA5SYNC_PLUG
#define GTA5SYNC_PLUG "RUNDIR:SEPARATOR:plugins"
#endif

#ifdef GTA5SYNC_WINRT
#undef GTA5SYNC_WIN
#endif

#ifndef GTA5SYNC_COMPILER
#ifdef __clang__
#define GTA5SYNC_COMPILER QString("Clang %1.%2.%3").arg(QString::number(__clang_major__), QString::number(__clang_minor__), QString::number(__clang_patchlevel__))
#elif defined(__GNUC__)
#define GTA5SYNC_COMPILER QString("GCC %1.%2.%3").arg(QString::number(__GNUC__), QString::number(__GNUC_MINOR__), QString::number(__GNUC_PATCHLEVEL__))
#elif defined(__GNUG__)
#define GTA5SYNC_COMPILER QString("GCC %1.%2.%3").arg(QString::number(__GNUG__), QString::number(__GNUC_MINOR__), QString::number(__GNUC_PATCHLEVEL__))
#elif defined(_MSC_VER)
#define GTA5SYNC_COMPILER QString("MSVC %1").arg(QString::number(_MSC_VER).insert(2, "."))
#else
#define GTA5SYNC_COMPILER QString("Unknown Compiler")
#endif
#endif

#ifndef GTA5SYNC_BUILDDATETIME
#define GTA5SYNC_BUILDDATETIME QString("%1, %2").arg(__DATE__, __TIME__)
#endif

#ifndef GTA5SYNC_BUILDSTRING
#define GTA5SYNC_BUILDSTRING QString("%1, %2").arg(QT_VERSION_STR, GTA5SYNC_COMPILER)
#endif

#endif // CONFIG_H
