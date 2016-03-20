#ifndef PICTUREDIALOG_H
#define PICTUREDIALOG_H

#include <QDialog>

namespace Ui {
class PictureDialog;
}

class PictureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PictureDialog(QWidget *parent = 0);
    ~PictureDialog();

private:
    Ui::PictureDialog *ui;
};

#endif // PICTUREDIALOG_H
