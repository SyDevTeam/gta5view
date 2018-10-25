/*****************************************************************************
* gta5view Grand Theft Auto V Profile Viewer
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

#include "TranslationClass.h"
#include "DonationDialog.h"
#include "config.h"
#include <QSettings>
#include <QDebug>

DonationDialog::DonationDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Donate"));
    setLayout(&layout);
    titleLabel.setText(tr("<h4>Hello, thank you for using %1!</h4>").arg(GTA5SYNC_APPSTR));
    layout.addWidget(&titleLabel);
    informationLabel.setText(tr("When you think %1 is useful for you, you should consider donate for support future development.").arg(GTA5SYNC_APPSTR));
    informationLabel.setWordWrap(true);
    layout.addWidget(&informationLabel);
    donateLabel.setText(QString("<a href=\"%1\"><img src=\":/img/donate.png\"></a>").arg(donateUrl()));
    donateLabel.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    donateLabel.setOpenExternalLinks(true);
    layout.addWidget(&donateLabel);
    layout.addLayout(&buttomLayout);
    showAgainBox.setChecked(true);
    showAgainBox.setText(tr("Show Again"));
    showAgainBox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttomLayout.addWidget(&showAgainBox);
    closeButton.setText(tr("&Close"));
    connect(&closeButton, SIGNAL(clicked()), this, SLOT(close()));
    buttomLayout.addWidget(&closeButton);
    resize(((double)sizeHint().width() * 1.5), sizeHint().height());
}

DonationDialog::~DonationDialog()
{
}

void DonationDialog::closeEvent(QCloseEvent *ev)
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Startup");
    settings.setValue("ShowDonation", showAgainBox.isChecked());
    settings.endGroup();
    ev->accept();
}

QString DonationDialog::donateUrl()
{
    QString donationUrl;
    QString currencyCode = QLocale::system().currencySymbol(QLocale::CurrencyIsoCode);
    if (currencyCode == "EUR")
    {
        donationUrl = QString("https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=%1&item_name=Donation+for+%2&item_number=%3+Version&currency_code=EUR").arg(QString(GTA5SYNC_DONATION_EMAIL).replace("/at/", "@"), QString(GTA5SYNC_APPSTR).replace(" ", "+"), QString(GTA5SYNC_BUILDCODE).replace(" ", "+"));
    }
    else if (currencyCode == "GBP")
    {
        donationUrl = QString("https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=%1&item_name=Donation+for+%2&item_number=%3+Version&currency_code=GBP").arg(QString(GTA5SYNC_DONATION_EMAIL).replace("/at/", "@"), QString(GTA5SYNC_APPSTR).replace(" ", "+"), QString(GTA5SYNC_BUILDCODE).replace(" ", "+"));
    }
    else
    {
        donationUrl = QString("https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=%1&item_name=Donation+for+%2&item_number=%3+Version&currency_code=USD").arg(QString(GTA5SYNC_DONATION_EMAIL).replace("/at/", "@"), QString(GTA5SYNC_APPSTR).replace(" ", "+"), QString(GTA5SYNC_BUILDCODE).replace(" ", "+"));
    }
    return donationUrl;
}
