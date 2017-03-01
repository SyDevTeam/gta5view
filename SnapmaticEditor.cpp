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

#include "SnapmaticEditor.h"
#include "ui_SnapmaticEditor.h"
#include "SnapmaticPicture.h"
#include "StringParser.h"
#include <QTextDocument>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QFile>

SnapmaticEditor::SnapmaticEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SnapmaticEditor)
{
    ui->setupUi(this);
    ui->cbSelfie->setVisible(false);
    ui->cbMugshot->setVisible(false);
    ui->cbEditor->setVisible(false);
    ui->cmdApply->setDefault(true);

    if (QIcon::hasThemeIcon("dialog-apply"))
    {
        ui->cmdApply->setIcon(QIcon::fromTheme("dialog-apply"));
    }
    if (QIcon::hasThemeIcon("dialog-cancel"))
    {
        ui->cmdCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
    }

    snapmaticTitle = "";
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
        ui->cbMugshot->setChecked(false);
        ui->cbEditor->setChecked(false);
        ui->cbSelfie->setChecked(true);
    }
}

void SnapmaticEditor::on_rbMugshot_toggled(bool checked)
{
    if (checked)
    {
        ui->cbSelfie->setChecked(false);
        ui->cbEditor->setChecked(false);
        ui->cbDirector->setChecked(false);
        ui->cbMugshot->setChecked(true);
    }
}

void SnapmaticEditor::on_rbEditor_toggled(bool checked)
{
    if (checked)
    {
        ui->cbSelfie->setChecked(false);
        ui->cbMugshot->setChecked(false);
        ui->cbDirector->setChecked(false);
        ui->cbEditor->setChecked(true);
    }
}

void SnapmaticEditor::on_rbCustom_toggled(bool checked)
{
    if (checked)
    {
        ui->cbSelfie->setChecked(false);
        ui->cbMugshot->setChecked(false);
        ui->cbEditor->setChecked(false);
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
    if (ui->cbSelfie->isChecked())
    {
        ui->rbSelfie->setChecked(true);
    }
    else if (ui->cbMugshot->isChecked())
    {
        ui->rbMugshot->setChecked(true);
    }
    else if (ui->cbEditor->isChecked())
    {
        ui->rbEditor->setChecked(true);
    }
    else
    {
        ui->rbCustom->setChecked(true);
    }
    setSnapmaticTitle(picture->getPictureTitle());
}

void SnapmaticEditor::setSnapmaticTitle(const QString &title)
{
    if (title.length() > 39)
    {
        snapmaticTitle = title.left(39);
    }
    else
    {
        snapmaticTitle = title;
    }
    QString editStr = QString("<a href=\"g5e://edittitle\" style=\"text-decoration: none;\">%1</a>").arg(tr("Edit"));
    QString titleStr = tr("Title: %1 (%2)").arg(StringParser::escapeString(snapmaticTitle), editStr);
    ui->labTitle->setText(titleStr);
    if (SnapmaticPicture::verifyTitle(snapmaticTitle))
    {
        ui->labAppropriate->setText(tr("Appropriate: %1").arg(QString("<span style=\"color: green\">%1</a>").arg(tr("Yes", "Yes, should work fine"))));
    }
    else
    {
        ui->labAppropriate->setText(tr("Appropriate: %1").arg(QString("<span style=\"color: red\">%1</a>").arg(tr("No", "No, could lead to issues"))));
    }
}

void SnapmaticEditor::on_cmdCancel_clicked()
{
    close();
}

void SnapmaticEditor::on_cmdApply_clicked()
{
    if (ui->cbQualify->isChecked())
    {
        qualifyAvatar();
    }
    localSpJson.isSelfie = ui->cbSelfie->isChecked();
    localSpJson.isMug = ui->cbMugshot->isChecked();
    localSpJson.isFromRSEditor = ui->cbEditor->isChecked();
    localSpJson.isFromDirector = ui->cbDirector->isChecked();
    localSpJson.isMeme = ui->cbMeme->isChecked();
    if (smpic)
    {
        QString originalFileName = smpic->getPictureFilePath();
        QString adjustedFileName = originalFileName;
        if (adjustedFileName.right(7) == ".hidden") // for the hidden file system
        {
            adjustedFileName.remove(adjustedFileName.length() - 7, 7);
        }
        QString backupFileName = adjustedFileName + ".bak";
        if (!QFile::exists(backupFileName))
        {
            QFile::copy(adjustedFileName, backupFileName);
        }
        SnapmaticProperties fallbackProperties = smpic->getSnapmaticProperties();
        QString fallbackTitle = smpic->getPictureTitle();
        smpic->setSnapmaticProperties(localSpJson);
        smpic->setPictureTitle(snapmaticTitle);
        if (!smpic->exportPicture(originalFileName))
        {
            QMessageBox::warning(this, tr("Snapmatic Properties"), tr("Patching of Snapmatic Properties failed because of I/O Error"));
            smpic->setSnapmaticProperties(fallbackProperties);
            smpic->setPictureTitle(fallbackTitle);
        }
        else
        {
            smpic->emitUpdate();
        }
    }
    close();
}

void SnapmaticEditor::qualifyAvatar()
{
    ui->rbSelfie->setChecked(true);
    ui->cbDirector->setChecked(false);
    ui->cbMeme->setChecked(false);
    ui->cmdApply->setDefault(true);
}

void SnapmaticEditor::on_cbQualify_toggled(bool checked)
{
    if (checked)
    {
        ui->cbMeme->setEnabled(false);
        ui->cbDirector->setEnabled(false);
        ui->rbCustom->setEnabled(false);
        ui->rbSelfie->setEnabled(false);
        ui->rbEditor->setEnabled(false);
        ui->rbMugshot->setEnabled(false);
    }
    else
    {
        ui->cbMeme->setEnabled(true);
        ui->rbCustom->setEnabled(true);
        ui->rbSelfie->setEnabled(true);
        ui->rbEditor->setEnabled(true);
        ui->rbMugshot->setEnabled(true);
        if (ui->rbSelfie->isChecked() || ui->rbCustom->isChecked())
        {
            ui->cbDirector->setEnabled(true);
        }
    }
}

void SnapmaticEditor::on_labTitle_linkActivated(const QString &link)
{
    if (link == "g5e://edittitle")
    {
        bool ok;
        QString newTitle = QInputDialog::getText(this, tr("Snapmatic Title"), tr("New Snapmatic title:"), QLineEdit::Normal, snapmaticTitle, &ok, windowFlags());
        if (ok && !newTitle.isEmpty())
        {
            setSnapmaticTitle(newTitle);
        }
    }
}
