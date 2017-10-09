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

#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"
#include "TranslationClass.h"
#include "StandardPaths.h"
#include "UserInterface.h"
#include "AppEnv.h"
#include "config.h"
#include <QStringBuilder>
#include <QDesktopWidget>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QLocale>
#include <QString>
#include <QDebug>
#include <QList>
#include <QDir>

OptionsDialog::OptionsDialog(ProfileDatabase *profileDB, QWidget *parent) :
    QDialog(parent), profileDB(profileDB),
    ui(new Ui::OptionsDialog)
{
    // Set Window Flags
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);

    // Setup User Interface
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    ui->labPicCustomRes->setVisible(false);

    QRect desktopResolution = qApp->desktop()->screenGeometry(parent);
    int desktopSizeWidth = desktopResolution.width();
    int desktopSizeHeight = desktopResolution.height();
    aspectRatio = Qt::KeepAspectRatio;
    defExportSize = QSize(960, 536);
    cusExportSize = defExportSize;
    defaultQuality = 100;
    customQuality = 100;
    contentMode = 0;
    settings = new QSettings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);

    percentString = ui->labPicQuality->text();
    ui->labPicQuality->setText(percentString.arg(QString::number(defaultQuality)));
    ui->rbPicDesktopRes->setText(ui->rbPicDesktopRes->text().arg(QString::number(desktopSizeWidth), QString::number(desktopSizeHeight)));
    ui->rbPicDefaultRes->setText(ui->rbPicDefaultRes->text().arg(QString::number(defExportSize.width()), QString::number(defExportSize.height())));

    if (QIcon::hasThemeIcon("dialog-ok"))
    {
        ui->cmdOK->setIcon(QIcon::fromTheme("dialog-ok"));
    }
    if (QIcon::hasThemeIcon("dialog-cancel"))
    {
        ui->cmdCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
    }

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    resize(435 * screenRatio, 405 * screenRatio);

    setupTreeWidget();
    setupLanguageBox();
    setupRadioButtons();
    setupDefaultProfile();
    setupPictureSettings();
    setupCustomGTAFolder();
    setupSnapmaticPictureViewer();

#ifdef GTA5SYNC_DISABLED
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabSync));
#endif

    this->setWindowTitle(windowTitle().arg(GTA5SYNC_APPSTR));
}

OptionsDialog::~OptionsDialog()
{
    delete settings;
    foreach(QTreeWidgetItem *playerItem, playerItems)
    {
        delete playerItem;
    }
    delete ui;
}

void OptionsDialog::setupTreeWidget()
{
    foreach(const QString &playerIDStr, profileDB->getPlayers())
    {
        bool ok;
        int playerID = playerIDStr.toInt(&ok);
        if (ok)
        {
            QString playerName = profileDB->getPlayerName(playerID);

            QStringList playerTreeViewList;
            playerTreeViewList << playerIDStr;
            playerTreeViewList << playerName;

            QTreeWidgetItem *playerItem = new QTreeWidgetItem(playerTreeViewList);
            ui->twPlayers->addTopLevelItem(playerItem);
            playerItems += playerItem;
        }
    }
    ui->twPlayers->sortItems(1, Qt::AscendingOrder);
}

void OptionsDialog::setupLanguageBox()
{
    settings->beginGroup("Interface");
    currentLanguage = settings->value("Language","System").toString();
    settings->endGroup();

    QString cbSysStr = tr("%1 (Next Closest Language)", "First language a person can talk with a different person/application. \"Native\" or \"Not Native\".").arg(tr("System",
                                                                                                                                                                      "System in context of System default"));
    ui->cbLanguage->addItem(cbSysStr, "System");

    QStringList availableLanguages;
    availableLanguages << QString("en_GB");
#ifndef GTA5SYNC_QCONF
    availableLanguages << TCInstance->listTranslations(AppEnv::getExLangFolder());
#endif
    availableLanguages << TCInstance->listTranslations(AppEnv::getInLangFolder());
    availableLanguages.removeDuplicates();
    availableLanguages.sort();

    foreach(const QString &lang, availableLanguages)
    {
        QLocale langLocale(lang);
        QString cbLangStr = langLocale.nativeLanguageName() % " (" % langLocale.nativeCountryName() % ") [" % lang % "]";

        QString langIconStr = "flag-" % TranslationClass::getCountryCode(langLocale);

        ui->cbLanguage->addItem(QIcon::fromTheme(langIconStr), cbLangStr, lang);
        if (currentLanguage == lang)
        {
#if QT_VERSION >= 0x050000
            ui->cbLanguage->setCurrentText(cbLangStr);
#else
            int indexOfLang = ui->cbLanguage->findText(cbLangStr);
            ui->cbLanguage->setCurrentIndex(indexOfLang);
#endif
        }
    }
}

