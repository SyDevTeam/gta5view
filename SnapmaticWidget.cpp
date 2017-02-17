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

#include "SnapmaticWidget.h"
#include "ui_SnapmaticWidget.h"
#include "SnapmaticPicture.h"
#include "SnapmaticEditor.h"
#include "DatabaseThread.h"
#include "PictureDialog.h"
#include "PictureExport.h"
#include "config.h"
#include <QMessageBox>
#include <QPixmap>
#include <QTimer>
#include <QDebug>
#include <QMenu>
#include <QFile>

SnapmaticWidget::SnapmaticWidget(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent) :
    ProfileWidget(parent), profileDB(profileDB), crewDB(crewDB), threadDB(threadDB),
    ui(new Ui::SnapmaticWidget)
{
    ui->setupUi(this);
    ui->cmdView->setVisible(false);
    ui->cmdCopy->setVisible(false);
    ui->cmdExport->setVisible(false);
    ui->cmdDelete->setVisible(false);
    ui->cbSelected->setVisible(false);

    QPalette palette;
    highlightBackColor = palette.highlight().color();
    highlightTextColor = palette.highlightedText().color();
    palette.setCurrentColorGroup(QPalette::Disabled);
    highlightHiddenColor = palette.text().color();

    picPath = "";
    picStr = "";
    smpic = 0;

    installEventFilter(this);
}

SnapmaticWidget::~SnapmaticWidget()
{
    delete ui;
}

bool SnapmaticWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == this)
    {
        if (ev->type() == QEvent::Enter)
        {
            setStyleSheet(QString("QFrame#SnapmaticFrame{background-color: rgb(%1, %2, %3)}QLabel#labPicStr{color: rgb(%4, %5, %6)}").arg(QString::number(highlightBackColor.red()), QString::number(highlightBackColor.green()), QString::number(highlightBackColor.blue()), QString::number(highlightTextColor.red()), QString::number(highlightTextColor.green()), QString::number(highlightTextColor.blue())));
            return true;
        }
        else if(ev->type() == QEvent::Leave)
        {
            setStyleSheet("");
            return true;
        }
    }
    return false;
}

void SnapmaticWidget::setSnapmaticPicture(SnapmaticPicture *picture)
{
    smpic = picture;
    picPath = picture->getPictureFilePath();
    picTitl = picture->getPictureTitl();
    picStr = picture->getPictureStr();

    QPixmap SnapmaticPixmap = QPixmap::fromImage(picture->getImage().scaled(ui->labPicture->width(), ui->labPicture->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), Qt::AutoColor);
    ui->labPicStr->setText(picStr + "\n" + picTitl + "");
    ui->labPicture->setPixmap(SnapmaticPixmap);

    picture->clearCache();

    adjustTextColor();
}

void SnapmaticWidget::on_cmdView_clicked()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Interface");
    bool navigationBar = settings.value("NavigationBar", false).toBool();
    settings.endGroup();

    PictureDialog *picDialog = new PictureDialog(profileDB, crewDB, this);
    picDialog->setWindowFlags(picDialog->windowFlags()^Qt::WindowContextHelpButtonHint);
    picDialog->setSnapmaticPicture(smpic, true);
    picDialog->setModal(true);

    // be ready for playerName updated
    QObject::connect(threadDB, SIGNAL(playerNameUpdated()), picDialog, SLOT(playerNameUpdated()));
    QObject::connect(picDialog, SIGNAL(nextPictureRequested()), this, SLOT(dialogNextPictureRequested()));
    QObject::connect(picDialog, SIGNAL(previousPictureRequested()), this, SLOT(dialogPreviousPictureRequested()));

    // add previous next buttons
    if (navigationBar) picDialog->addPreviousNextButtons();

    // show picture dialog
    picDialog->show();
    if (navigationBar) picDialog->stylizeDialog();
    //picDialog->adaptNewDialogSize();
    picDialog->setMinimumSize(picDialog->size());
    picDialog->setMaximumSize(picDialog->size());
    picDialog->exec();
    delete picDialog;
}

void SnapmaticWidget::on_cmdCopy_clicked()
{
    PictureExport::exportAsSnapmatic(this, smpic);
}

void SnapmaticWidget::on_cmdExport_clicked()
{
    PictureExport::exportAsPicture(this, smpic);
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
            ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
        }
    }
    else
    {
        if (getContentMode() == 0 && rect().contains(ev->pos()) && ev->button() == Qt::LeftButton)
        {
            on_cmdView_clicked();
        }
    }
}

void SnapmaticWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    ProfileWidget::mouseDoubleClickEvent(ev);

    if (!ui->cbSelected->isVisible() && getContentMode() == 1 && ev->button() == Qt::LeftButton)
    {
        on_cmdView_clicked();
    }
}

void SnapmaticWidget::setSelected(bool isSelected)
{
    ui->cbSelected->setChecked(isSelected);
}

void SnapmaticWidget::pictureSelected()
{
    setSelected(!ui->cbSelected->isChecked());
}

void SnapmaticWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    emit contextMenuTriggered(ev);
}

void SnapmaticWidget::dialogNextPictureRequested()
{
    emit nextPictureRequested((QWidget*)sender());
}

void SnapmaticWidget::dialogPreviousPictureRequested()
{
    emit previousPictureRequested((QWidget*)sender());
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

void SnapmaticWidget::adjustTextColor()
{
    if (isHidden())
    {
        ui->labPicStr->setStyleSheet(QString("QLabel{color: rgb(%1, %2, %3);}").arg(QString::number(highlightHiddenColor.red()), QString::number(highlightHiddenColor.green()), QString::number(highlightHiddenColor.blue())));
    }
    else
    {
        ui->labPicStr->setStyleSheet("");
    }
}

bool SnapmaticWidget::makePictureHidden()
{
    if (smpic->setPictureHidden())
    {
        picPath = smpic->getPictureFilePath();
        adjustTextColor();
        return true;
    }
    return false;
}

bool SnapmaticWidget::makePictureVisible()
{
    if (smpic->setPictureVisible())
    {
        picPath = smpic->getPictureFilePath();
        adjustTextColor();
        return true;
    }
    return false;
}

void SnapmaticWidget::makePictureHiddenSlot()
{
    makePictureHidden();
}

void SnapmaticWidget::makePictureVisibleSlot()
{
    makePictureVisible();
}

void SnapmaticWidget::editSnapmaticProperties()
{
    SnapmaticEditor *snapmaticEditor = new SnapmaticEditor(this);
    snapmaticEditor->setWindowFlags(snapmaticEditor->windowFlags()^Qt::WindowContextHelpButtonHint);
    snapmaticEditor->setSnapmaticPicture(smpic);
    snapmaticEditor->setModal(true);
    snapmaticEditor->exec();
    delete snapmaticEditor;
}

bool SnapmaticWidget::isSelected()
{
    return ui->cbSelected->isChecked();
}

bool SnapmaticWidget::isHidden()
{
    if (picPath.right(7) == ".hidden")
    {
        return true;
    }
    return false;
}

void SnapmaticWidget::setSelectionMode(bool selectionMode)
{
    ui->cbSelected->setVisible(selectionMode);
}

void SnapmaticWidget::selectAllWidgets()
{
    emit allWidgetsSelected();
}

void SnapmaticWidget::deselectAllWidgets()
{
    emit allWidgetsDeselected();
}

SnapmaticPicture* SnapmaticWidget::getPicture()
{
    return smpic;
}

QString SnapmaticWidget::getPicturePath()
{
    return picPath;
}

QString SnapmaticWidget::getWidgetType()
{
    return "SnapmaticWidget";
}
