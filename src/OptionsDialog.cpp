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

#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"
#include "TranslationClass.h"
#include "StandardPaths.h"
#include "UserInterface.h"
#include "AppEnv.h"
#include "config.h"
#include <QStringBuilder>
#include <QJsonDocument>
#include <QStyleFactory>
#include <QApplication>
#include <QJsonObject>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QStringList>
#include <QClipboard>
#include <QLocale>
#include <QString>
#include <QTimer>
#include <QDebug>
#include <QList>
#include <QDir>

#if QT_VERSION >= 0x050000
#include <QScreen>
#else
#include <QDesktopWidget>
#endif

#ifdef GTA5SYNC_TELEMETRY
#include "TelemetryClass.h"
#endif

OptionsDialog::OptionsDialog(ProfileDatabase *profileDB, QWidget *parent) :
    QDialog(parent), profileDB(profileDB),
    ui(new Ui::OptionsDialog)
{
    // Set Window Flags
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    // Setup User Interface
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    ui->cmdCancel->setDefault(true);
    ui->cmdCancel->setFocus();

    qreal screenRatioPR = AppEnv::screenRatioPR();

    // Set Icon for OK Button
    if (QIcon::hasThemeIcon("dialog-ok")) {
        ui->cmdOK->setIcon(QIcon::fromTheme("dialog-ok"));
    }
    else if (QIcon::hasThemeIcon("gtk-ok")) {
        ui->cmdOK->setIcon(QIcon::fromTheme("gtk-ok"));
    }

    // Set Icon for Cancel Button
    if (QIcon::hasThemeIcon("dialog-cancel")) {
        ui->cmdCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
    }
    else if (QIcon::hasThemeIcon("gtk-cancel")) {
        ui->cmdCancel->setIcon(QIcon::fromTheme("gtk-cancel"));
    }

    // Set Icon for Copy Button
    if (QIcon::hasThemeIcon("edit-copy")) {
        ui->cmdCopyStatsID->setIcon(QIcon::fromTheme("edit-copy"));
    }

    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    setupTreeWidget();
    setupLanguageBox(&settings);
    setupRadioButtons(&settings);
    setupDefaultProfile(&settings);
    setupCustomGameFolder(&settings);
    setupInterfaceSettings(&settings);
    setupStatisticsSettings(&settings);
    setupSnapmaticPictureViewer(&settings);
    setupWindowsGameSettings();

#ifndef Q_QS_ANDROID
    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    resize(435 * screenRatio, 405 * screenRatio);
#endif

    ui->rbModern->setText(ui->rbModern->text().arg(GTA5SYNC_APPSTR));
    ui->rbClassic->setText(ui->rbClassic->text().arg(GTA5SYNC_APPSTR));
    setWindowTitle(windowTitle().arg(GTA5SYNC_APPSTR));
}

OptionsDialog::~OptionsDialog()
{
    qDeleteAll(playerItems.begin(), playerItems.end());
    playerItems.clear();
    delete ui;
}

void OptionsDialog::setupTreeWidget()
{
    const QStringList players = profileDB->getPlayers();
    if (players.length() != 0) {
        for (auto it = players.constBegin(); it != players.constEnd(); it++) {
            bool ok;
            int playerID = it->toInt(&ok);
            if (ok) {
                const QString playerName = profileDB->getPlayerName(playerID);

                QStringList playerTreeViewList;
                playerTreeViewList += *it;
                playerTreeViewList += playerName;

                QTreeWidgetItem *playerItem = new QTreeWidgetItem(playerTreeViewList);
                ui->twPlayers->addTopLevelItem(playerItem);
                playerItems += playerItem;
            }
        }
        ui->twPlayers->sortItems(1, Qt::AscendingOrder);
    }
    else {
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabPlayers));
    }
}