void OptionsDialog::setupRadioButtons()
{
    bool contentModeOk;
    settings->beginGroup("Profile");
    contentMode = settings->value("ContentMode", 0).toInt(&contentModeOk);
    settings->endGroup();

    if (contentModeOk)
    {
        if (contentMode == 0)
        {
            ui->rbOpenWithSC->setChecked(true);
        }
        else if (contentMode == 1)
        {
            ui->rbOpenWithDC->setChecked(true);
        }
        else if (contentMode == 2)
        {
            ui->rbSelectWithSC->setChecked(true);
        }
    }
}

void OptionsDialog::on_cmdOK_clicked()
{
    applySettings();
    close();
}

void OptionsDialog::applySettings()
{
    settings->beginGroup("Interface");
#if QT_VERSION >= 0x050000
    settings->setValue("Language", ui->cbLanguage->currentData());
#else
    settings->setValue("Language", ui->cbLanguage->itemData(ui->cbLanguage->currentIndex()));
#endif
#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
    settings->setValue("NavigationBar", ui->cbSnapmaticNavigationBar->isChecked());
#endif
#endif
    settings->endGroup();

    settings->beginGroup("Profile");
    int newContentMode = 0;
    if (ui->rbOpenWithSC->isChecked())
    {
        newContentMode = 0;
    }
    else if (ui->rbOpenWithDC->isChecked())
    {
        newContentMode = 1;
    }
    else if (ui->rbSelectWithSC->isChecked())
    {
        newContentMode = 2;
    }
    settings->setValue("ContentMode", newContentMode);
#if QT_VERSION >= 0x050000
    settings->setValue("Default", ui->cbProfiles->currentData());
#else
    settings->setValue("Default", ui->cbProfiles->itemData(ui->cbProfiles->currentIndex()));
#endif
    settings->endGroup();

    settings->beginGroup("Pictures");
    if (ui->cbPicCustomQuality->isChecked())
    {
        settings->setValue("CustomQuality", ui->hsPicQuality->value());
    }
    settings->setValue("CustomQualityEnabled", ui->cbPicCustomQuality->isChecked());
    QString sizeMode = "Default";
    if (ui->rbPicDesktopRes->isChecked())
    {
        sizeMode = "Desktop";
    }
    else if (ui->rbPicCustomRes->isChecked())
    {
        sizeMode = "Custom";
        settings->setValue("CustomSize", QSize(ui->sbPicExportWidth->value(), ui->sbPicExportHeight->value()));
    }
    settings->setValue("ExportSizeMode", sizeMode);
    settings->setValue("AspectRatio", aspectRatio);
    settings->endGroup();

    bool forceCustomFolder = ui->cbForceCustomFolder->isChecked();
    settings->beginGroup("dir");
    settings->setValue("dir", ui->txtFolder->text());
    settings->setValue("force", forceCustomFolder);
    settings->endGroup();

#if QT_VERSION >= 0x050000
    bool languageChanged = ui->cbLanguage->currentData().toString() != currentLanguage;
#else
    bool languageChanged = ui->cbLanguage->itemData(ui->cbLanguage->currentIndex()).toString() != currentLanguage;
#endif
    if (languageChanged)
    {
        TCInstance->unloadTranslation(qApp);
        TCInstance->initUserLanguage();
        TCInstance->loadTranslation(qApp);
    }

#if QT_VERSION >= 0x050000
    emit settingsApplied(newContentMode, ui->cbLanguage->currentData().toString());
#else
    emit settingsApplied(newContentMode, ui->cbLanguage->itemData(ui->cbLanguage->currentIndex()).toString());
#endif

    if ((forceCustomFolder && ui->txtFolder->text() != currentCFolder) || (forceCustomFolder != currentFFolder && forceCustomFolder))
    {
        QMessageBox::information(this, tr("%1", "%1").arg(GTA5SYNC_APPSTR), tr("The new Custom Folder will initialise after you restart %1.").arg(GTA5SYNC_APPSTR));
    }
}

void OptionsDialog::setupDefaultProfile()
{
    settings->beginGroup("Profile");
    defaultProfile = settings->value("Default", "").toString();
    settings->endGroup();

    QString cbNoneStr = tr("No Profile", "No Profile, as default");
    ui->cbProfiles->addItem(cbNoneStr, "");
}

void OptionsDialog::commitProfiles(QStringList profiles)
{
    foreach(const QString &profile, profiles)
    {
        ui->cbProfiles->addItem(tr("Profile: %1").arg(profile), profile);
        if (defaultProfile == profile)
        {
#if QT_VERSION >= 0x050000
            ui->cbProfiles->setCurrentText(tr("Profile: %1").arg(profile));
#else
            int indexOfProfile = ui->cbProfiles->findText(tr("Profile: %1").arg(profile));
            ui->cbProfiles->setCurrentIndex(indexOfProfile);
#endif
        }
    }
}

