#include "data.h"
#include <QPainter>
#include "surface.h"
#include <QMenu>
#include <QFileDialog>
#include <QDebug>
#include "reader.h"
#include "player.h"

Surface::Surface(QWidget *parent) :
    QWidget(parent)
{
    reader = NULL;
    player = NULL;
    data = NULL;
}

void Surface::mousePressEvent(QMouseEvent *)
{
    static QMenu* menu = new QMenu(this);
    menu->addAction("打开", this, SLOT(slotOpen()));
    menu->exec(QCursor::pos());
}

void Surface::slotOpen()
{
    QString filename = QFileDialog::getOpenFileName(this);
    if(filename.length() == 0)
        return;

    data = new Data;
    data->filename = filename;
    data->init();

    reader = new Reader();
    reader->data = data;
    reader->start();

    player = new Player;
    player->data = data;
    player->start();

    connect(player, &Player::sigNewFrame, [&](QImage image){
        this->image = image;
        update();
    });
}

void Surface::paintEvent(QPaintEvent *)
{
    if(image.isNull())return;

    QPainter p(this);
    QImage a = image.scaled(size(),
                            Qt::IgnoreAspectRatio,
                            Qt::SmoothTransformation);
    p.drawImage(0, 0, a);
}