void OptionsDialog::setupLanguageBox(QSettings *settings)
{
    settings->beginGroup("Interface");
    currentLanguage = settings->value("Language", "System").toString();
    currentAreaLanguage = settings->value("AreaLanguage", "Auto").toString();
    settings->endGroup();

    const QString cbSysStr = tr("%1 (Language priority)", "First language a person can talk with a different person/application. \"Native\" or \"Not Native\".").arg(tr("System",
                                                                                                                                                                  "System in context of System default"));
#ifdef Q_OS_WIN
    QString cbAutoStr;
    if (AppEnv::getGTAVLanguage(AppEnv::getGTAVVersion()) != GameLanguage::Undefined) {
        cbAutoStr = tr("%1 (Game language)", "Next closest language compared to the Game settings").arg(tr("Auto", "Automatic language choice."));
    }
    else {
        cbAutoStr = tr("%1 (Closest to Interface)", "Next closest language compared to the Interface").arg(tr("Auto", "Automatic language choice."));
    }
#else
    const QString cbAutoStr = tr("%1 (Closest to Interface)", "Next closest language compared to the Interface").arg(tr("Auto", "Automatic language choice."));
#endif
    ui->cbLanguage->addItem(cbSysStr, "System");
    ui->cbAreaLanguage->addItem(cbAutoStr, "Auto");

    QStringList availableLanguages;
    availableLanguages << QString("en_GB");
#ifndef GTA5SYNC_QCONF
    availableLanguages << TranslationClass::listTranslations(AppEnv::getExLangFolder());
#endif
    availableLanguages << TranslationClass::listTranslations(AppEnv::getInLangFolder());
    availableLanguages.removeDuplicates();
    availableLanguages.sort();

    for (const QString &lang : qAsConst(availableLanguages)) {
        QLocale langLocale(lang);
        const QString cbLangStr = langLocale.nativeLanguageName() % " (" % langLocale.nativeCountryName() % ") [" % lang % "]";
        const QString langIconPath = AppEnv::getImagesFolder() % "/flag-" % TranslationClass::getCountryCode(langLocale) % ".png";

        if (QFile::exists(langIconPath)) {
            ui->cbLanguage->addItem(QIcon(langIconPath), cbLangStr, lang);
        }
        else {
            ui->cbLanguage->addItem(cbLangStr, lang);
        }
        if (currentLanguage == lang) {
#if QT_VERSION >= 0x050000
            ui->cbLanguage->setCurrentText(cbLangStr);
#else
            int indexOfLang = ui->cbLanguage->findText(cbLangStr);
            ui->cbLanguage->setCurrentIndex(indexOfLang);
#endif
        }
    }

    QString aCurrentLanguage = QString("en_GB");
    if (Translator->isLanguageLoaded())
        aCurrentLanguage = Translator->getCurrentLanguage();
    QLocale currentLocale = QLocale(aCurrentLanguage);
    ui->labCurrentLanguage->setText(tr("Current: %1").arg(currentLocale.nativeLanguageName() % " (" % currentLocale.nativeCountryName() % ") [" % aCurrentLanguage % "]"));

    availableLanguages.clear();
    availableLanguages << TranslationClass::listAreaTranslations();
    availableLanguages.removeDuplicates();
    availableLanguages.sort();

    for (const QString &lang : qAsConst(availableLanguages)) {
        // correcting Language Location if possible
        QString aLang = lang;
        if (QFile::exists(":/global/global." % lang % ".loc")) {
            QFile locFile(":/global/global." % lang % ".loc");
            if (locFile.open(QFile::ReadOnly)) {
                aLang = QString::fromUtf8(locFile.readLine()).trimmed();
                locFile.close();
            }
        }

        QLocale langLocale(aLang);
        const QString cbLangStr = langLocale.nativeLanguageName() % " (" % langLocale.nativeCountryName() % ") [" % aLang % "]";
        ui->cbAreaLanguage->addItem(cbLangStr, lang);
        if (currentAreaLanguage == lang) {
#if QT_VERSION >= 0x050000
            ui->cbAreaLanguage->setCurrentText(cbLangStr);
#else
            int indexOfLang = ui->cbAreaLanguage->findText(cbLangStr);
            ui->cbAreaLanguage->setCurrentIndex(indexOfLang);
#endif
        }
    }

    QString aCurrentAreaLanguage = Translator->getCurrentAreaLanguage();
    if (QFile::exists(":/global/global." % aCurrentAreaLanguage % ".loc")) {
        qDebug() << "locFile found";
        QFile locFile(":/global/global." % aCurrentAreaLanguage % ".loc");
        if (locFile.open(QFile::ReadOnly)) {
            aCurrentAreaLanguage = QString::fromUtf8(locFile.readLine()).trimmed();
            locFile.close();
        }
    }
    currentLocale = QLocale(aCurrentAreaLanguage);
    ui->labCurrentAreaLanguage->setText(tr("Current: %1").arg(currentLocale.nativeLanguageName() % " (" % currentLocale.nativeCountryName() % ") [" % aCurrentAreaLanguage % "]"));
}

