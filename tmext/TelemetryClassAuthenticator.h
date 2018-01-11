/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2018 Syping
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

#ifndef TELEMETRYCLASSAUTHENTICATOR_H
#define TELEMETRYCLASSAUTHENTICATOR_H

#include <QApplication>
#include <QObject>
#include <QString>
#include <QUrl>

class TelemetryClassAuthenticator : public QObject
{
    Q_OBJECT
public:
    static const QUrl getTrackingPushURL();
    static const QUrl getTrackingRegURL();
    static const QString getTrackingAuthID();
    static const QString getTrackingAuthPW();
    static bool havePushURL();
    static bool haveRegURL();
    static bool haveAccessData();
};


#endif // TELEMETRYCLASSAUTHENTICATOR_H
