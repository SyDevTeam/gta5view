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

#ifndef SNAPMATICWIDGET_H
#define SNAPMATICWIDGET_H

#include "SnapmaticPicture.h"
#include "ProfileDatabase.h"
#include "DatabaseThread.h"
#include "ProfileWidget.h"
#include "CrewDatabase.h"
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QWidget>
#include <QColor>

namespace Ui {
class SnapmaticWidget;
}

class SnapmaticWidget : public ProfileWidget
{
    Q_OBJECT

public:
    SnapmaticWidget(ProfileDatabase *profileDB, CrewDatabase *crewDB, DatabaseThread *threadDB, QWidget *parent = 0);
    void setSnapmaticPicture(SnapmaticPicture *picture);
    void setSelectionMode(bool selectionMode);
    void setSelected(bool isSelected);
    bool deletePicture();
    bool makePictureVisible();
    bool makePictureHidden();
    SnapmaticPicture *getPicture();
    QString getPicturePath();
    QString getWidgetType();
    bool isSelected();
    bool isHidden();
    void retranslate();
    ~SnapmaticWidget();

private slots:
    void on_cmdView_clicked();
    void on_cmdCopy_clicked();
    void on_cmdExport_clicked();
    void on_cmdDelete_clicked();
    void on_cbSelected_stateChanged(int arg1);
    void adjustTextColor();
    void pictureSelected();
    void selectAllWidgets();
    void deselectAllWidgets();
    void dialogNextPictureRequested();
    void dialogPreviousPictureRequested();
    void makePictureVisibleSlot();
    void makePictureHiddenSlot();
    void editSnapmaticProperties();
    void snapmaticUpdated();

protected:
    void mouseDoubleClickEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void contextMenuEvent(QContextMenuEvent *ev);

private:
    ProfileDatabase *profileDB;
    CrewDatabase *crewDB;
    DatabaseThread *threadDB;
    Ui::SnapmaticWidget *ui;
    SnapmaticPicture *smpic;
    QColor highlightHiddenColor;

signals:
    void pictureDeleted();
    void widgetSelected();
    void widgetDeselected();
    void allWidgetsSelected();
    void allWidgetsDeselected();
    void nextPictureRequested(QWidget *dialog);
    void previousPictureRequested(QWidget *dialog);
    void contextMenuTriggered(QContextMenuEvent *ev);
};

#endif // SNAPMATICWIDGET_H