void OptionsDialog::on_rbPicCustomRes_toggled(bool checked)
{
    ui->labPicCustomRes->setEnabled(checked);
    ui->sbPicExportWidth->setEnabled(checked);
    ui->sbPicExportHeight->setEnabled(checked);
    ui->labPicXDescription->setEnabled(checked);
}

void OptionsDialog::on_cbPicCustomQuality_toggled(bool checked)
{
    ui->hsPicQuality->setEnabled(checked);
    ui->labPicQuality->setEnabled(checked);
    ui->labPicQualityDescription->setEnabled(checked);
}

void OptionsDialog::on_hsPicQuality_valueChanged(int value)
{
    customQuality = value;
    ui->labPicQuality->setText(percentString.arg(QString::number(value)));
}

void OptionsDialog::setupPictureSettings()
{
    settings->beginGroup("Pictures");

    // Quality Settings
    customQuality = settings->value("CustomQuality", defaultQuality).toInt();
    if (customQuality < 1 || customQuality > 100)
    {
        customQuality = 100;
    }
    ui->hsPicQuality->setValue(customQuality);
    ui->cbPicCustomQuality->setChecked(settings->value("CustomQualityEnabled", false).toBool());

    // Size Settings
    cusExportSize = settings->value("CustomSize", defExportSize).toSize();
    if (cusExportSize.width() > 3840)
    {
        cusExportSize.setWidth(3840);
    }
    else if (cusExportSize.height() > 2160)
    {
        cusExportSize.setHeight(2160);
    }
    if (cusExportSize.width() < 1)
    {
        cusExportSize.setWidth(1);
    }
    else if (cusExportSize.height() < 1)
    {
        cusExportSize.setHeight(1);
    }
    ui->sbPicExportWidth->setValue(cusExportSize.width());
    ui->sbPicExportHeight->setValue(cusExportSize.height());

    QString sizeMode = settings->value("ExportSizeMode", "Default").toString();
    if (sizeMode == "Desktop")
    {
        ui->rbPicDesktopRes->setChecked(true);
    }
    else if (sizeMode == "Custom")
    {
        ui->rbPicCustomRes->setChecked(true);
    }
    else
    {
        ui->rbPicDefaultRes->setChecked(true);
    }

    aspectRatio = (Qt::AspectRatioMode)settings->value("AspectRatio", Qt::KeepAspectRatio).toInt();
    if (aspectRatio == Qt::IgnoreAspectRatio)
    {
        ui->cbIgnoreAspectRatio->setChecked(true);
    }

    settings->endGroup();
}

void OptionsDialog::on_cbIgnoreAspectRatio_toggled(bool checked)
{
    if (checked)
    {
        aspectRatio = Qt::IgnoreAspectRatio;
    }
    else
    {
        aspectRatio = Qt::KeepAspectRatio;
    }
}

void OptionsDialog::setupCustomGTAFolder()
{
    bool ok;
    QString defaultGameFolder = AppEnv::getGameFolder(&ok);
    settings->beginGroup("dir");
    currentCFolder = settings->value("dir", "").toString();
    currentFFolder = settings->value("force", false).toBool();
    if (currentCFolder == "" && ok)
    {
        currentCFolder = defaultGameFolder;
    }
    ui->txtFolder->setText(currentCFolder);
    ui->cbForceCustomFolder->setChecked(currentFFolder);
    settings->endGroup();
}

void OptionsDialog::setupSnapmaticPictureViewer()
{
#ifdef GTA5SYNC_WIN
#if QT_VERSION >= 0x050200
    settings->beginGroup("Interface");
    ui->cbSnapmaticNavigationBar->setChecked(settings->value("NavigationBar", false).toBool());
    settings->endGroup();
#else
    ui->cbSnapmaticNavigationBar->setVisible(false);
    ui->gbSnapmaticPictureViewer->setVisible(false);
#endif
#else
    ui->cbSnapmaticNavigationBar->setVisible(false);
    ui->gbSnapmaticPictureViewer->setVisible(false);
#endif
}

void OptionsDialog::on_cmdExploreFolder_clicked()
{
    QString GTAV_Folder = QFileDialog::getExistingDirectory(this, UserInterface::tr("Select GTA V Folder..."), StandardPaths::documentsLocation(), QFileDialog::ShowDirsOnly);
    if (QFileInfo(GTAV_Folder).exists())
    {
        ui->txtFolder->setText(GTAV_Folder);
    }
}
