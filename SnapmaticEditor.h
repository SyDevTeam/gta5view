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

#ifndef SNAPMATICEDITOR_H
#define SNAPMATICEDITOR_H

#include <QDialog>
#include "SnapmaticPicture.h"

namespace Ui {
class SnapmaticEditor;
}

class SnapmaticEditor : public QDialog
{
    Q_OBJECT

public:
    explicit SnapmaticEditor(QWidget *parent = 0);
    void setSnapmaticPicture(SnapmaticPicture *picture);
    void setSnapmaticTitle(const QString &title);
    ~SnapmaticEditor();

private slots:
    void on_cbSelfie_toggled(bool checked);
    void on_cbMugshot_toggled(bool checked);
    void on_cbDirector_toggled(bool checked);
    void on_cbEditor_toggled(bool checked);
    void on_rbSelfie_toggled(bool checked);
    void on_rbMugshot_toggled(bool checked);
    void on_rbEditor_toggled(bool checked);
    void on_rbCustom_toggled(bool checked);
    void on_cmdCancel_clicked();
    void on_cmdApply_clicked();
    void on_cbQualify_toggled(bool checked);
    void on_labTitle_linkActivated(const QString &link);

private:
    Ui::SnapmaticEditor *ui;
    SnapmaticProperties localSpJson;
    SnapmaticPicture *smpic;
    QString snapmaticTitle;
    void qualifyAvatar();
};

#endif // SNAPMATICEDITOR_H
