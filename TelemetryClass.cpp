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
#include "TelemetryClass.h"
#include "AppEnv.h"
#include "config.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QStringBuilder>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QSysInfo>
#include <QLocale>
#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QDir>

#ifdef GTA5SYNC_WIN
#include "windows.h"
#include "intrin.h"
#endif

TelemetryClass TelemetryClass::telemetryClassInstance;

void TelemetryClass::init()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Telemetry");
#ifndef GTA5SYNC_BUILDTYPE_DEV
    telemetryEnabled = settings.value("IsEnabled", false).toBool();
#else
    telemetryEnabled = true; // Always enable Telemetry for Developer Versions
#endif
    telemetryClientID = settings.value("ClientID", QString()).toString();
    settings.endGroup();
}

void TelemetryClass::refresh()
{
    init();
}

bool TelemetryClass::canPush()
{
    if (!isEnabled() || !isRegistered() || !TelemetryClassAuthenticator::havePushURL()) return false;
    return true;
}

bool TelemetryClass::canRegister()
{
    if (!isEnabled() || isRegistered() || !TelemetryClassAuthenticator::haveRegURL()) return false;
    return true;
}

bool TelemetryClass::isEnabled()
{
    return telemetryEnabled;
}

bool TelemetryClass::isStateForced()
{
    return telemetryStateForced;
}

bool TelemetryClass::isRegistered()
{
    return !telemetryClientID.isEmpty();
}

void TelemetryClass::setEnabled(bool enabled)
{
    telemetryEnabled = enabled;
    telemetryStateForced = true;
}

void TelemetryClass::setDisabled(bool disabled)
{
    telemetryEnabled = !disabled;
    telemetryStateForced = true;
}

void TelemetryClass::push(TelemetryCategory category)
{
    if (!canPush()) return;
    switch (category)
    {
    case TelemetryCategory::OperatingSystemSpec:
        push(category, getOperatingSystem());
        break;
    case TelemetryCategory::HardwareSpec:
        push(category, getSystemHardware());
        break;
    case TelemetryCategory::UserLocaleData:
        push(category, getSystemLocaleList());
        break;
    case TelemetryCategory::ApplicationConfiguration:
        break;
    case TelemetryCategory::ApplicationSpec:
        push(category, getApplicationSpec());
        break;
    case TelemetryCategory::UserFeedback:
        break;
    case TelemetryCategory::CustomEmitted:
        break;
    }
}

void TelemetryClass::push(TelemetryCategory category, QJsonDocument json)
{
    if (!canPush()) return;

    QJsonDocument jsonDocument(json);
    QJsonObject jsonObject = jsonDocument.object();
    jsonObject["ClientID"] = telemetryClientID;
    jsonDocument.setObject(jsonObject);

    QHttpMultiPart *httpMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart categoryPart;
    categoryPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"json-category\""));
    categoryPart.setBody(categoryToString(category).toUtf8());

    QHttpPart jsonPart;
    jsonPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    jsonPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"json-deflated\""));
    jsonPart.setBody(qCompress(jsonDocument.toJson(QJsonDocument::Compact)));

    httpMultiPart->append(categoryPart);
    httpMultiPart->append(jsonPart);

    QNetworkAccessManager *netManager = new QNetworkAccessManager();
    QNetworkRequest netRequest(TelemetryClassAuthenticator::getTrackingPushURL());
    QNetworkReply *netReply = netManager->post(netRequest, httpMultiPart);
    httpMultiPart->setParent(netReply);

    connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(pushFinished(QNetworkReply*)));
}

QJsonDocument TelemetryClass::getOperatingSystem()
{
    QJsonDocument jsonDocument;
    QJsonObject jsonObject;
#if QT_VERSION >= 0x050400
    jsonObject["OSName"] = QSysInfo::prettyProductName();
    jsonObject["OSArch"] = QSysInfo::currentCpuArchitecture();
#endif
    jsonDocument.setObject(jsonObject);
    return jsonDocument;
}

QJsonDocument TelemetryClass::getSystemHardware()
{
    QJsonDocument jsonDocument;
    QJsonObject jsonObject;
#ifdef GTA5SYNC_WIN
    {
        int CPUInfo[4] = {-1};
        unsigned nExIds, i = 0;
        char CPUBrandString[0x40];
        __cpuid(CPUInfo, 0x80000000);
        nExIds = CPUInfo[0];
        for (i = 0x80000000; i <= nExIds; ++i)
        {
            __cpuid(CPUInfo, i);
            if (i == 0x80000002) { memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo)); }
            else if (i == 0x80000003) { memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo)); }
            else if (i == 0x80000004) { memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo)); }
        }
        jsonObject["CPUName"] = QString(CPUBrandString).trimmed();
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        jsonObject["CPUThreads"] = QString::number(sysInfo.dwNumberOfProcessors);
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        GlobalMemoryStatusEx(&statex);
        jsonObject["SystemRAM"] = QString(QString::number((statex.ullTotalPhys / 1024) / 1024) % "MB");
    }
