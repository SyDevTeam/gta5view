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

#include "TelemetryClassAuthenticator.h"
#include <QUrlQuery>
#include <QUrl>

#ifndef GTA5SYNC_TELEMETRY_PUSHURL
#define GTA5SYNC_TELEMETRY_PUSHURL ""
#endif

#ifndef GTA5SYNC_TELEMETRY_REGURL
#define GTA5SYNC_TELEMETRY_REGURL ""
#endif

#ifndef GTA5SYNC_TELEMETRY_AUTHID
#define GTA5SYNC_TELEMETRY_AUTHID ""
#endif

#ifndef GTA5SYNC_TELEMETRY_AUTHPW
#define GTA5SYNC_TELEMETRY_AUTHPW ""
#endif

const QUrl TelemetryClassAuthenticator::getTrackingPushURL()
{
    if (haveAccessData())
    {
        QUrl pushUrl(GTA5SYNC_TELEMETRY_PUSHURL);
        QUrlQuery pushQuery(pushUrl);
        if (!getTrackingAuthID().isEmpty()) { pushQuery.addQueryItem("tid", getTrackingAuthID()); }
        if (!getTrackingAuthPW().isEmpty()) { pushQuery.addQueryItem("tpw", getTrackingAuthPW()); }
        pushUrl.setQuery(pushQuery.query(QUrl::FullyEncoded));
        return pushUrl;
    }
    else
    {
        QUrl pushUrl(GTA5SYNC_TELEMETRY_PUSHURL);
        return pushUrl;
    }
}

const QUrl TelemetryClassAuthenticator::getTrackingRegURL()
{
    if (haveAccessData())
    {
        QUrl regUrl(GTA5SYNC_TELEMETRY_REGURL);
        QUrlQuery regQuery(regUrl);
        if (!getTrackingAuthID().isEmpty()) { regQuery.addQueryItem("tid", getTrackingAuthID()); }
        if (!getTrackingAuthPW().isEmpty()) { regQuery.addQueryItem("tpw", getTrackingAuthPW()); }
        regUrl.setQuery(regQuery.query(QUrl::FullyEncoded));
        return regUrl;
    }
    else
    {
        QUrl regUrl(GTA5SYNC_TELEMETRY_REGURL);
        return regUrl;
    }
}

const QString TelemetryClassAuthenticator::getTrackingAuthID()
{
    return QString(GTA5SYNC_TELEMETRY_AUTHID);
}

const QString TelemetryClassAuthenticator::getTrackingAuthPW()
{
    return QString(GTA5SYNC_TELEMETRY_AUTHPW);
}

bool TelemetryClassAuthenticator::havePushURL()
{
    return !getTrackingPushURL().isEmpty();
}

bool TelemetryClassAuthenticator::haveRegURL()
{
    return !getTrackingRegURL().isEmpty();
}

bool TelemetryClassAuthenticator::haveAccessData()
{
    if (getTrackingAuthID().isEmpty() && getTrackingAuthPW().isEmpty()) { return false; }
    return true;
}
