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
#include "DatabaseThread.h"
#include "PictureDialog.h"
#include <QMessageBox>
#include <QPixmap>
#include <QDebug>
#include <QFile>

SnapmaticWidget::SnapmaticWidget(ProfileDatabase *profileDB, DatabaseThread *threadDB, QWidget *parent) :
    QWidget(parent), profileDB(profileDB), threadDB(threadDB),
    ui(new Ui::SnapmaticWidget)
{
    ui->setupUi(this);
    picPath = "";
    picStr = "";
    smpic = 0;
}

SnapmaticWidget::~SnapmaticWidget()
{
    delete ui;
}

void SnapmaticWidget::setSnapmaticPicture(SnapmaticPicture *picture, QString picturePath)
{
    smpic = picture;
    picPath = picturePath;
    picStr = picture->getPictureStr();

    QPixmap SnapmaticPixmap = QPixmap::fromImage(picture->getPicture(), Qt::AutoColor);
    SnapmaticPixmap.scaled(ui->labPicture->width(), ui->labPicture->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labPicStr->setText(picStr);
    ui->labPicture->setPixmap(SnapmaticPixmap);
}

void SnapmaticWidget::on_cmdView_clicked()
{
    PictureDialog *picDialog = new PictureDialog(profileDB, this);
    picDialog->setWindowFlags(picDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
    picDialog->setSnapmaticPicture(smpic, true);
    picDialog->setModal(true);

    // be ready for playerName updated
    QObject::connect(threadDB, SIGNAL(playerNameUpdated()), picDialog, SLOT(on_playerNameUpdated()));

    // show picture dialog
    picDialog->show();
    picDialog->exec();
    picDialog->deleteLater();
    delete picDialog;
}

void SnapmaticWidget::on_cmdDelete_clicked()
{
    int uchoice = QMessageBox::question(this, tr("Delete picture"), tr("Are you sure to delete %1 from your Snapmatic pictures?").arg("\""+picStr+"\""), QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
    if (uchoice == QMessageBox::Yes)
    {
        if (!QFile::exists(picPath))
        {
            emit pictureDeleted();
        }
        else if(QFile::remove(picPath))
        {
            emit pictureDeleted();
        }
        else
        {
            QMessageBox::warning(this, tr("Delete picture"), tr("Failed at deleting %1 from your Snapmatic pictures").arg("\""+picStr+"\""));
        }
    }
}