void OptionsDialog::setupRadioButtons(QSettings *settings)
{
    bool contentModeOk;
    settings->beginGroup("Profile");
    contentMode = settings->value("ContentMode", 0).toInt(&contentModeOk);
    settings->endGroup();

    if (contentModeOk) {
        switch (contentMode) {
        case 0:
        case 20:
            ui->rbModern->setChecked(true);
            ui->cbDoubleclick->setChecked(false);
            break;
        case 1:
        case 2:
        case 21:
            ui->rbModern->setChecked(true);
            ui->cbDoubleclick->setChecked(true);
            break;
        case 10:
            ui->rbClassic->setChecked(true);
            ui->cbDoubleclick->setChecked(false);
            break;
        case 11:
            ui->rbClassic->setChecked(true);
            ui->cbDoubleclick->setChecked(true);
            break;
        }
    }
}

void OptionsDialog::setupInterfaceSettings(QSettings *settings)
{
    settings->beginGroup("Startup");
    const QString currentStyle = QApplication::style()->objectName();
    const QString appStyle = settings->value("AppStyle", currentStyle).toString();
    bool customStyle = settings->value("CustomStyle", false).toBool();
    const QStringList availableStyles = QStyleFactory::keys();
    ui->cbStyleList->addItems(availableStyles);
    if (availableStyles.contains(appStyle, Qt::CaseInsensitive)) {
        // use 'for' for select to be sure it's case insensitive
        int currentIndex = 0;
        for (const QString &currentStyleFF : availableStyles) {
            if (currentStyleFF.toLower() == appStyle.toLower()) {
                ui->cbStyleList->setCurrentIndex(currentIndex);
            }
            currentIndex++;
        }
    }
    else {
        if (availableStyles.contains(currentStyle, Qt::CaseInsensitive)) {
            int currentIndex = 0;
            for (const QString &currentStyleFF : availableStyles) {
                if (currentStyleFF.toLower() == currentStyle.toLower()) {
                    ui->cbStyleList->setCurrentIndex(currentIndex);
                }
                currentIndex++;
            }
        }
    }
    ui->cbDefaultStyle->setChecked(!customStyle);
    ui->cbStyleList->setEnabled(customStyle);
    const QFont currentFont = QApplication::font();
    const QFont appFont = qvariant_cast<QFont>(settings->value("AppFont", currentFont));
    bool customFont = settings->value("CustomFont", false).toBool();
    ui->cbDefaultFont->setChecked(!customFont);
    ui->cbFont->setEnabled(customFont);
    ui->cbFont->setCurrentFont(appFont);
    settings->endGroup();
}

void OptionsDialog::on_cmdOK_clicked()
{
    applySettings();
    close();
}

void OptionsDialog::applySettings()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Interface");
#if QT_VERSION >= 0x050000
    settings.setValue("Language", ui->cbLanguage->currentData());
    settings.setValue("AreaLanguage", ui->cbAreaLanguage->currentData());
#else
    settings->setValue("Language", ui->cbLanguage->itemData(ui->cbLanguage->currentIndex()));
    settings->setValue("AreaLanguage", ui->cbAreaLanguage->itemData(ui->cbAreaLanguage->currentIndex()));
#endif
#ifdef Q_OS_WIN
#if QT_VERSION >= 0x050200
    settings.setValue("NavigationBar", ui->cbSnapmaticNavigationBar->isChecked());
#endif
#else
    settings.setValue("NavigationBar", ui->cbSnapmaticNavigationBar->isChecked());
#endif
    settings.endGroup();

    settings.beginGroup("Profile");
    int newContentMode = 20;
    if (ui->rbModern->isChecked()) {
        newContentMode = 20;
    }
    else if (ui->rbClassic->isChecked()) {
        newContentMode = 10;
    }
    if (ui->cbDoubleclick->isChecked()) {
        newContentMode++;
    }
    settings.setValue("ContentMode", newContentMode);
#if QT_VERSION >= 0x050000
    settings.setValue("Default", ui->cbProfiles->currentData());
#else
    settings->setValue("Default", ui->cbProfiles->itemData(ui->cbProfiles->currentIndex()));
