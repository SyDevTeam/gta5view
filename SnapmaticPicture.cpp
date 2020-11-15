/*****************************************************************************
* gta5spv Grand Theft Auto Snapmatic Picture Viewer
* Copyright (C) 2016-2020 Syping
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

#include "SnapmaticPicture.h"
#include <QStringBuilder>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QVariantMap>
#include <QJsonArray>
#include <QFileInfo>
#include <QPainter>
#include <QString>
#include <QBuffer>
#include <QDebug>
#include <QImage>
#include <QSize>
#include <QFile>

#if QT_VERSION < 0x060000
#include <QTextCodec>
#endif

#if QT_VERSION >= 0x050000
#include <QSaveFile>
#else
#include "StandardPaths.h"
#endif

// IMAGES VALUES
#define snapmaticResolutionW 960
#define snapmaticResolutionH 536
#define snapmaticResolution QSize(snapmaticResolutionW, snapmaticResolutionH)

SnapmaticPicture::SnapmaticPicture(const QString &fileName, QObject *parent) : QObject(parent), picFilePath(fileName)
{
    reset();
}

SnapmaticPicture::~SnapmaticPicture()
{
}

void SnapmaticPicture::reset()
{
    // INIT PIC
    p_ragePhoto.clear();
    cachePicture = QImage();
    picExportFileName = QString();
    pictureStr = QString();
    lastStep = QString();
    sortStr = QString();

    // INIT PIC BOOLS
    isFormatSwitch = false;
    picOk = false;

    // INIT JSON
    jsonOk = false;

    // SNAPMATIC PROPERTIES
    localProperties = {};
}

bool SnapmaticPicture::preloadFile()
{
    QFile *picFile = new QFile(picFilePath);
    picFileName = QFileInfo(picFilePath).fileName();

    isFormatSwitch = false;

    if (!picFile->open(QFile::ReadOnly))
    {
        lastStep = "1;/1,OpenFile," % convertDrawStringForLog(picFilePath);
        delete picFile;
        return false;
    }

    p_ragePhoto.setIODevice(picFile);
    bool ok = p_ragePhoto.load();
    picFile->close();
    delete picFile;
    if (!ok)
        return false;

    if (picFilePath.right(4) != QLatin1String(".g5e"))
    {
        if (p_ragePhoto.photoFormat() == RagePhoto::PhotoFormat::G5EX)
            isFormatSwitch = true;
    }
    emit preloaded();
    return ok;
}

bool SnapmaticPicture::readingPicture(bool cacheEnabled_)
{
    // Start opening file
    // lastStep is like currentStep

    // Set boolean values
    cacheEnabled = cacheEnabled_;

    bool ok = true;
    if (!p_ragePhoto.isLoaded())
        ok = preloadFile();

    if (!ok)
        return false;

    if (cacheEnabled) picOk = cachePicture.loadFromData(p_ragePhoto.photoData(), "JPEG");
    if (!cacheEnabled)
    {
        QImage tempPicture;
        picOk = tempPicture.loadFromData(p_ragePhoto.photoData(), "JPEG");
    }

    parseJsonContent(); // JSON parsing is own function
    updateStrings();

    emit loaded();
    return picOk;
}

void SnapmaticPicture::updateStrings()
{
    QString cmpPicTitl = p_ragePhoto.title();
    cmpPicTitl.replace('\"', "''");
    cmpPicTitl.replace(' ', '_');
    cmpPicTitl.replace(':', '-');
    cmpPicTitl.remove('\\');
    cmpPicTitl.remove('{');
    cmpPicTitl.remove('}');
    cmpPicTitl.remove('/');
    cmpPicTitl.remove('<');
    cmpPicTitl.remove('>');
    cmpPicTitl.remove('*');
    cmpPicTitl.remove('?');
    cmpPicTitl.remove('.');
    pictureStr = tr("PHOTO - %1").arg(localProperties.createdDateTime.toString("MM/dd/yy HH:mm:ss"));
    sortStr = localProperties.createdDateTime.toString("yyMMddHHmmss") % QString::number(localProperties.uid);
    QString exportStr = localProperties.createdDateTime.toString("yyyyMMdd") % "-" % QString::number(localProperties.uid);
    if (getSnapmaticFormat() == SnapmaticFormat::G5E_Format) picFileName = "PGTA5" % QString::number(localProperties.uid);
    picExportFileName = exportStr % "_" % cmpPicTitl;
}

bool SnapmaticPicture::readingPictureFromFile(const QString &fileName, bool cacheEnabled_)
{
    if (!fileName.isEmpty())
    {
        picFilePath = fileName;
        return readingPicture(cacheEnabled_);
    }
    else
    {
        return false;
    }
}

bool SnapmaticPicture::setImage(const QImage &picture, bool eXtendMode)
{
    quint32 jpegPicStreamLength = p_ragePhoto.photoBuffer();
    QByteArray picByteArray;
    int comLvl = 100;
    bool saveSuccess = false;
    while (comLvl != 0 && !saveSuccess) {
        QByteArray picByteArrayT;
        QBuffer picStreamT(&picByteArrayT);
        picStreamT.open(QIODevice::WriteOnly);
        saveSuccess = picture.save(&picStreamT, "JPEG", comLvl);
        picStreamT.close();
        if (saveSuccess) {
            quint32 size = picByteArrayT.length();
            if (size > jpegPicStreamLength) {
                if (!eXtendMode) {
                    comLvl--;
                    saveSuccess = false;
                }
                else {
                    p_ragePhoto.setPhotoBuffer(size, true);
                    picByteArray = picByteArrayT;
                }
            }
            else {
                picByteArray = picByteArrayT;
            }
        }
    }
    if (saveSuccess)
        return setPictureStream(picByteArray);
    return false;
}

bool SnapmaticPicture::setPictureStream(const QByteArray &streamArray) // clean method
{
    bool success = p_ragePhoto.setPhotoData(streamArray);
    if (success) {
        if (cacheEnabled) {
            QImage replacedPicture;
            replacedPicture.loadFromData(streamArray);
            cachePicture = replacedPicture;
        }
        return true;
    }
    else {
        return false;
    }
}

bool SnapmaticPicture::setPictureTitl(const QString &newTitle_)
{
    QString newTitle = newTitle_;
    if (newTitle.length() > 39) {
        newTitle = newTitle.left(39);
    }
    p_ragePhoto.setTitle(newTitle);
    return true;
}

int SnapmaticPicture::getContentMaxLength()
{
    return p_ragePhoto.photoBuffer();
}

QString SnapmaticPicture::getExportPictureFileName()
{
    return picExportFileName;
}

QString SnapmaticPicture::getOriginalPictureFileName()
{
    QString newPicFileName = picFileName;
    if (picFileName.right(4) == ".bak")
    {
        newPicFileName = QString(picFileName).remove(picFileName.length() - 4, 4);
    }
    if (picFileName.right(7) == ".hidden")
    {
        newPicFileName = QString(picFileName).remove(picFileName.length() - 7, 7);
    }
    return newPicFileName;
}

QString SnapmaticPicture::getOriginalPictureFilePath()
{
    QString newPicFilePath = picFilePath;
    if (picFilePath.right(4) == ".bak")
    {
        newPicFilePath = QString(picFilePath).remove(picFilePath.length() - 4, 4);
    }
    if (picFilePath.right(7) == ".hidden")
    {
        newPicFilePath = QString(picFilePath).remove(picFilePath.length() - 7, 7);
    }
    return newPicFilePath;
}

QString SnapmaticPicture::getPictureFileName()
{
    return picFileName;
}

QString SnapmaticPicture::getPictureFilePath()
{
    return picFilePath;
}

QString SnapmaticPicture::getPictureSortStr()
{
    return sortStr;
}

QString SnapmaticPicture::getPictureTitl()
{
    return p_ragePhoto.title();
}

QString SnapmaticPicture::getPictureStr()
{
    return pictureStr;
}

QString SnapmaticPicture::getLastStep(bool readable)
{
    if (readable)
    {
        QStringList lastStepList = lastStep.split(";/");
        if (lastStepList.length() < 2) { return lastStep; }
        bool intOk;
        QStringList descStepList = lastStepList.at(1).split(",");
        if (descStepList.length() < 1) { return lastStep; }
        int argsCount = descStepList.at(0).toInt(&intOk);
        if (!intOk) { return lastStep; }
        if (argsCount == 1)
        {
            QString currentAction = descStepList.at(1);
            QString actionFile = descStepList.at(2);
            if (currentAction == "OpenFile")
            {
                return tr("open file %1").arg(actionFile);
            }
        }
        else if (argsCount == 3 || argsCount == 4)
        {
            QString currentAction = descStepList.at(1);
            QString actionFile = descStepList.at(2);
            QString actionError = descStepList.at(4);
            QString actionError2;
            if (argsCount == 4) { actionError2 = descStepList.at(5); }
            if (currentAction == "ReadingFile")
            {
                QString readableError = actionError;
                if (actionError == "NOHEADER")
                {
                    readableError = tr("header not exists");
                }
                else if (actionError == "MALFORMEDHEADER")
                {
                    readableError = tr("header is malformed");
                }
                else if (actionError == "NOJPEG" || actionError == "NOPIC")
                {
                    readableError = tr("picture not exists (%1)").arg(actionError);
                }
                else if (actionError == "NOJSON" || actionError == "CTJSON")
                {
                    readableError = tr("JSON not exists (%1)").arg(actionError);
                }
                else if (actionError == "NOTITL" || actionError == "CTTITL")
                {
                    readableError = tr("title not exists (%1)").arg(actionError);
                }
                else if (actionError == "NODESC" || actionError == "CTDESC")
                {
                    readableError = tr("description not exists (%1)").arg(actionError);
                }
                else if (actionError == "JSONINCOMPLETE" && actionError2 == "JSONERROR")
                {
                    readableError = tr("JSON is incomplete and malformed");
                }
                else if (actionError == "JSONINCOMPLETE")
                {
                    readableError = tr("JSON is incomplete");
                }
                else if (actionError == "JSONERROR")
                {
                    readableError = tr("JSON is malformed");
                }
                return tr("reading file %1 because of %2", "Example for %2: JSON is malformed error").arg(actionFile, readableError);
            }
            else
            {
                return lastStep;
            }
        }
        else
        {
            return lastStep;
        }
    }
    return lastStep;

}

QImage SnapmaticPicture::getImage(bool fastLoad)
{
    Q_UNUSED(fastLoad)
    if (cacheEnabled)
    {
        return cachePicture;
    }
    else
    {
        return QImage::fromData(p_ragePhoto.photoData(), "JPEG");
    }
    return QImage();
}

QByteArray SnapmaticPicture::getPictureStream()
{
    return p_ragePhoto.photoData();
}

bool SnapmaticPicture::isPicOk()
{
    return picOk;
}

void SnapmaticPicture::clearCache()
{
    cacheEnabled = false;
    cachePicture = QImage();
}

void SnapmaticPicture::emitUpdate()
{
    emit updated();
}

void SnapmaticPicture::emitCustomSignal(const QString &signal)
{
    emit customSignal(signal);
}

// JSON part

bool SnapmaticPicture::isJsonOk()
{
    return jsonOk;
}

QString SnapmaticPicture::getJsonStr()
{
    return QString::fromUtf8(p_ragePhoto.jsonData());
}

SnapmaticProperties SnapmaticPicture::getSnapmaticProperties()
{
    return localProperties;
}

void SnapmaticPicture::parseJsonContent()
{
    QJsonObject jsonObject = p_ragePhoto.jsonObject();
    QVariantMap jsonMap = jsonObject.toVariantMap();

    bool jsonIncomplete = false;
    bool jsonError = false;
    if (jsonObject.contains("loc"))
    {
        if (jsonObject["loc"].isObject())
        {
            QJsonObject locObject = jsonObject["loc"].toObject();
            if (locObject.contains("x"))
            {
                if (locObject["x"].isDouble()) { localProperties.location.x = locObject["x"].toDouble(); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
            if (locObject.contains("y"))
            {
                if (locObject["y"].isDouble()) { localProperties.location.y = locObject["y"].toDouble(); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
            if (locObject.contains("z"))
            {
                if (locObject["z"].isDouble()) { localProperties.location.z = locObject["z"].toDouble(); }
                else { jsonError = true; }
            }
            else { jsonIncomplete = true; }
        }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("uid"))
    {
        bool uidOk;
        localProperties.uid = jsonMap["uid"].toInt(&uidOk);
        if (!uidOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("area"))
    {
        if (jsonObject["area"].isString()) { localProperties.location.area = jsonObject["area"].toString(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("crewid"))
    {
        bool crewIDOk;
        localProperties.crewID = jsonMap["crewid"].toInt(&crewIDOk);
        if (!crewIDOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("street"))
    {
        bool streetIDOk;
        localProperties.streetID = jsonMap["street"].toInt(&streetIDOk);
        if (!streetIDOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("creat"))
    {
        bool timestampOk;
        QDateTime createdTimestamp;
        localProperties.createdTimestamp = jsonMap["creat"].toUInt(&timestampOk);
#if QT_VERSION >= 0x060000
        createdTimestamp.setSecsSinceEpoch(localProperties.createdTimestamp);
#else
        createdTimestamp.setTime_t(localProperties.createdTimestamp);
#endif
        localProperties.createdDateTime = createdTimestamp;
        if (!timestampOk) { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("plyrs"))
    {
        if (jsonObject["plyrs"].isArray()) { localProperties.playersList = jsonMap["plyrs"].toStringList(); }
        else { jsonError = true; }
    }
    // else { jsonIncomplete = true; } // 2016 Snapmatic pictures left out plyrs when none are captured, so don't force exists on that one
    if (jsonObject.contains("meme"))
    {
        if (jsonObject["meme"].isBool()) { localProperties.isMeme = jsonObject["meme"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("mug"))
    {
        if (jsonObject["mug"].isBool()) { localProperties.isMug = jsonObject["mug"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("slf"))
    {
        if (jsonObject["slf"].isBool()) { localProperties.isSelfie = jsonObject["slf"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("drctr"))
    {
        if (jsonObject["drctr"].isBool()) { localProperties.isFromDirector = jsonObject["drctr"].toBool(); }
        else { jsonError = true; }
    }
    else { jsonIncomplete = true; }
    if (jsonObject.contains("rsedtr"))
    {
        if (jsonObject["rsedtr"].isBool()) { localProperties.isFromRSEditor = jsonObject["rsedtr"].toBool(); }
        else { jsonError = true; }
    }
    // else { jsonIncomplete = true; } // Game release Snapmatic pictures prior May 2015 left out rsedtr, so don't force exists on that one

    if (!jsonIncomplete && !jsonError)
    {
        jsonOk = true;
    }
    else
    {
        if (jsonIncomplete && jsonError)
        {
            lastStep = "2;/4,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,JSONINCOMPLETE,JSONERROR";
        }
        else if (jsonIncomplete)
        {
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,JSONINCOMPLETE";
        }
        else if (jsonError)
        {
            lastStep = "2;/3,ReadingFile," % convertDrawStringForLog(picFilePath) % ",3,JSONERROR";
        }
        jsonOk = false;
    }
}

bool SnapmaticPicture::setSnapmaticProperties(SnapmaticProperties properties)
{
    QJsonObject jsonObject = p_ragePhoto.jsonObject();

    QJsonObject locObject;
    locObject["x"] = properties.location.x;
    locObject["y"] = properties.location.y;
    locObject["z"] = properties.location.z;

    jsonObject["loc"] = locObject;
    jsonObject["uid"] = properties.uid;
    jsonObject["area"] = properties.location.area;
    jsonObject["crewid"] = properties.crewID;
    jsonObject["street"] = properties.streetID;
    jsonObject["creat"] = QJsonValue::fromVariant(properties.createdTimestamp);
    jsonObject["plyrs"] = QJsonValue::fromVariant(properties.playersList);
    jsonObject["meme"] = properties.isMeme;
    jsonObject["mug"] = properties.isMug;
    jsonObject["slf"] = properties.isSelfie;
    jsonObject["drctr"] = properties.isFromDirector;
    jsonObject["rsedtr"] = properties.isFromRSEditor;

    QJsonDocument jsonDocument(jsonObject);
    if (setJsonStr(QString::fromUtf8(jsonDocument.toJson(QJsonDocument::Compact))))
    {
        localProperties = properties;
        return true;
    }
    return false;
}

bool SnapmaticPicture::setJsonStr(const QString &newJsonStr, bool updateProperties)
{
    if (p_ragePhoto.setJsonData(newJsonStr.toUtf8())) {
        if (updateProperties)
            parseJsonContent();
        return true;
    }
    else {
        return false;
    }
}

// FILE MANAGEMENT

bool SnapmaticPicture::exportPicture(const QString &fileName, SnapmaticFormat format_)
{
    // Keep current format when Auto_Format is used
    SnapmaticFormat format = format_;
    if (format_ == SnapmaticFormat::Auto_Format)
    {
        if (p_ragePhoto.photoFormat() == RagePhoto::PhotoFormat::G5EX)
        {
            format = SnapmaticFormat::G5E_Format;
        }
        else
        {
            format = SnapmaticFormat::PGTA_Format;
        }
    }

    bool saveSuccess = false;
#if QT_VERSION >= 0x050000
    QSaveFile *picFile = new QSaveFile(fileName);
#else
    QFile *picFile = new QFile(StandardPaths::tempLocation() % "/" % QFileInfo(fileName).fileName() % ".tmp");
#endif
    if (picFile->open(QIODevice::WriteOnly))
    {
        if (format == SnapmaticFormat::G5E_Format)
        {
            p_ragePhoto.save(picFile, RagePhoto::PhotoFormat::G5EX);
#if QT_VERSION >= 0x050000
            saveSuccess = picFile->commit();
#else
            saveSuccess = true;
            picFile->close();
#endif
            delete picFile;
        }
        else if (format == SnapmaticFormat::JPEG_Format)
        {
            picFile->write(p_ragePhoto.photoData());
#if QT_VERSION >= 0x050000
            saveSuccess = picFile->commit();
#else
            saveSuccess = true;
            picFile->close();
#endif
            delete picFile;
        }
        else
        {
            p_ragePhoto.save(picFile, RagePhoto::PhotoFormat::GTA5);
#if QT_VERSION >= 0x050000
            saveSuccess = picFile->commit();
#else
            saveSuccess = true;
            picFile->close();
#endif
            delete picFile;
        }
#if QT_VERSION <= 0x050000
        if (saveSuccess) {
            bool tempBakCreated = false;
            if (QFile::exists(fileName)) {
                if (!QFile::rename(fileName, fileName % ".tmp")) {
                    QFile::remove(StandardPaths::tempLocation() % "/" % QFileInfo(fileName).fileName() % ".tmp");
                    return false;
                }
                tempBakCreated = true;
            }
            if (!QFile::rename(StandardPaths::tempLocation() % "/" % QFileInfo(fileName).fileName() % ".tmp", fileName)) {
                QFile::remove(StandardPaths::tempLocation() % "/" % QFileInfo(fileName).fileName() % ".tmp");
                if (tempBakCreated)
                    QFile::rename(fileName % ".tmp", fileName);
                return false;
            }
            if (tempBakCreated)
                QFile::remove(fileName % ".tmp");
        }
#endif
        return saveSuccess;
    }
    else
    {
        delete picFile;
        return saveSuccess;
    }
}

void SnapmaticPicture::setPicFileName(const QString &picFileName_)
{
    picFileName = picFileName_;
}

void SnapmaticPicture::setPicFilePath(const QString &picFilePath_)
{
    picFilePath = picFilePath_;
}

bool SnapmaticPicture::deletePicFile()
{
    bool success = false;
    if (!QFile::exists(picFilePath))
    {
        success = true;
    }
    else if (QFile::remove(picFilePath))
    {
        success = true;
    }
    if (isHidden())
    {
        const QString picBakPath = QString(picFilePath).remove(picFilePath.length() - 7, 7) % ".bak";
        if (QFile::exists(picBakPath)) QFile::remove(picBakPath);
    }
    else {
        const QString picBakPath = picFilePath % ".bak";
        if (QFile::exists(picBakPath)) QFile::remove(picBakPath);
    }
    return success;
}

// VISIBILITY

bool SnapmaticPicture::isHidden()
{
    if (picFilePath.right(7) == QLatin1String(".hidden"))
    {
        return true;
    }
    return false;
}

bool SnapmaticPicture::isVisible()
{
    if (picFilePath.right(7) == QLatin1String(".hidden"))
    {
        return false;
    }
    return true;
}

bool SnapmaticPicture::setPictureHidden()
{
    if (p_ragePhoto.photoFormat() == RagePhoto::PhotoFormat::G5EX)
    {
        return false;
    }
    if (!isHidden())
    {
        QString newPicFilePath = QString(picFilePath % ".hidden");
        if (QFile::rename(picFilePath, newPicFilePath))
        {
            picFilePath = newPicFilePath;
            return true;
        }
        return false;
    }
    return true;
}

bool SnapmaticPicture::setPictureVisible()
{
    if (p_ragePhoto.photoFormat() == RagePhoto::PhotoFormat::G5EX)
    {
        return false;
    }
    if (isHidden())
    {
        QString newPicFilePath = QString(picFilePath).remove(picFilePath.length() - 7, 7);
        if (QFile::rename(picFilePath, newPicFilePath))
        {
            picFilePath = newPicFilePath;
            return true;
        }
        return false;
    }
    return true;
}

// PREDEFINED PROPERTIES

QSize SnapmaticPicture::getSnapmaticResolution()
{
    return snapmaticResolution;
}

// SNAPMATIC FORMAT

SnapmaticFormat SnapmaticPicture::getSnapmaticFormat()
{
    if (p_ragePhoto.photoFormat() == RagePhoto::PhotoFormat::G5EX)
    {
        return SnapmaticFormat::G5E_Format;
    }
    return SnapmaticFormat::PGTA_Format;
}

void SnapmaticPicture::setSnapmaticFormat(SnapmaticFormat format)
{
    if (format == SnapmaticFormat::G5E_Format)
    {
        p_ragePhoto.setPhotoFormat(RagePhoto::PhotoFormat::G5EX);
        return;
    }
    else if (format == SnapmaticFormat::PGTA_Format)
    {
        p_ragePhoto.setPhotoFormat(RagePhoto::PhotoFormat::GTA5);
        return;
    }
    qDebug() << "setSnapmaticFormat: Invalid SnapmaticFormat defined, valid SnapmaticFormats are G5E_Format and PGTA_Format";
}

bool SnapmaticPicture::isFormatSwitched()
{
    return isFormatSwitch;
}

// VERIFY CONTENT

bool SnapmaticPicture::verifyTitle(const QString &title)
{
    // VERIFY TITLE FOR BE A VALID SNAPMATIC TITLE
    if (title.length() <= 39 && title.length() > 0)
    {
        for (const QChar &titleChar : title)
        {
            if (!verifyTitleChar(titleChar)) return false;
        }
        return true;
    }
    return false;
}

bool SnapmaticPicture::verifyTitleChar(const QChar &titleChar)
{
    // VERIFY CHAR FOR BE A VALID SNAPMATIC CHARACTER
    if (titleChar.isLetterOrNumber() || titleChar.isPrint())
    {
        if (titleChar == '<' || titleChar == '>' || titleChar == '\\') return false;
        return true;
    }
    return false;
}

// STRING OPERATIONS

QString SnapmaticPicture::parseTitleString(const QByteArray &commitBytes, int maxLength)
{
    Q_UNUSED(maxLength)
#if QT_VERSION >= 0x060000
    QStringDecoder strDecoder = QStringDecoder(QStringDecoder::Utf16LE);
    QString retStr = strDecoder(commitBytes);
    retStr = retStr.trimmed();
#else
    QString retStr = QTextCodec::codecForName("UTF-16LE")->toUnicode(commitBytes).trimmed();
#endif
    retStr.remove(QChar('\x00'));
    return retStr;
}

QString SnapmaticPicture::convertDrawStringForLog(const QString &inputStr)
{
    QString outputStr = inputStr;
    return outputStr.replace("&","&u;").replace(",", "&c;");
}

QString SnapmaticPicture::convertLogStringForDraw(const QString &inputStr)
{
    QString outputStr = inputStr;
    return outputStr.replace("&c;",",").replace("&u;", "&");
}

// RAGEPHOTO

RagePhoto* SnapmaticPicture::ragePhoto()
{
    return &p_ragePhoto;
}
