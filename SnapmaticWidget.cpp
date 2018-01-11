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
#include "ImageEditorDialog.h"
#include "MapLocationDialog.h"
#include "JsonEditorDialog.h"
#include "SnapmaticPicture.h"
#include "SnapmaticEditor.h"
#include "DatabaseThread.h"
#include "PictureDialog.h"
#include "PictureExport.h"
#include "StringParser.h"
#include "AppEnv.h"
#include "config.h"
#include <QStringBuilder>
#include <QMessageBox>
#include <QPixmap>
#include <QTimer>
#include <QDebug>
#include <QMenu>
#include <QFile>

SnapmaticWidget::SnapmaticWidget(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QString profileName, QWidget *parent) :
    ProfileWidget(parent), profileDB(profileDB), crewDB(crewDB), threadDB(threadDB), profileName(profileName),
    ui(new Ui::SnapmaticWidget)
{
    ui->setupUi(this);
    ui->cmdView->setVisible(false);
    ui->cmdCopy->setVisible(false);
    ui->cmdExport->setVisible(false);
    ui->cmdDelete->setVisible(false);
    ui->cbSelected->setVisible(false);

    QPalette palette;
    palette.setCurrentColorGroup(QPalette::Disabled);
    highlightHiddenColor = palette.text().color();

    ui->SnapmaticFrame->setMouseTracking(true);
    ui->labPicture->setMouseTracking(true);
    ui->labPicStr->setMouseTracking(true);
    ui->cbSelected->setMouseTracking(true);
    smpic = nullptr;
}

SnapmaticWidget::~SnapmaticWidget()
{
    delete ui;
}

void SnapmaticWidget::setSnapmaticPicture(SnapmaticPicture *picture)
{
    smpic = picture;
    QObject::connect(picture, SIGNAL(updated()), this, SLOT(snapmaticUpdated()));
    QObject::connect(picture, SIGNAL(customSignal(QString)), this, SLOT(customSignal(QString)));

    qreal screenRatio = AppEnv::screenRatio();
    ui->labPicture->setFixedSize(48 * screenRatio, 27 * screenRatio);

    QPixmap SnapmaticPixmap = QPixmap::fromImage(picture->getImage().scaled(ui->labPicture->width(), ui->labPicture->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), Qt::AutoColor);
    ui->labPicStr->setText(smpic->getPictureStr() % "\n" % smpic->getPictureTitl());
    ui->labPicture->setPixmap(SnapmaticPixmap);

    picture->clearCache();

    adjustTextColor();
}

void SnapmaticWidget::snapmaticUpdated()
{
    ui->labPicStr->setText(smpic->getPictureStr() % "\n" % smpic->getPictureTitl());
}

void SnapmaticWidget::customSignal(QString signal)
{
    if (signal == "PictureUpdated")
    {
        QPixmap SnapmaticPixmap = QPixmap::fromImage(smpic->getImage().scaled(ui->labPicture->width(), ui->labPicture->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), Qt::AutoColor);
        ui->labPicture->setPixmap(SnapmaticPixmap);
    }
}

void SnapmaticWidget::retranslate()
{
    smpic->updateStrings();
    ui->labPicStr->setText(smpic->getPictureStr() % "\n" % smpic->getPictureTitl());
}

void SnapmaticWidget::on_cmdView_clicked()
{
    QSettings settings(GTA5SYNC_APPVENDOR, GTA5SYNC_APPSTR);
    settings.beginGroup("Interface");
    bool navigationBar = settings.value("NavigationBar", false).toBool();
    settings.endGroup();

    PictureDialog *picDialog = new PictureDialog(profileDB, crewDB, profileName, this);
    picDialog->setSnapmaticPicture(smpic, true);
    picDialog->setModal(true);

    // be ready for crewName and playerName updated
    QObject::connect(threadDB, SIGNAL(crewNameUpdated()), picDialog, SLOT(crewNameUpdated()));
    QObject::connect(threadDB, SIGNAL(playerNameUpdated()), picDialog, SLOT(playerNameUpdated()));
    QObject::connect(picDialog, SIGNAL(nextPictureRequested()), this, SLOT(dialogNextPictureRequested()));
    QObject::connect(picDialog, SIGNAL(previousPictureRequested()), this, SLOT(dialogPreviousPictureRequested()));

    // add previous next buttons
    if (navigationBar) picDialog->addPreviousNextButtons();

    // show picture dialog
#ifdef Q_OS_ANDROID
    // Android ...
    picDialog->showMaximized();
#else
    picDialog->show();
    if (navigationBar) picDialog->stylizeDialog();
    //picDialog->adaptNewDialogSize();
    picDialog->setMinimumSize(picDialog->size());
    picDialog->setMaximumSize(picDialog->size());
#endif
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
    if (deletePicture()) emit pictureDeleted();
}

