/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
* Copyright (C) 2016-2021 Syping
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

#include "SnapmaticEditor.h"
#include "ui_SnapmaticEditor.h"
#include "SnapmaticPicture.h"
#include "PlayerListDialog.h"
#include "StringParser.h"
#include "wrapper.h"
#include "AppEnv.h"
#include "config.h"
#include <QStringBuilder>
#include <QTextDocument>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QFile>

#ifdef GTA5SYNC_TELEMETRY
#include "TelemetryClass.h"
#include <QJsonDocument>
#include <QJsonObject>
#endif

SnapmaticEditor::SnapmaticEditor(CrewDatabase *crewDB, ProfileDatabase *profileDB, QWidget *parent) :
    QDialog(parent), crewDB(crewDB), profileDB(profileDB),
    ui(new Ui::SnapmaticEditor)
{
    // Set Window Flags
#if QT_VERSION >= 0x050900
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
#else
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
#endif

    ui->setupUi(this);
    ui->cmdCancel->setDefault(true);
    ui->cmdCancel->setFocus();

    // Set Icon for Apply Button
    if (QIcon::hasThemeIcon("dialog-ok-apply")) {
        ui->cmdApply->setIcon(QIcon::fromTheme("dialog-ok-apply"));
    }
    else if (QIcon::hasThemeIcon("dialog-apply")) {
        ui->cmdApply->setIcon(QIcon::fromTheme("dialog-apply"));
    }
    else if (QIcon::hasThemeIcon("gtk-apply")) {
        ui->cmdApply->setIcon(QIcon::fromTheme("gtk-apply"));
    }
    else if (QIcon::hasThemeIcon("dialog-ok")) {
        ui->cmdApply->setIcon(QIcon::fromTheme("dialog-ok"));
    }
    else if (QIcon::hasThemeIcon("gtk-ok")) {
        ui->cmdApply->setIcon(QIcon::fromTheme("dialog-ok"));
    }

    // Set Icon for Cancel Button
    if (QIcon::hasThemeIcon("dialog-cancel")) {
        ui->cmdCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
    }
    else if (QIcon::hasThemeIcon("gtk-cancel")) {
        ui->cmdCancel->setIcon(QIcon::fromTheme("gtk-cancel"));
    }

    snapmaticTitle = QString();
    smpic = 0;

#ifndef Q_OS_ANDROID
    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    resize(400 * screenRatio, 360 * screenRatio);
#endif
}

SnapmaticEditor::~SnapmaticEditor()
{
    delete ui;
}

void SnapmaticEditor::selfie_toggled(bool checked)
{
    isSelfie = checked;
}


void SnapmaticEditor::mugshot_toggled(bool checked)
{
    if (checked) {
        isMugshot = true;
        ui->cbDirector->setEnabled(false);
        ui->cbDirector->setChecked(false);
    }
    else {
        isMugshot = false;
        ui->cbDirector->setEnabled(true);
    }
}

void SnapmaticEditor::editor_toggled(bool checked)
{
    if (checked) {
        isEditor = true;
        ui->cbDirector->setEnabled(false);
        ui->cbDirector->setChecked(false);
    }
    else {
        isEditor = false;
        ui->cbDirector->setEnabled(true);
    }
}

void SnapmaticEditor::on_rbSelfie_toggled(bool checked)
{
    if (checked) {
        mugshot_toggled(false);
        editor_toggled(false);
        selfie_toggled(true);
    }
}

void SnapmaticEditor::on_rbMugshot_toggled(bool checked)
{
    if (checked) {
        selfie_toggled(false);
        editor_toggled(false);
        mugshot_toggled(true);
    }
}

void SnapmaticEditor::on_rbEditor_toggled(bool checked)
{
    if (checked) {
        selfie_toggled(false);
        mugshot_toggled(false);
        editor_toggled(true);
    }
}

void SnapmaticEditor::on_rbCustom_toggled(bool checked)
{
    if (checked) {
        selfie_toggled(false);
        mugshot_toggled(false);
        editor_toggled(false);
    }
}

void SnapmaticEditor::setSnapmaticPicture(SnapmaticPicture *picture)
{
    smpic = picture;
    snapmaticProperties = smpic->getSnapmaticProperties();
    ui->rbCustom->setChecked(true);
    crewID = snapmaticProperties.crewID;
    isSelfie = snapmaticProperties.isSelfie;
    isMugshot = snapmaticProperties.isMug;
    isEditor = snapmaticProperties.isFromRSEditor;
    playersList = snapmaticProperties.playersList;
    ui->cbDirector->setChecked(snapmaticProperties.isFromDirector);
    ui->cbMeme->setChecked(snapmaticProperties.isMeme);
    if (isSelfie) {
        ui->rbSelfie->setChecked(true);
    }
    else if (isMugshot) {
        ui->rbMugshot->setChecked(true);
    }
    else if (isEditor) {
        ui->rbEditor->setChecked(true);
    }
    else {
        ui->rbCustom->setChecked(true);
    }
    setSnapmaticCrew(returnCrewName(crewID));
    setSnapmaticTitle(picture->getPictureTitle());
    setSnapmaticPlayers(insertPlayerNames(playersList));
}