#endif
    settings.endGroup();

    const bool forceCustomFolder = ui->cbForceCustomFolder->isChecked();
    const bool forceCustomFolder_RDR2 = ui->cbForceCustomFolder_RDR2->isChecked();
    settings.beginGroup("GameDirectory");
    settings.beginGroup("GTA V");
    settings.setValue("Directory", ui->txtFolder->text());
    settings.setValue("ForceCustom", forceCustomFolder);
    settings.endGroup();
    settings.beginGroup("RDR 2");
    settings.setValue("Directory", ui->txtFolder_RDR2->text());
    settings.setValue("ForceCustom", forceCustomFolder_RDR2);
    settings.endGroup();
    settings.endGroup();

    const bool defaultStyle = ui->cbDefaultStyle->isChecked();
    settings.beginGroup("Startup");
    if (!defaultStyle) {
        QString newStyle = ui->cbStyleList->currentText();
        settings.setValue("CustomStyle", true);
        settings.setValue("AppStyle", newStyle);
        QApplication::setStyle(QStyleFactory::create(newStyle));
    }
    else {
        settings.setValue("CustomStyle", false);
    }
    const bool defaultFont = ui->cbDefaultFont->isChecked();
    if (!defaultFont) {
        QFont newFont = ui->cbFont->currentFont();
        settings.setValue("CustomFont", true);
        settings.setValue("AppFont", newFont);
        QApplication::setFont(newFont);
    }
    else {
        settings.setValue("CustomFont", false);
    }
    settings.endGroup();

#ifdef GTA5SYNC_TELEMETRY
    settings.beginGroup("Telemetry");
    settings.setValue("PushAppConf", ui->cbAppConfigStats->isChecked());
    settings.setValue("PushUsageData", ui->cbUsageData->isChecked());
    if (!Telemetry->isStateForced()) { settings.setValue("IsEnabled", ui->cbParticipateStats->isChecked()); }
    settings.endGroup();
    Telemetry->refresh();
    Telemetry->work();
    if (ui->cbUsageData->isChecked() && Telemetry->canPush()) {
        QJsonDocument jsonDocument;
        QJsonObject jsonObject;
        jsonObject["Type"] = "SettingsUpdated";
#if QT_VERSION >= 0x060000
        jsonObject["UpdateTime"] = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
#else
        jsonObject["UpdateTime"] = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#endif
        jsonDocument.setObject(jsonObject);
        Telemetry->push(TelemetryCategory::PersonalData, jsonDocument);
    }
#endif

#if QT_VERSION >= 0x050000
    bool languageChanged = ui->cbLanguage->currentData().toString() != currentLanguage;
    bool languageAreaChanged = ui->cbAreaLanguage->currentData().toString() != currentAreaLanguage;
#else
    bool languageChanged = ui->cbLanguage->itemData(ui->cbLanguage->currentIndex()).toString() != currentLanguage;
    bool languageAreaChanged = ui->cbAreaLanguage->itemData(ui->cbLanguage->currentIndex()).toString() != currentAreaLanguage;
#endif
    if (languageChanged) {
        Translator->unloadTranslation(qApp);
        Translator->initUserLanguage();
        Translator->loadTranslation(qApp);
    }
    else if (languageAreaChanged) {
        Translator->initUserLanguage();
    }

    settings.sync();
    emit settingsApplied(newContentMode, languageChanged);

    if ((forceCustomFolder && ui->txtFolder->text() != currentCFolder) || (forceCustomFolder != currentFFolder && forceCustomFolder)) {
        QMessageBox::information(this, tr("%1", "%1").arg(GTA5SYNC_APPSTR), tr("The new Custom Folder will initialise after you restart %1.").arg(GTA5SYNC_APPSTR));
    }
}

void OptionsDialog::setupDefaultProfile(QSettings *settings)
{
    settings->beginGroup("Profile");
    defaultProfile = settings->value("Default", QString()).toString();
    settings->endGroup();

    QString cbNoneStr = tr("No Profile", "No Profile, as default");
    ui->cbProfiles->addItem(cbNoneStr, QString());
}

void OptionsDialog::commitProfiles(const QStringList &profiles)
{
    for (const QString &profile : profiles) {
        ui->cbProfiles->addItem(tr("Profile: %1").arg(profile), profile);
        if (defaultProfile == profile) {
#if QT_VERSION >= 0x050000
            ui->cbProfiles->setCurrentText(tr("Profile: %1").arg(profile));
#else
            int indexOfProfile = ui->cbProfiles->findText(tr("Profile: %1").arg(profile));
            ui->cbProfiles->setCurrentIndex(indexOfProfile);
#endif
        }
    }
}

