#include "data.h"
#include <QDateTime>
#include <QDebug>
Data::Data(QObject *parent) :
    QObject(parent)
{
    quit = false;
    ic = NULL;
    aCtx = NULL;
    vCtx = NULL;
    sws = NULL;
    swr = NULL;
}

Data::~Data()
{
    if(ic) avformat_free_context(ic);
    if(aCtx) avcodec_free_context(&aCtx);
    if(vCtx) avcodec_free_context(&vCtx);
    if(sws) sws_freeContext(sws);
    if(swr) swr_free(&swr);
}

void Data::init()
{
    /* for reader */
    int ret = avformat_open_input(&ic, filename.toUtf8().data(), NULL, NULL);
    if(ret != 0)
    {
        qDebug() << "open input error" << filename;
        return;
    }

    ret = avformat_find_stream_info(ic, NULL);
    if(ret != 0)
    {
        qDebug() << "find stream error";
        return;
    }

    for(int i=0; i<(int)ic->nb_streams; ++i)
    {
        AVStream* stream = ic->streams[i];
        AVCodecContext* ctx = stream->codec;

        if(ctx->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vCtx = ctx;
            vIndex = i;

            AVCodec* codec = avcodec_find_decoder(ctx->codec_id);
            avcodec_open2(ctx, codec, NULL);
        }
        else if(ctx->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            aCtx = ctx;
            aIndex = i;

            AVCodec* codec = avcodec_find_decoder(ctx->codec_id);
            avcodec_open2(ctx, codec, NULL);
        }
    }

    /* for decorder */
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
    if(!info.isFormatSupported(format))
        format = info.nearestFormat(format);


    sws = sws_getCachedContext(NULL,
                                     vCtx->width,
                                     vCtx->height,
                                     vCtx->pix_fmt,
                                     vCtx->width,
                                     vCtx->height,
                                     AV_PIX_FMT_RGBA,
                                     SWS_BICUBIC, NULL, NULL, NULL);


    swr = swr_alloc_set_opts(NULL, aCtx->channel_layout,
                                         AV_SAMPLE_FMT_S16,
                                         format.sampleRate(),
                                         aCtx->channel_layout,
                                         aCtx->sample_fmt,
                                         aCtx->sample_rate,
                                         0, NULL);
    swr_init(swr);

    // for play
    output = new QAudioOutput(format);
    io = output->start();
}

void Data::addPacket(AVPacket *pkt)
{
    mutex.lock();
    pkts.append(pkt);
    mutex.unlock();
}

int Data::getPacketCount()
{
    mutex.lock();
    int ret = pkts.size();
    mutex.unlock();
    return ret;
}
AVPacket* Data::getPacket()
{
    AVPacket* ret = NULL;
    mutex.lock();
    if(pkts.size() > 0)
    {
        ret = pkts.first();
        pkts.removeFirst();
    }
    mutex.unlock();
    return ret;
}

