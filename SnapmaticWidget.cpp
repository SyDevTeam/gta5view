/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016 Syping
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

#include "SnapmaticWidget.h"
#include "ui_SnapmaticWidget.h"
#include "SnapmaticPicture.h"
#include "PictureDialog.h"
#include <QPixmap>

SnapmaticWidget::SnapmaticWidget(ProfileDatabase *profileDB, QWidget *parent) :
    QWidget(parent), profileDB(profileDB),
    ui(new Ui::SnapmaticWidget)
{
    ui->setupUi(this);
    picPath = "";
    smpic = 0;
}

SnapmaticWidget::~SnapmaticWidget()
{
    delete ui;
}

void SnapmaticWidget::setSnapmaticPicture(SnapmaticPicture *picture, QString picturePath)
{
    QPixmap SnapmaticPixmap = QPixmap::fromImage(picture->getPicture(), Qt::AutoColor);
    SnapmaticPixmap.scaled(ui->labPicture->width(), ui->labPicture->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labPicStr->setText(picture->getPictureStr());
    ui->labPicture->setPixmap(SnapmaticPixmap);
    smpic = picture;
    picPath = picturePath;
}

void SnapmaticWidget::on_cmdView_clicked()
{
    PictureDialog *picDialog = new PictureDialog(profileDB, this);
    picDialog->setWindowFlags(picDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
    picDialog->setSnapmaticPicture(smpic, true);
    picDialog->setModal(true);
    picDialog->show();
    picDialog->exec();
    picDialog->deleteLater();
}
