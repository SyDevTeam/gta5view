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

#ifndef TELEMETRYCLASS_H
#define TELEMETRYCLASS_H

#include <QNetworkReply>
#include <QApplication>
#include <QObject>
#include <QString>

enum class TelemetryCategory : int { OperatingSystemSpec = 0, HardwareSpec = 1, UserLocaleData = 2, ApplicationConfiguration = 3, UserFeedback = 4, ApplicationSpec = 5, CustomEmitted = 99};

class TelemetryClass : public QObject
{
    Q_OBJECT
public:
    static TelemetryClass* getInstance() { return &telemetryClassInstance; }
    static QString categoryToString(TelemetryCategory category);
    bool canPush();
    bool canRegister();
    bool isEnabled();
    bool isStateForced();
    bool isRegistered();
    void init();
    void refresh();
    void setEnabled(bool enabled);
    void setDisabled(bool disabled);
    void push(TelemetryCategory category);
    void push(TelemetryCategory category, const QJsonDocument json);
    void registerClient();

private:
    static TelemetryClass telemetryClassInstance;
    QString telemetryClientID;
    bool telemetryEnabled;
    bool telemetryStateForced;

    QJsonDocument getOperatingSystem();
    QJsonDocument getSystemHardware();
    QJsonDocument getApplicationSpec();
    QJsonDocument getSystemLocaleList();

public slots:
    void pushStartupSet();

private slots:
    void pushFinished(QNetworkReply *reply);
    void registerFinished(QNetworkReply *reply);

signals:
    void pushed();
    void registered();
};

extern TelemetryClass telemetryClass;

#define Telemetry TelemetryClass::getInstance()

#endif // TELEMETRYCLASS_H