void OptionsDialog::setupStatisticsSettings(QSettings *settings)
{
#ifdef GTA5SYNC_TELEMETRY
    ui->cbParticipateStats->setText(tr("Participate in %1 User Statistics").arg(GTA5SYNC_APPSTR));
    ui->labUserStats->setText(QString("<a href=\"%2\">%1</a>").arg(tr("View %1 User Statistics Online").arg(GTA5SYNC_APPSTR), TelemetryClass::getWebURL().toString()));

    settings->beginGroup("Telemetry");
    ui->cbParticipateStats->setChecked(Telemetry->isEnabled());
    ui->cbAppConfigStats->setChecked(settings->value("PushAppConf", false).toBool());
    ui->cbUsageData->setChecked(settings->value("PushUsageData", false).toBool());
    settings->endGroup();

    if (Telemetry->isStateForced()) {
        ui->cbParticipateStats->setEnabled(false);
    }

    if (Telemetry->isRegistered()) {
        ui->labParticipationID->setText(tr("Participation ID: %1").arg(Telemetry->getRegisteredID()));
    }
    else {
        ui->labParticipationID->setText(tr("Participation ID: %1").arg(tr("Not registered")));
        ui->cmdCopyStatsID->setVisible(false);
    }
#else
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabStats));
#endif
}

void OptionsDialog::setupWindowsGameSettings()
{
#ifdef GTA5SYNC_GAME
    GameVersion gameVersion = AppEnv::getGameVersion();
#ifdef Q_OS_WIN
    if (gameVersion != GameVersion::NoVersion) {
        if (gameVersion == GameVersion::SocialClubVersion) {
            ui->gbSteam->setDisabled(true);
            ui->labSocialClubFound->setText(tr("Found: %1").arg(QString("<span style=\"color: green\">%1</span>").arg(tr("Yes"))));
            ui->labSteamFound->setText(tr("Found: %1").arg(QString("<span style=\"color: red\">%1</span>").arg(tr("No"))));
            if (AppEnv::getGameLanguage(GameVersion::SocialClubVersion) != GameLanguage::Undefined) {
                ui->labSocialClubLanguage->setText(tr("Language: %1").arg(QLocale(AppEnv::gameLanguageToString(AppEnv::getGameLanguage(GameVersion::SocialClubVersion))).nativeLanguageName()));
            }
            else {
                ui->labSocialClubLanguage->setText(tr("Language: %1").arg(tr("OS defined")));
            }
            ui->labSteamLanguage->setVisible(false);
        }
        else if (gameVersion == GameVersion::SteamVersion) {
            ui->gbSocialClub->setDisabled(true);
            ui->labSocialClubFound->setText(tr("Found: %1").arg(QString("<span style=\"color: red\">%1</span>").arg(tr("No"))));
            ui->labSteamFound->setText(tr("Found: %1").arg(QString("<span style=\"color: green\">%1</span>").arg(tr("Yes"))));
            ui->labSocialClubLanguage->setVisible(false);
            if (AppEnv::getGameLanguage(GameVersion::SteamVersion) != GameLanguage::Undefined) {
                ui->labSteamLanguage->setText(tr("Language: %1").arg(QLocale(AppEnv::gameLanguageToString(AppEnv::getGameLanguage(GameVersion::SteamVersion))).nativeLanguageName()));
            }
            else {
                ui->labSteamLanguage->setText(tr("Language: %1").arg(tr("Steam defined")));
            }
        }
        else {
            ui->labSocialClubFound->setText(tr("Found: %1").arg(QString("<span style=\"color: green\">%1</span>").arg(tr("Yes"))));
            ui->labSteamFound->setText(tr("Found: %1").arg(QString("<span style=\"color: green\">%1</span>").arg(tr("Yes"))));
            if (AppEnv::getGameLanguage(GameVersion::SocialClubVersion) != GameLanguage::Undefined) {
                ui->labSocialClubLanguage->setText(tr("Language: %1").arg(QLocale(AppEnv::gameLanguageToString(AppEnv::getGameLanguage(GameVersion::SocialClubVersion))).nativeLanguageName()));
            }
            else {
                ui->labSocialClubLanguage->setText(tr("Language: %1").arg(tr("OS defined")));
            }
            if (AppEnv::getGameLanguage(GameVersion::SteamVersion) != GameLanguage::Undefined) {
                ui->labSteamLanguage->setText(tr("Language: %1").arg(QLocale(AppEnv::gameLanguageToString(AppEnv::getGameLanguage(GameVersion::SteamVersion))).nativeLanguageName()));
            }
            else {
                ui->labSteamLanguage->setText(tr("Language: %1").arg(tr("Steam defined")));
            }
        }
    }
    else {
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabGame));
    }
