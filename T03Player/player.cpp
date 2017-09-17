#include "player.h"
#include <QAudioDeviceInfo>
#include <QImage>
#include <QDebug>

Player::Player(QObject *parent) :
    QThread(parent)
{
    sig = false;
}

void Player::run()
{

    // 保存音频原始数据
    int out_size = 320000*2;
    quint8* audioBuf = (quint8*)av_malloc(out_size);

    // 保存视频原始数据
    QImage image(data->vCtx->width, data->vCtx->height, QImage::Format_RGBA8888);
    uint8_t* dst[1] = {image.bits()};
    int dstStride[1] = {4*image.width()};

    AVFrame* vFrm = av_frame_alloc();
    AVFrame* aFrm = av_frame_alloc();

    while(!data->quit)
    {
        AVPacket* pkt = data->getPacket();
        if(pkt == NULL)
        {
            QThread::msleep(20);
            continue;
        }

        if(pkt->stream_index == data->vIndex)
        {
            avcodec_send_packet(data->vCtx, pkt);
            avcodec_receive_frame(data->vCtx, vFrm);

            sws_scale(data->sws, vFrm->data, vFrm->linesize,
                      0, vFrm->height, dst, dstStride);

            emit sigNewFrame(image);
        }
        else if(pkt->stream_index == data->aIndex)
        {
            avcodec_send_packet(data->aCtx, pkt);
            avcodec_receive_frame(data->aCtx, aFrm);

            int ret = swr_convert(data->swr, &audioBuf, out_size,
                              (const uint8_t**)aFrm->data, aFrm->nb_samples);

            int len = ret*4;
            int wlen = 0;
            while(wlen < len)
            {
                int ret = data->io->write((char*)audioBuf + wlen, len - wlen);
                if(ret > 0)
                {
                    wlen += ret;
                    data->io->waitForBytesWritten(1);
                }
                else
                {
                    exit(1);
                }
            }
        }

        av_packet_free(&pkt);
    }

    delete []audioBuf;
    av_frame_free(&vFrm);
    av_frame_free(&aFrm);
}
