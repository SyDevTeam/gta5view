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

#include <QStringBuilder>
#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include "AppEnv.h"
#include "config.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    // Set Window Flags
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);

    // Build Strings
    QString appVersion = qApp->applicationVersion();
    QString buildType = tr(GTA5SYNC_BUILDTYPE);
    buildType.replace("_", " ");
    QString projectBuild = AppEnv::getBuildDateTime();
    QString buildStr = GTA5SYNC_BUILDSTRING;

    // Translator Comments
    //: Using specific library, example Using libmyfuck
    QString usingStr = tr("Using %1 %2");
    //: Translated by translator, example Translated by Syping
    QString translatedByStr = tr("Translated by %1");
    //: Enter your name there
    QString translatedByVal = tr("NAME_OF_TRANSLATOR");
    //: Enter your proilfe there, example a GitHub profile, E-Mail with "mailto: afucker@sumfuck.com" or a webpage
    QString translatorProfile = tr("TRANSLATOR_PROFILE");
    QString additionalContent = "";
    if (translatedByVal != "NAME_OF_TRANSLATOR")
    {
        if (translatorProfile != "TRANSLATOR_PROFILE")
        {
            additionalContent += translatedByStr.arg(QString("<a href=\"%1\">%2</a>").arg(translatorProfile, translatedByVal));
        }
        else
        {
            additionalContent += translatedByStr.arg(translatedByVal);
        }
    }
#ifdef WITH_LIBJPEGTURBO // DONT USE IT FOR NOW
    bool additionalContentClip = false;
    if (!additionalContent.isEmpty())
    {
        additionalContentClip = true;
        additionalContent += " (";
    }
    additionalContent += usingStr.arg("libjpegturbo", WITH_LIBJPEGTURBO);
    if (additionalContentClip)
    {
        additionalContent += ")";
    }
#else
    Q_UNUSED(usingStr)
#endif

    // Project Description
#ifdef GTA5SYNC_ENABLED
    QString projectDes = tr("A project for viewing and sync Grand Theft Auto V Snapmatic<br/>\nPictures and Savegames");
#else
    QString projectDes = tr("A project for viewing Grand Theft Auto V Snapmatic<br/>\nPictures and Savegames");
#endif

    // Copyright Description
    QString copyrightDes1 = tr("Copyright &copy; <a href=\"%1\">%2</a> %3");
    copyrightDes1 = copyrightDes1.arg(GTA5SYNC_APPVENDORLINK, GTA5SYNC_APPVENDOR, GTA5SYNC_COPYRIGHT);
    QString copyrightDes2 = tr("%1 is licensed under <a href=\"https://www.gnu.org/licenses/gpl-3.0.html#content\">GNU GPLv3</a>");
    copyrightDes2 = copyrightDes2.arg(GTA5SYNC_APPSTR);
    QString copyrightDesA;
    if (!additionalContent.isEmpty())
    {
        copyrightDesA = copyrightDes1 % "<br/>" % additionalContent % "<br/>" % copyrightDes2;
    }
    else
    {
        copyrightDesA = copyrightDes1 % "<br/>" % copyrightDes2;
    }

    // Setup User Interface
    ui->setupUi(this);
    aboutStr = ui->labAbout->text();
    titleStr = this->windowTitle();
    ui->labAbout->setText(aboutStr.arg(GTA5SYNC_APPSTR, projectDes, appVersion % " (" % buildType % ")", projectBuild, buildStr, qVersion(), copyrightDesA));
    this->setWindowTitle(titleStr.arg(GTA5SYNC_APPSTR));

    // Set Icon for Close Button
    if (QIcon::hasThemeIcon("dialog-close"))
    {
        ui->cmdClose->setIcon(QIcon::fromTheme("dialog-close"));
    }
    else if (QIcon::hasThemeIcon("gtk-close"))
    {
        ui->cmdClose->setIcon(QIcon::fromTheme("gtk-close"));
    }

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    if (!additionalContent.isEmpty())
    {
        resize(375 * screenRatio, 270 * screenRatio);
    }
    else
    {
        resize(375 * screenRatio, 260 * screenRatio);
    }
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
