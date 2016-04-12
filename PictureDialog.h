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

#ifndef PICTUREDIALOG_H
#define PICTUREDIALOG_H

#include "SnapmaticPicture.h"
#include "ProfileDatabase.h"
#include <QDialog>
#include <QMenu>

namespace Ui {
class PictureDialog;
}

class PictureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PictureDialog(ProfileDatabase *profileDB, QWidget *parent = 0);
    void setSnapmaticPicture(SnapmaticPicture *picture, QString picPath, bool readOk, bool indexed, int index);
    void setSnapmaticPicture(SnapmaticPicture *picture, QString picPath, bool readOk);
    void setSnapmaticPicture(SnapmaticPicture *picture, QString picPath);
    void setSnapmaticPicture(SnapmaticPicture *picture, bool readOk, int index);
    void setSnapmaticPicture(SnapmaticPicture *picture, bool readOk);
    void setSnapmaticPicture(SnapmaticPicture *picture, int index);
    void setSnapmaticPicture(SnapmaticPicture *picture);
    bool isIndexed();
    int getIndex();
    ~PictureDialog();

public slots:
    void playerNameUpdated();
    void dialogNextPictureRequested();
    void dialogPreviousPictureRequested();

private slots:
    void copySnapmaticPicture();
    void exportSnapmaticPicture();
    void on_labPicture_mouseDoubleClicked();

signals:
    void nextPictureRequested();
    void previousPictureRequested();
    void newPictureCommited(QImage picture);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private:
    ProfileDatabase *profileDB;
    Ui::PictureDialog *ui;
    SnapmaticPicture *smpic;
    QImage snapmaticPicture;
    QString jsonDrawString;
    QString windowTitleStr;
    QStringList plyrsList;
    QString picTitl;
    QString picPath;
    QString crewID;
    QString locX;
    QString locY;
    QString locZ;
    bool indexed;
    int index;
    QMenu *exportMenu;
};

#endif // PICTUREDIALOG_H
