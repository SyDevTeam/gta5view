#ifndef SAVEGAMEDIALOG_H
#define SAVEGAMEDIALOG_H

#include "SavegameData.h"
#include <QDialog>

namespace Ui {
class SavegameDialog;
}

class SavegameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SavegameDialog(QWidget *parent = 0);
    void setSavegameData(SavegameData *savegame, QString sgdPath, bool readOk);
    ~SavegameDialog();

private slots:
    void on_cmdClose_clicked();
    void on_cmdCopy_clicked();

private:
    Ui::SavegameDialog *ui;
    QString savegameLabStr;
    QString sgdPath;
};

#endif // SAVEGAMEDIALOG_H