#else
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabGame));
#endif
#else
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabGame));
#endif
}

void OptionsDialog::setupCustomGameFolder(QSettings *settings)
{
    bool ok_GTAV, ok_RDR2;
    const QString defaultGameFolder = AppEnv::getGTAVFolder(&ok_GTAV);
    const QString defaultGameFolderR = AppEnv::getRDR2Folder(&ok_RDR2);
    settings->beginGroup("dir");
    currentCFolder = settings->value("dir", QString()).toString();
    currentFFolder = settings->value("force", false).toBool();
    settings->endGroup();
    settings->beginGroup("GameDirectory");
    settings->beginGroup("GTA V");
    currentCFolder = settings->value("Directory", currentCFolder).toString();
    currentFFolder = settings->value("ForceCustom", currentFFolder).toBool();
    settings->endGroup();
    settings->beginGroup("RDR 2");
    currentCFolderR = settings->value("Directory", QString()).toString();
    currentFFolderR = settings->value("ForceCustom", false).toBool();
    settings->endGroup();
    settings->endGroup();
    if (currentCFolder.isEmpty() && ok_GTAV)
        currentCFolder = defaultGameFolder;
    if (currentCFolderR.isEmpty() && ok_RDR2)
        currentCFolderR = defaultGameFolderR;
    ui->txtFolder->setText(currentCFolder);
    ui->cbForceCustomFolder->setChecked(currentFFolder);
    ui->txtFolder_RDR2->setText(currentCFolderR);
    ui->cbForceCustomFolder_RDR2->setChecked(currentFFolderR);
}

void OptionsDialog::setupSnapmaticPictureViewer(QSettings *settings)
{
#ifdef Q_OS_WIN
#if QT_VERSION >= 0x050200
    settings->beginGroup("Interface");
    ui->cbSnapmaticNavigationBar->setChecked(settings->value("NavigationBar", true).toBool());
    settings->endGroup();
#else
    ui->cbSnapmaticNavigationBar->setVisible(false);
    ui->gbSnapmaticPictureViewer->setVisible(false);
#endif
#else
    settings->beginGroup("Interface");
    ui->cbSnapmaticNavigationBar->setChecked(settings->value("NavigationBar", true).toBool());
    settings->endGroup();
#endif
}

void OptionsDialog::on_cmdExploreFolder_clicked()
{
    const QString GTAV_Folder = QFileDialog::getExistingDirectory(this, UserInterface::tr("Select GTA V Folder..."), StandardPaths::documentsLocation(), QFileDialog::ShowDirsOnly);
    if (!GTAV_Folder.isEmpty() && QDir(GTAV_Folder).exists()) {
        ui->txtFolder->setText(GTAV_Folder);
    }
}

void OptionsDialog::on_cbDefaultStyle_toggled(bool checked)
{
    ui->cbStyleList->setDisabled(checked);
    ui->labStyle->setDisabled(checked);
}

void OptionsDialog::on_cbDefaultFont_toggled(bool checked)
{
    ui->cbFont->setDisabled(checked);
    ui->cmdFont->setDisabled(checked);
    ui->labFont->setDisabled(checked);
}

void OptionsDialog::on_cmdCopyStatsID_clicked()
{
#ifdef GTA5SYNC_TELEMETRY
    QApplication::clipboard()->setText(Telemetry->getRegisteredID());
#endif
}

void OptionsDialog::on_cbFont_currentFontChanged(const QFont &font)
{
    ui->cbFont->setFont(font);
}

void OptionsDialog::on_cmdFont_clicked()
{
    bool ok;
    const QFont font = QFontDialog::getFont(&ok, ui->cbFont->currentFont(), this);
    if (ok) {
        ui->cbFont->setCurrentFont(font);
        ui->cbFont->setFont(font);
    }
}
