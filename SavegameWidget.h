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

#ifndef SAVEGAMEWIDGET_H
#define SAVEGAMEWIDGET_H
#include "ProfileWidget.h"
#include "SavegameData.h"
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QWidget>
#include <QColor>

namespace Ui {
class SavegameWidget;
}

class SavegameWidget : public ProfileWidget
{
    Q_OBJECT

public:
    SavegameWidget(QWidget *parent = 0);
    void setSavegameData(SavegameData *savegame, QString savegamePath);
    void setSelectionMode(bool selectionMode);
    void setSelected(bool isSelected);
    SavegameData* getSavegame();
    QString getWidgetType();
    bool isSelected();
    void retranslate();
    ~SavegameWidget();

private slots:
    void on_cmdView_clicked();
    void on_cmdCopy_clicked();
    void on_cmdDelete_clicked();
    void on_cbSelected_stateChanged(int arg1);
    void savegameSelected();
    void selectAllWidgets();
    void deselectAllWidgets();

protected:
    void mouseDoubleClickEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void contextMenuEvent(QContextMenuEvent *ev);

private:
    Ui::SavegameWidget *ui;
    SavegameData *sgdata;
    QString labelAutosaveStr;
    QString labelSaveStr;
    QString sgdPath;
    QString sgdStr;
    void renderString(const QString &savegameString, const QString &fileName);

signals:
    void savegameDeleted();
    void widgetSelected();
    void widgetDeselected();
    void allWidgetsSelected();
    void allWidgetsDeselected();
    void contextMenuTriggered(QContextMenuEvent *ev);
};

#endif // SAVEGAMEWIDGET_H
