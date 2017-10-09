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
#include "AppEnv.h"
#include <QColorDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QDebug>
#include <QRgb>

// IMAGES VALUES
#define snapmaticResolutionW 960
#define snapmaticResolutionH 536
#define snapmaticAvatarResolution 470
#define snapmaticAvatarPlacementW 145
#define snapmaticAvatarPlacementH 66

ImportDialog::ImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);
    importAgreed = false;
    insideAvatarZone = false;
    avatarAreaImage = QImage(":/img/avatarareaimport.png");
    selectedColour = QColor::fromRgb(0, 0, 0, 255);

    if (QIcon::hasThemeIcon("dialog-ok"))
    {
        ui->cmdOK->setIcon(QIcon::fromTheme("dialog-ok"));
    }
    if (QIcon::hasThemeIcon("dialog-cancel"))
    {
        ui->cmdCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
    }

    ui->cbIgnore->setChecked(false);
    ui->labColour->setText(tr("Background Colour: <span style=\"color: %1\">%1</span>").arg(selectedColour.name()));

    qreal screenRatio = AppEnv::screenRatio();
    snapmaticResolutionLW = 430 * screenRatio;
    snapmaticResolutionLH = 240 * screenRatio;
    setMinimumSize(430 * screenRatio, 380 * screenRatio);
    setMaximumSize(430 * screenRatio, 380 * screenRatio);
    setFixedSize(430 * screenRatio, 380 * screenRatio);
    ui->vlButtom->setSpacing(6 * screenRatio);
    ui->vlButtom->setContentsMargins(9 * screenRatio, 6 * screenRatio, 9 * screenRatio, 9 * screenRatio);
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

void ImportDialog::processImage()
{
    QImage snapmaticImage = workImage;
    QPixmap snapmaticPixmap(snapmaticResolutionW, snapmaticResolutionH);
    snapmaticPixmap.fill(selectedColour);
    QPainter snapmaticPainter(&snapmaticPixmap);
    if (insideAvatarZone)
    {
        // Avatar mode
        int diffWidth = 0;
        int diffHeight = 0;
        if (!ui->cbIgnore->isChecked())
        {
            snapmaticImage = snapmaticImage.scaled(snapmaticAvatarResolution, snapmaticAvatarResolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (snapmaticImage.width() > snapmaticImage.height())
            {
                diffHeight = snapmaticAvatarResolution - snapmaticImage.height();
                diffHeight = diffHeight / 2;
            }
            else if (snapmaticImage.width() < snapmaticImage.height())
            {
                diffWidth = snapmaticAvatarResolution - snapmaticImage.width();
                diffWidth = diffWidth / 2;
            }
        }
        else
        {
            snapmaticImage = snapmaticImage.scaled(snapmaticAvatarResolution, snapmaticAvatarResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        snapmaticPainter.drawImage(snapmaticAvatarPlacementW + diffWidth, snapmaticAvatarPlacementH + diffHeight, snapmaticImage);
        imageTitle = "Custom Avatar";
    }
    else
    {
        // Picture mode
        int diffWidth = 0;
        int diffHeight = 0;
        if (!ui->cbIgnore->isChecked())
        {
            snapmaticImage = snapmaticImage.scaled(snapmaticResolutionW, snapmaticResolutionH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (snapmaticImage.width() != snapmaticResolutionW)
            {
                diffWidth = snapmaticResolutionW - snapmaticImage.width();
                diffWidth = diffWidth / 2;
            }
            else if (snapmaticImage.height() != snapmaticResolutionH)
            {
                diffHeight = snapmaticResolutionH - snapmaticImage.height();
                diffHeight = diffHeight / 2;
            }
        }
        else
        {
            snapmaticImage = snapmaticImage.scaled(snapmaticResolutionW, snapmaticResolutionH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        snapmaticPainter.drawImage(0 + diffWidth, 0 + diffHeight, snapmaticImage);
        imageTitle = "Custom Picture";
    }
    snapmaticPainter.end();
    newImage = snapmaticPixmap.toImage();
    ui->labPicture->setPixmap(snapmaticPixmap.scaled(snapmaticResolutionLW, snapmaticResolutionLH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
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
        insideAvatarZone = true;
        ui->cbAvatar->setChecked(true);
    }
    processImage();
}

bool ImportDialog::isImportAgreed()
{
    return importAgreed;
}

QString ImportDialog::getImageTitle()
{
    return imageTitle;
}

void ImportDialog::on_cbIgnore_toggled(bool checked)
{
    Q_UNUSED(checked)
    processImage();
}

void ImportDialog::on_cbAvatar_toggled(bool checked)
{
    if (workImage.width() == workImage.height() && !checked)
    {
        if (QMessageBox::No == QMessageBox::warning(this, tr("Snapmatic Avatar Zone"), tr("Are you sure to use a square image outside of the Avatar Zone?\nWhen you want to use it as Avatar the image will be detached!"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
        {
            ui->cbAvatar->setChecked(true);
            insideAvatarZone = true;
            return;
        }
    }
    insideAvatarZone = ui->cbAvatar->isChecked();
    processImage();
}

void ImportDialog::on_cmdCancel_clicked()
{
    close();
}

void ImportDialog::on_cmdOK_clicked()
{
    importAgreed = true;
    close();
}

void ImportDialog::on_labPicture_labelPainted()
{
    if (insideAvatarZone)
    {
        QImage avatarAreaFinalImage(avatarAreaImage);
        if (selectedColour.lightness() > 127)
        {
            avatarAreaFinalImage.setColor(1, qRgb(0, 0, 0));
        }
        QPainter labelPainter(ui->labPicture);
        labelPainter.drawImage(0, 0, avatarAreaFinalImage.scaled(snapmaticResolutionLW, snapmaticResolutionLH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        labelPainter.end();
    }
}

void ImportDialog::on_cmdColourChange_clicked()
{
    QColor newSelectedColour = QColorDialog::getColor(selectedColour, this, tr("Select Colour..."));
    if (newSelectedColour.isValid())
    {
        selectedColour = newSelectedColour;
        ui->labColour->setText(tr("Background Colour: <span style=\"color: %1\">%1</span>").arg(selectedColour.name()));
        processImage();
    }
}