bool SnapmaticWidget::deletePicture()
{
    int uchoice = QMessageBox::question(this, tr("Delete picture"), tr("Are you sure to delete %1 from your Snapmatic pictures?").arg("\""+smpic->getPictureStr()+"\""), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (uchoice == QMessageBox::Yes)
    {
        if (smpic->deletePicFile())
        {
            return true;
        }
        else
        {
            QMessageBox::warning(this, tr("Delete picture"), tr("Failed at deleting %1 from your Snapmatic pictures").arg("\""+smpic->getPictureStr()+"\""));
        }
    }
    return false;
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
            if (ev->modifiers().testFlag(Qt::ShiftModifier))
            {
                ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
            }
            else
            {
                on_cmdView_clicked();
            }
        }
        else if (!ui->cbSelected->isVisible() && getContentMode() == 1 && ev->button() == Qt::LeftButton && ev->modifiers().testFlag(Qt::ShiftModifier))
        {
            ui->cbSelected->setChecked(!ui->cbSelected->isChecked());
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
        adjustTextColor();
        return true;
    }
    return false;
}

bool SnapmaticWidget::makePictureVisible()
{
    if (smpic->setPictureVisible())
    {
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
    SnapmaticEditor *snapmaticEditor = new SnapmaticEditor(crewDB, profileDB, this);
    snapmaticEditor->setSnapmaticPicture(smpic);
    snapmaticEditor->setModal(true);
    snapmaticEditor->show();
    snapmaticEditor->exec();
    delete snapmaticEditor;
}

void SnapmaticWidget::editSnapmaticRawJson()
{
    JsonEditorDialog *jsonEditor = new JsonEditorDialog(smpic, this);
    jsonEditor->setModal(true);
    jsonEditor->show();
    jsonEditor->exec();
    delete jsonEditor;
}

void SnapmaticWidget::editSnapmaticImage()
{
    ImageEditorDialog *imageEditor = new ImageEditorDialog(smpic, profileName, this);
    imageEditor->setModal(true);
    imageEditor->show();
    imageEditor->exec();
    delete imageEditor;
}

void SnapmaticWidget::openMapViewer()
{
    SnapmaticPicture *picture = smpic;
    MapLocationDialog *mapLocDialog = new MapLocationDialog(picture->getSnapmaticProperties().location.x, picture->getSnapmaticProperties().location.y, this);
    mapLocDialog->setModal(true);
    mapLocDialog->show();
    mapLocDialog->exec();
    if (mapLocDialog->propUpdated())
    {
        // Update Snapmatic Properties
        SnapmaticProperties localSpJson = picture->getSnapmaticProperties();
        localSpJson.location.x = mapLocDialog->getXpos();
        localSpJson.location.y = mapLocDialog->getYpos();
        localSpJson.location.z = 0;

        // Update Snapmatic Picture
        QString currentFilePath = picture->getPictureFilePath();
        QString originalFilePath = picture->getOriginalPictureFilePath();
        QString backupFileName = originalFilePath % ".bak";
        if (!QFile::exists(backupFileName))
        {
            QFile::copy(currentFilePath, backupFileName);
        }
        SnapmaticProperties fallbackProperties = picture->getSnapmaticProperties();
        picture->setSnapmaticProperties(localSpJson);
        if (!picture->exportPicture(currentFilePath))
        {
            QMessageBox::warning(this, SnapmaticEditor::tr("Snapmatic Properties"), SnapmaticEditor::tr("Patching of Snapmatic Properties failed because of I/O Error"));
            picture->setSnapmaticProperties(fallbackProperties);
        }
    }
    delete mapLocDialog;
}

bool SnapmaticWidget::isSelected()
{
    return ui->cbSelected->isChecked();
}

bool SnapmaticWidget::isHidden()
{
    return smpic->isHidden();
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
    return smpic->getPictureFilePath();
}

QString SnapmaticWidget::getWidgetType()
{
    return "SnapmaticWidget";
}