void SnapmaticEditor::insertPlayerNames(QStringList *players)
{
    for (int i = 0; i < players->size(); ++i) {
        players->replace(i, profileDB->getPlayerName(players->at(i)));
    }
}

QStringList SnapmaticEditor::insertPlayerNames(const QStringList &players)
{
    QStringList playersWI = players;
    insertPlayerNames(&playersWI);
    return playersWI;
}

void SnapmaticEditor::setSnapmaticPlayers(const QStringList &players)
{
    QString editStr = QString("<a href=\"g5e://editplayers\" style=\"text-decoration: none;\">%1</a>").arg(tr("Edit"));
    QString playersStr;
    if (players.length() != 1) {
        playersStr = tr("Players: %1 (%2)", "Multiple Player are inserted here");
    }
    else {
        playersStr = tr("Player: %1 (%2)", "One Player is inserted here");
    }
    if (players.length() != 0) {
        ui->labPlayers->setText(playersStr.arg(players.join(", "), editStr));
    }
    else {
        ui->labPlayers->setText(playersStr.arg(QApplication::translate("PictureDialog", "No Players"), editStr));
    }
#ifndef Q_OS_ANDROID
    ui->gbValues->resize(ui->gbValues->width(), ui->gbValues->heightForWidth(ui->gbValues->width()));
    ui->frameWidget->resize(ui->gbValues->width(), ui->frameWidget->heightForWidth(ui->frameWidget->width()));
    if (heightForWidth(width()) > height())
        resize(width(), heightForWidth(width()));
#endif
}

void SnapmaticEditor::setSnapmaticTitle(const QString &title)
{
    if (title.length() > 39) {
        snapmaticTitle = title.left(39);
    }
    else {
        snapmaticTitle = title;
    }
    QString editStr = QString("<a href=\"g5e://edittitle\" style=\"text-decoration: none;\">%1</a>").arg(tr("Edit"));
    QString titleStr = tr("Title: %1 (%2)").arg(StringParser::escapeString(snapmaticTitle), editStr);
    ui->labTitle->setText(titleStr);
    if (SnapmaticPicture::verifyTitle(snapmaticTitle)) {
        ui->labAppropriate->setText(tr("Appropriate: %1").arg(QString("<span style=\"color: green\">%1</span>").arg(tr("Yes", "Yes, should work fine"))));
    }
    else {
        ui->labAppropriate->setText(tr("Appropriate: %1").arg(QString("<span style=\"color: red\">%1</span>").arg(tr("No", "No, could lead to issues"))));
    }
#ifndef Q_OS_ANDROID
    ui->gbValues->resize(ui->gbValues->width(), ui->gbValues->heightForWidth(ui->gbValues->width()));
    ui->frameWidget->resize(ui->gbValues->width(), ui->frameWidget->heightForWidth(ui->frameWidget->width()));
    if (heightForWidth(width()) > height())
        resize(width(), heightForWidth(width()));
#endif
}

void SnapmaticEditor::setSnapmaticCrew(const QString &crew)
{
    QString editStr = QString("<a href=\"g5e://editcrew\" style=\"text-decoration: none;\">%1</a>").arg(tr("Edit"));
    QString crewStr = tr("Crew: %1 (%2)").arg(StringParser::escapeString(crew), editStr);
    ui->labCrew->setText(crewStr);
#ifndef Q_OS_ANDROID
    ui->gbValues->resize(ui->gbValues->width(), ui->gbValues->heightForWidth(ui->gbValues->width()));
    ui->frameWidget->resize(ui->gbValues->width(), ui->frameWidget->heightForWidth(ui->frameWidget->width()));
    if (heightForWidth(width()) > height())
        resize(width(), heightForWidth(width()));
#endif
}

QString SnapmaticEditor::returnCrewName(int crewID_)
{
    return crewDB->getCrewName(crewID_);
}

void SnapmaticEditor::on_cmdCancel_clicked()
{
    close();
}

