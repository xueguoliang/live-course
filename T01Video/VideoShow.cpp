#include "VideoShow.h"

VideoShow::VideoShow(QWidget *parent) :
    QWidget(parent)
{
}

void VideoShow::paintEvent(QPaintEvent *)
{
    if(image.isNull()) return;

    QPainter p(this);
    p.drawImage(0, 0, image.scaled(width(), height(),
                             Qt::IgnoreAspectRatio,
                             Qt::SmoothTransformation));
}
