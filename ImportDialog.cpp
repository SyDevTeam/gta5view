/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2017 Syping
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

#include "ImportDialog.h"
#include "ui_ImportDialog.h"
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QDebug>

ImportDialog::ImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);
    doImport = false;
    avatarAreaImage = QImage(":/img/avatarareaimport.png");

    if (QIcon::hasThemeIcon("dialog-ok"))
    {
        ui->cmdOK->setIcon(QIcon::fromTheme("dialog-ok"));
    }
    if (QIcon::hasThemeIcon("dialog-cancel"))
    {
        ui->cmdCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
    }

    ui->rbKeep->setChecked(true);
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

void ImportDialog::processImage()
{
    QImage snapmaticImage = workImage;
    QPixmap snapmaticPixmap(960, 536);
    snapmaticPixmap.fill(Qt::black);
    QPainter snapmaticPainter(&snapmaticPixmap);
    if (ui->cbAvatar->isChecked())
    {
        // Avatar mode
        int diffWidth = 0;
        int diffHeight = 0;
        if (ui->rbKeep->isChecked())
        {
            snapmaticImage = snapmaticImage.scaled(470, 470, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (snapmaticImage.width() > snapmaticImage.height())
            {
                diffHeight = 470 - snapmaticImage.height();
                diffHeight = diffHeight / 2;
            }
            else if (snapmaticImage.width() < snapmaticImage.height())
            {
                diffWidth = 470 - snapmaticImage.width();
                diffWidth = diffWidth / 2;
            }
        }
        else
        {
            snapmaticImage = snapmaticImage.scaled(470, 470, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        snapmaticPainter.drawImage(0, 0, avatarAreaImage);
        snapmaticPainter.drawImage(145 + diffWidth, 66 + diffHeight, snapmaticImage);
        imageTitle = "Custom Avatar";
    }
    else
    {
        // Picture mode
        int diffWidth = 0;
        int diffHeight = 0;
        if (ui->rbKeep->isChecked())
        {
            snapmaticImage = snapmaticImage.scaled(960, 536, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (snapmaticImage.width() != 960)
            {
                diffWidth = 960 - snapmaticImage.width();
                diffWidth = diffWidth / 2;
            }
            else if (snapmaticImage.height() != 536)
            {
                diffHeight = 536 - snapmaticImage.height();
                diffHeight = diffHeight / 2;
            }
        }
        else
        {
            snapmaticImage = snapmaticImage.scaled(960, 536, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        snapmaticPainter.drawImage(0 + diffWidth, 0 + diffHeight, snapmaticImage);
        imageTitle = "Custom Picture";
    }
    snapmaticPainter.end();
    newImage = snapmaticPixmap.toImage();
    ui->labPicture->setPixmap(snapmaticPixmap.scaled(430, 240, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

QImage ImportDialog::image()
{
    return newImage;
}

void ImportDialog::setImage(const QImage &image_)
{
    workImage = image_;
    if (workImage.width() == workImage.height())
    {
        ui->cbAvatar->setChecked(true);
    }
    processImage();
}

bool ImportDialog::isDoImport()
{
    return doImport;
}

QString ImportDialog::getImageTitle()
{
    return imageTitle;
}

void ImportDialog::on_rbIgnore_clicked()
{
    processImage();
}

void ImportDialog::on_rbKeep_clicked()
{
    processImage();
}

void ImportDialog::on_cbAvatar_clicked()
{
    processImage();
}

void ImportDialog::on_cmdCancel_clicked()
{
    close();
}

void ImportDialog::on_cmdOK_clicked()
{
    doImport = true;
    close();
}