void SnapmaticEditor::on_cmdApply_clicked()
{
    if (ui->cbQualify->isChecked()) {
        qualifyAvatar();
    }
    snapmaticProperties.crewID = crewID;
    snapmaticProperties.isSelfie = isSelfie;
    snapmaticProperties.isMug = isMugshot;
    snapmaticProperties.isFromRSEditor = isEditor;
    snapmaticProperties.isFromDirector = ui->cbDirector->isChecked();
    snapmaticProperties.isMeme = ui->cbMeme->isChecked();
    snapmaticProperties.playersList = playersList;
    if (smpic) {
        QString currentFilePath = smpic->getPictureFilePath();
        QString originalFilePath = smpic->getOriginalPictureFilePath();
        QString backupFileName = originalFilePath % ".bak";
        if (!QFile::exists(backupFileName)) {
            QFile::copy(currentFilePath, backupFileName);
        }
        SnapmaticProperties fallbackProperties = smpic->getSnapmaticProperties();
        QString fallbackTitle = smpic->getPictureTitle();
        smpic->setSnapmaticProperties(snapmaticProperties);
        smpic->setPictureTitle(snapmaticTitle);
        if (!smpic->exportPicture(currentFilePath)) {
            QMessageBox::warning(this, tr("Snapmatic Properties"), tr("Patching of Snapmatic Properties failed because of I/O Error"));
            smpic->setSnapmaticProperties(fallbackProperties);
            smpic->setPictureTitle(fallbackTitle);
        }
        else {
            smpic->updateStrings();
            smpic->emitUpdate();
#ifdef GTA5SYNC_TELEMETRY
            QSettings telemetrySettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
            telemetrySettings.beginGroup("Telemetry");
            bool pushUsageData = telemetrySettings.value("PushUsageData", false).toBool();
            telemetrySettings.endGroup();
            if (pushUsageData && Telemetry->canPush()) {
                QJsonDocument jsonDocument;
                QJsonObject jsonObject;
                jsonObject["Type"] = "PropertyEdited";
                jsonObject["EditedSize"] = QString::number(smpic->getContentMaxLength());
#if QT_VERSION >= 0x060000
                jsonObject["EditedTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
                jsonObject["EditedTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
                jsonDocument.setObject(jsonObject);
                Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
            }
#endif
        }
    }
    close();
}

void SnapmaticEditor::qualifyAvatar()
{
    ui->rbSelfie->setChecked(true);
    ui->cbDirector->setChecked(false);
    ui->cbMeme->setChecked(false);
    ui->cmdApply->setDefault(true);
}

void SnapmaticEditor::on_cbQualify_toggled(bool checked)
{
    if (checked) {
        ui->cbMeme->setEnabled(false);
        ui->cbDirector->setEnabled(false);
        ui->rbCustom->setEnabled(false);
        ui->rbSelfie->setEnabled(false);
        ui->rbEditor->setEnabled(false);
        ui->rbMugshot->setEnabled(false);
    }
    else {
        ui->cbMeme->setEnabled(true);
        ui->rbCustom->setEnabled(true);
        ui->rbSelfie->setEnabled(true);
        ui->rbEditor->setEnabled(true);
        ui->rbMugshot->setEnabled(true);
        if (ui->rbSelfie->isChecked() || ui->rbCustom->isChecked()) {
            ui->cbDirector->setEnabled(true);
        }
    }
}

void SnapmaticEditor::on_labPlayers_linkActivated(const QString &link)
{
    if (link == "g5e://editplayers") {
        PlayerListDialog *playerListDialog = new PlayerListDialog(playersList, profileDB, this);
        connect(playerListDialog, SIGNAL(playerListUpdated(QStringList)), this, SLOT(playerListUpdated(QStringList)));
        playerListDialog->setModal(true);
        playerListDialog->show();
        playerListDialog->exec();
        delete playerListDialog;
    }
}

void SnapmaticEditor::on_labTitle_linkActivated(const QString &link)
{
    if (link == "g5e://edittitle") {
        bool ok;
        QString newTitle = QInputDialog::getText(this, tr("Snapmatic Title"), tr("New Snapmatic title:"), QLineEdit::Normal, snapmaticTitle, &ok, windowFlags());
        if (ok && !newTitle.isEmpty()) {
            setSnapmaticTitle(newTitle);
        }
    }
}

void SnapmaticEditor::on_labCrew_linkActivated(const QString &link)
{
    if (link == "g5e://editcrew") {
        bool ok;
        int indexNum = 0;
        QStringList itemList;
        QStringList crewList = crewDB->getCrews();
        if (!crewList.contains(QLatin1String("0"))) {
            crewList += QLatin1String("0");
        }
        crewList.sort();
        for (const QString &crew : crewList) {
            itemList += QString("%1 (%2)").arg(crew, returnCrewName(crew.toInt()));
        }
        if (crewList.contains(QString::number(crewID))) {
            indexNum = crewList.indexOf(QString::number(crewID));
        }
        QString newCrew = QInputDialog::getItem(this, tr("Snapmatic Crew"), tr("New Snapmatic crew:"), itemList, indexNum, true, &ok, windowFlags());
        if (ok && !newCrew.isEmpty()) {
            if (newCrew.contains(" "))
                newCrew = newCrew.split(" ").at(0);
            if (newCrew.length() > 10)
                return;
            for (const QChar &crewChar : qAsConst(newCrew)) {
                if (!crewChar.isNumber()) {
                    return;
                }
            }
            if (!crewList.contains(newCrew)) {
                crewDB->addCrew(crewID);
            }
            crewID = newCrew.toInt();
            setSnapmaticCrew(returnCrewName(crewID));
        }
    }
}

void SnapmaticEditor::playerListUpdated(QStringList playerList)
{
    playersList = playerList;
    setSnapmaticPlayers(insertPlayerNames(playerList));
}
