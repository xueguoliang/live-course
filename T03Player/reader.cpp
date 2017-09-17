
#include "reader.h"
#include <QDebug>

Reader::Reader(QObject *parent) :
    QThread(parent)
{
}

void Reader::run()
{
    while(!data->quit)
    {
        while(data->getPacketCount() > 1000)
        {
            QThread::msleep(20);
        }

        AVPacket* pkt = av_packet_alloc();

        int ret = av_read_frame(data->ic, pkt);
        if(ret != 0)
        {
            qDebug() << "reader quit ret:" << ret;
            break;
        }

        if(pkt->stream_index == data->vIndex ||
                pkt->stream_index == data->aIndex)
        {
            data->addPacket(pkt);
        }
        else
        {
            av_packet_free(&pkt);
        }
    }

    qDebug() << "reader quit";
}
