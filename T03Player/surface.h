#ifndef SURFACE_H
#define SURFACE_H

#include <QWidget>
#include <QAudioOutput>
#include <QIODevice>
class Reader;
class Player;
class Data;

class Surface : public QWidget
{
    Q_OBJECT
public:
    explicit Surface(QWidget *parent = 0);
    Reader* reader;
    Player* player;
    Data* data;

    void mousePressEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

    QImage image;

signals:

public slots:
    void slotOpen();

};

#endif // SURFACE_H
