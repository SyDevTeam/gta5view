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

#ifndef DONATIONDIALOG_H
#define DONATIONDIALOG_H

#include <QLabel>
#include <QDialog>
#include <QString>
#include <QCheckBox>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>

namespace Ui {
class DonationDialog;
}

class DonationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DonationDialog(QWidget *parent = nullptr);
    ~DonationDialog();

protected:
    void closeEvent(QCloseEvent *ev);

private:
    QString donateUrl();
    QVBoxLayout layout;
    QLabel titleLabel;
    QLabel informationLabel;
    QLabel donateLabel;
    QCheckBox showAgainBox;
    QHBoxLayout buttomLayout;
    QPushButton closeButton;
};

#endif // DONATIONDIALOG_H
