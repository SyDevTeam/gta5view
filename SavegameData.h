#ifndef SAVEGAMEDATA_H
#define SAVEGAMEDATA_H

#include <QObject>

class SavegameData : public QObject
{
    Q_OBJECT
public:
    explicit SavegameData(QObject *parent = 0);

signals:

public slots:
};

#endif // SAVEGAMEDATA_H