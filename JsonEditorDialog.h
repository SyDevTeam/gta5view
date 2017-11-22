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

#ifndef JSONEDITORDIALOG_H
#define JSONEDITORDIALOG_H

#include "SnapmaticPicture.h"
#include "JSHighlighter.h"
#include <QDialog>

namespace Ui {
class JsonEditorDialog;
}

class JsonEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JsonEditorDialog(SnapmaticPicture *picture, QWidget *parent = 0);
    bool saveJsonContent();
    ~JsonEditorDialog();

protected:
    void closeEvent(QCloseEvent *ev);

private slots:
    void on_cmdClose_clicked();
    void on_cmdSave_clicked();

signals:
    void codeUpdated(QString jsonCode);

private:
    QString jsonCode;
    JSHighlighter *jsonHl;
    SnapmaticPicture *smpic;
    Ui::JsonEditorDialog *ui;
};

#endif // JSONEDITORDIALOG_H
