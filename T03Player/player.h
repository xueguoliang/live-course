#ifndef PLAYER_H
#define PLAYER_H

#include <QThread>
#include "data.h"

class Player : public QThread
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = 0);

    void run();
    Data* data;

    bool sig;

signals:
    void sigNewFrame(QImage image);

public slots:

};


#endif // PLAYER_H