#else
    QDir procDir("/proc");
    if (procDir.exists())
    {
        QFile cpuInfo("/proc/cpuinfo");
        if (cpuInfo.open(QFile::ReadOnly))
        {
            QByteArray cpuInfoArray = cpuInfo.readAll();
            QBuffer cpuInfoBuffer(&cpuInfoArray);
            if (cpuInfoBuffer.open(QBuffer::ReadOnly))
            {
                QByteArray toFind = "model name";
                while (cpuInfoBuffer.canReadLine())
                {
                    QByteArray cpuData = cpuInfoBuffer.readLine();
                    if (cpuData.left(toFind.length()) == toFind)
                    {
                        jsonObject["CPUName"] = QString::fromUtf8(cpuData).split(':').at(1).trimmed();
                        break;
                    }
                }
                int cpuThreads = 0;
                toFind = "processor";
                cpuInfoBuffer.seek(0);
                while (cpuInfoBuffer.canReadLine())
                {
                    QByteArray cpuData = cpuInfoBuffer.readLine();
                    if (cpuData.left(toFind.length()) == toFind)
                    {
                        cpuThreads++;
                    }
                }
                jsonObject["CPUThreads"] = QString::number(cpuThreads);
            }
        }

        QFile memInfo("/proc/meminfo");
        if (memInfo.open(QFile::ReadOnly))
        {
            QByteArray memInfoArray = memInfo.readAll();
            QBuffer memInfoBuffer(&memInfoArray);
            if (memInfoBuffer.open(QBuffer::ReadOnly))
            {
                QByteArray toFind = "MemTotal:";
                while (memInfoBuffer.canReadLine())
                {
                    QByteArray memData = memInfoBuffer.readLine();
                    if (memData.left(toFind.length()) == toFind)
                    {
                        QByteArray memDataVal = memData.mid(toFind.length()).trimmed();
                        int totalMemoryInKB = memDataVal.left(memDataVal.length() - 3).toInt();
                        jsonObject["SystemRAM"] = QString(QString::number(totalMemoryInKB / 1024) % "MB");
                        break;
                    }
                }
            }
        }
    }
#endif

    jsonDocument.setObject(jsonObject);
    return jsonDocument;
}

QJsonDocument TelemetryClass::getApplicationSpec()
{
    QJsonDocument jsonDocument;
    QJsonObject jsonObject;
#if QT_VERSION >= 0x050400
    jsonObject["Arch"] = QSysInfo::buildCpuArchitecture();
#endif
    jsonObject["Name"] = GTA5SYNC_APPSTR;
    jsonObject["Version"] = GTA5SYNC_APPVER;
    jsonObject["BuildDateTime"] = AppEnv::getBuildDateTime();
    jsonObject["BuildType"] = GTA5SYNC_BUILDTYPE;
    jsonObject["QtVersion"] = qVersion();
    jsonDocument.setObject(jsonObject);
    return jsonDocument;
}

QJsonDocument TelemetryClass::getSystemLocaleList()
{
    QJsonDocument jsonDocument;
    QJsonObject jsonObject;
    QStringList languagesList = QLocale::system().uiLanguages();
    if (languagesList.length() >= 1)
    {
        jsonObject["PrimaryLanguage"] = languagesList.at(0);
    }
    if (languagesList.length() >= 2)
    {
        languagesList.removeAt(0);
        jsonObject["SecondaryLanguages"] = QJsonValue::fromVariant(languagesList);
    }
    jsonDocument.setObject(jsonObject);
    return jsonDocument;
}

QString TelemetryClass::categoryToString(TelemetryCategory category)
{
    switch (category)
    {
    case TelemetryCategory::OperatingSystemSpec:
        return QString("OperatingSystemSpec");
        break;
    case TelemetryCategory::HardwareSpec:
        return QString("HardwareSpec");
        break;
    case TelemetryCategory::UserLocaleData:
        return QString("UserLocaleData");
        break;
    case TelemetryCategory::ApplicationConfiguration:
        return QString("ApplicationConfiguration");
        break;
    case TelemetryCategory::UserFeedback:
        return QString("UserFeedback");
        break;
    case TelemetryCategory::ApplicationSpec:
        return QString("ApplicationSpec");
        break;
    case TelemetryCategory::CustomEmitted:
        return QString("CustomEmitted");
        break;
    default:
        return QString("UnknownCategory");
        break;
    }
}

void TelemetryClass::registerClient()
{
    QNetworkAccessManager *netManager = new QNetworkAccessManager();
    QNetworkRequest netRequest(TelemetryClassAuthenticator::getTrackingRegURL());
    netManager->get(netRequest);

    connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(registerFinished(QNetworkReply*)));
}

void TelemetryClass::pushStartupSet()
{
    push(TelemetryCategory::ApplicationSpec);
    push(TelemetryCategory::UserLocaleData);
    push(TelemetryCategory::OperatingSystemSpec);
    push(TelemetryCategory::HardwareSpec);
}

void TelemetryClass::pushFinished(QNetworkReply *reply)
{
#ifdef GTA5SYNC_DEBUG
    qDebug() << "Telemetry" << reply->readAll().trimmed();
#endif
    reply->deleteLater();
    sender()->deleteLater();
    emit pushed();
}

void TelemetryClass::registerFinished(QNetworkReply *reply)
{
    if (reply->canReadLine())
    {
        QByteArray readData = reply->readLine();
        if (QString::fromUtf8(readData).trimmed() == QString("Registration success!") && reply->canReadLine())
        {
            readData = reply->readLine();
            telemetryClientID = QString::fromUtf8(readData).trimmed();
            QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            settings.beginGroup("Telemetry");
            settings.setValue("ClientID", telemetryClientID);
            settings.endGroup();
#ifdef GTA5SYNC_DEBUG
            qDebug() << "Telemetry" << QString("Registration success!");
#endif
        }
        else
        {
#ifdef GTA5SYNC_DEBUG
            qDebug() << "Telemetry" << QString("Registration failed!");
#endif
        }
    }
    else
    {
#ifdef GTA5SYNC_DEBUG
        qDebug() << "Telemetry" << QString("Registration failed!");
#endif
    }
    reply->deleteLater();
    sender()->deleteLater();
    emit registered();
}
