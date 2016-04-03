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
#include "PictureExport.h"
#include "PictureCopy.h"
#include <QMessageBox>
#include <QPixmap>
#include <QTimer>
#include <QDebug>
#include <QMenu>
#include <QFile>

SnapmaticWidget::SnapmaticWidget(ProfileDatabase *profileDB, DatabaseThread *threadDB, QWidget *parent) :
    ProfileWidget(parent), profileDB(profileDB), threadDB(threadDB),
    ui(new Ui::SnapmaticWidget)
{
    ui->setupUi(this);
    ui->cmdView->setVisible(false);
    ui->cmdCopy->setVisible(false);
    ui->cmdExport->setVisible(false);
    ui->cmdDelete->setVisible(false);
    ui->cbSelected->setVisible(false);

    QPalette palette;
    QColor highlightBackColor = palette.highlight().color();
    QColor highlightTextColor = palette.highlightedText().color();
    setStyleSheet(QString("QFrame:hover#SnapmaticFrame{background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));

    clkIssued = 0;
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
    picDialog->setSnapmaticPicture(smpic, picPath, true);
    picDialog->setModal(true);

    // be ready for playerName updated
    QObject::connect(threadDB, SIGNAL(playerNameUpdated()), picDialog, SLOT(playerNameUpdated()));

    // show picture dialog
    picDialog->show();
    picDialog->exec();
    picDialog->deleteLater();
    delete picDialog;
}

void SnapmaticWidget::on_cmdCopy_clicked()
{
    PictureCopy::CopyPicture(this, picPath);
}

void SnapmaticWidget::on_cmdExport_clicked()
{
    PictureExport::ExportPicture(this, smpic);
}

void SnapmaticWidget::on_cmdDelete_clicked()
{
    int uchoice = QMessageBox::question(this, tr("Delete picture"), tr("Are you sure to delete %1 from your Snapmatic pictures?").arg("\""+picStr+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
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

void SnapmaticWidget::mousePressEvent(QMouseEvent *ev)
{
    ProfileWidget::mousePressEvent(ev);
}

void SnapmaticWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    ProfileWidget::mouseReleaseEvent(ev);
    if (ui->cbSelected->isVisible())
    {
        if (rect().contains(ev->pos()) && ev->button() == Qt::LeftButton)
        {
            clkIssued = false;
            //QTimer::singleShot(QApplication::doubleClickInterval(), this, SLOT(changeCheckedState()));
            ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
        }
    }
    else
    {
        if (rect().contains(ev->pos()) && ev->button() == Qt::LeftButton)
        {
            on_cmdView_clicked();
        }
    }
}

void SnapmaticWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    QWidget::mouseDoubleClickEvent(ev);

//  if (ev->button() == Qt::LeftButton)
//  {
//      clkIssued = true;
//      on_cmdView_clicked();
//  }
}

void SnapmaticWidget::changeCheckedState()
{
    if (!clkIssued)
    {
        ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
    }
}

void SnapmaticWidget::setSelected(bool isSelected)
{
    ui->cbSelected->setChecked(isSelected);
}

void SnapmaticWidget::pictureSelected()
{
    setSelected(true);
}

void SnapmaticWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu contextMenu(this);
    if (!ui->cbSelected->isVisible())
    {
        contextMenu.addAction(tr("Select"), this, SLOT(pictureSelected()));
        contextMenu.addSeparator();
    }
    contextMenu.addAction(tr("View picture"), this, SLOT(on_cmdView_clicked()));
    contextMenu.addAction(tr("Copy picture"), this, SLOT(on_cmdCopy_clicked()));
    contextMenu.addAction(tr("Export picture"), this, SLOT(on_cmdExport_clicked()));
    contextMenu.addAction(tr("Delete picture"), this, SLOT(on_cmdDelete_clicked()));
    contextMenu.exec(ev->globalPos());
    setStyleSheet(styleSheet()); // fix multi highlight bug
}

void SnapmaticWidget::on_cbSelected_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        emit widgetSelected();
    }
    else if (arg1 == Qt::Unchecked)
    {
        emit widgetDeselected();
    }
}

bool SnapmaticWidget::isSelected()
{
    return ui->cbSelected->isChecked();
}

void SnapmaticWidget::setSelectionMode(bool selectionMode)
{
    ui->cbSelected->setVisible(selectionMode);
}

SnapmaticPicture* SnapmaticWidget::getPicture()
{
    return smpic;
}
