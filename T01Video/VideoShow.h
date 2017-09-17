#ifndef VIDEOSHOW_H
#define VIDEOSHOW_H

#include <QWidget>
#include <QPainter>
#include <QImage>

class VideoShow : public QWidget
{
    Q_OBJECT
public:
    explicit VideoShow(QWidget *parent = 0);

    void paintEvent(QPaintEvent *);
    QImage image;


signals:

public slots:

};

#endif // VIDEOSHOW_H
