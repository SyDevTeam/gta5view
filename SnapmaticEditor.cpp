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

#include "SnapmaticEditor.h"
#include "ui_SnapmaticEditor.h"
#include "SnapmaticPicture.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>

SnapmaticEditor::SnapmaticEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SnapmaticEditor)
{
    ui->setupUi(this);
    smpic = 0;
}

SnapmaticEditor::~SnapmaticEditor()
{
    delete ui;
}

void SnapmaticEditor::on_cbSelfie_toggled(bool checked)
{
    if (checked)
    {
        ui->cbMugshot->setEnabled(false);
        ui->cbEditor->setEnabled(false);
    }
    else if (!ui->cbDirector->isChecked())
    {
        ui->cbMugshot->setEnabled(true);
        ui->cbEditor->setEnabled(true);
    }
}


void SnapmaticEditor::on_cbMugshot_toggled(bool checked)
{
    if (checked)
    {
        ui->cbSelfie->setEnabled(false);
        ui->cbEditor->setEnabled(false);
        ui->cbDirector->setEnabled(false);
    }
    else
    {
        ui->cbSelfie->setEnabled(true);
        ui->cbEditor->setEnabled(true);
        ui->cbDirector->setEnabled(true);
    }
}

void SnapmaticEditor::on_cbDirector_toggled(bool checked)
{
    if (checked)
    {
        ui->cbMugshot->setEnabled(false);
        ui->cbEditor->setEnabled(false);
    }
    else if (!ui->cbSelfie->isChecked())
    {
        ui->cbMugshot->setEnabled(true);
        ui->cbEditor->setEnabled(true);
    }
}

void SnapmaticEditor::on_cbEditor_toggled(bool checked)
{
    if (checked)
    {
        ui->cbSelfie->setEnabled(false);
        ui->cbMugshot->setEnabled(false);
        ui->cbDirector->setEnabled(false);
    }
    else
    {
        ui->cbSelfie->setEnabled(true);
        ui->cbMugshot->setEnabled(true);
        ui->cbDirector->setEnabled(true);
    }
}

void SnapmaticEditor::on_rbSelfie_toggled(bool checked)
{
    if (checked)
    {
        ui->cbSelfie->setChecked(true);
        ui->cbMugshot->setChecked(false);
        ui->cbEditor->setChecked(false);
        ui->cbDirector->setChecked(false);
        ui->cbMeme->setChecked(false);
        ui->cbSelfie->setEnabled(false);
        ui->cbMugshot->setEnabled(false);
        ui->cbEditor->setEnabled(false);
        ui->cbDirector->setEnabled(false);
        ui->cbMeme->setEnabled(false);
    }
}

void SnapmaticEditor::on_rbMugshot_toggled(bool checked)
{
    if (checked)
    {
        ui->cbSelfie->setChecked(false);
        ui->cbMugshot->setChecked(true);
        ui->cbEditor->setChecked(false);
        ui->cbDirector->setChecked(false);
        ui->cbMeme->setChecked(false);
        ui->cbSelfie->setEnabled(false);
        ui->cbMugshot->setEnabled(false);
        ui->cbEditor->setEnabled(false);
        ui->cbDirector->setEnabled(false);
        ui->cbMeme->setEnabled(false);
    }
}

void SnapmaticEditor::on_rbEditor_toggled(bool checked)
{
    if (checked)
    {
        ui->cbSelfie->setChecked(false);
        ui->cbMugshot->setChecked(false);
        ui->cbEditor->setChecked(true);
        ui->cbDirector->setChecked(false);
        ui->cbMeme->setChecked(false);
        ui->cbSelfie->setEnabled(false);
        ui->cbMugshot->setEnabled(false);
        ui->cbEditor->setEnabled(false);
        ui->cbDirector->setEnabled(false);
        ui->cbMeme->setEnabled(false);
    }
}

void SnapmaticEditor::on_rbCustom_toggled(bool checked)
{
    if (checked)
    {
        ui->cbSelfie->setChecked(false);
        ui->cbMugshot->setChecked(false);
        ui->cbEditor->setChecked(false);
        ui->cbDirector->setChecked(false);
        ui->cbMeme->setChecked(false);
        ui->cbSelfie->setEnabled(true);
        ui->cbMugshot->setEnabled(true);
        ui->cbEditor->setEnabled(true);
        ui->cbDirector->setEnabled(true);
        ui->cbMeme->setEnabled(true);
    }
}

void SnapmaticEditor::setSnapmaticPicture(SnapmaticPicture *picture)
{
    smpic = picture;
    localSpJson = smpic->getSnapmaticProperties();
    ui->rbCustom->setChecked(true);
    ui->cbSelfie->setChecked(localSpJson.isSelfie);
    ui->cbMugshot->setChecked(localSpJson.isMug);
    ui->cbEditor->setChecked(localSpJson.isFromRSEditor);
    ui->cbDirector->setChecked(localSpJson.isFromDirector);
    ui->cbMeme->setChecked(localSpJson.isMeme);
}

void SnapmaticEditor::on_cmdCancel_clicked()
{
    close();
}

void SnapmaticEditor::on_cmdApply_clicked()
{
    localSpJson.isSelfie = ui->cbSelfie->isChecked();
    localSpJson.isMug = ui->cbMugshot->isChecked();
    localSpJson.isFromRSEditor = ui->cbEditor->isChecked();
    localSpJson.isFromDirector = ui->cbDirector->isChecked();
    localSpJson.isMeme = ui->cbMeme->isChecked();
    if (smpic)
    {
        QString originalFileName = smpic->getPictureFileName();
        QString adjustedFileName = originalFileName;
        QString backupFileName = originalFileName + ".bak";
        if (adjustedFileName.right(7) == ".hidden") // for the hidden file system
        {
            adjustedFileName.remove(adjustedFileName.length() - 7, 7);
        }
        if (!QFile::exists(backupFileName))
        {
            QFile::copy(adjustedFileName, backupFileName);
        }
        smpic->setSnapmaticProperties(localSpJson);
        if (!smpic->exportPicture(adjustedFileName))
        {
            QMessageBox::warning(this, tr("Snapmatic Properties"), tr("Patching of Snapmatic Properties failed because of I/O Error"));
        }
    }
    close();
}
