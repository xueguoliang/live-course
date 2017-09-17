
#include <QCoreApplication>
#include <QDebug>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QIODevice>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
//#include "libavdevice/avdevice.h"

//#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    av_register_all();
    avcodec_register_all();

    AVFormatContext* ic = NULL;
    int ret = avformat_open_input(&ic, "1.mp4", NULL, NULL);
    if(ret != 0)
    {
        qDebug() << "error open file";
        exit(1);
    }

    ret = avformat_find_stream_info(ic, NULL);
    if(ret != 0)
    {
        qDebug() << "find stream info error";
        exit(1);
    }

    AVCodecContext* aCtx = NULL;
    int aIndex;
    for(aIndex=0; aIndex<ic->nb_streams; ++aIndex)
    {
        AVStream* stream = ic->streams[aIndex];
        if(stream->codec->codec_type != AVMEDIA_TYPE_AUDIO)
            continue;
        aCtx = stream->codec;
        AVCodec* codec = avcodec_find_decoder(aCtx->codec_id);
        avcodec_open2(aCtx, codec, NULL);
        break;
    }

    qDebug() << aCtx->codec->name << aCtx->codec->long_name;
    exit(1);

    // 播放设备设置
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
    if(!info.isFormatSupported(format))
        format = info.nearestFormat(format);

    QAudioOutput* output = new QAudioOutput(format);
    QIODevice* io = output->start();



    // 设置resample
    int speed = 2;
    SwrContext* swr = swr_alloc_set_opts(NULL, aCtx->channel_layout,
                                         AV_SAMPLE_FMT_S16,
                                         format.sampleRate()/speed,
                                         aCtx->channel_layout,
                                         aCtx->sample_fmt,
                                         aCtx->sample_rate,
                                         0, NULL);
    swr_init(swr);

    int out_size = 320000*2;
    quint8* play_buf = (quint8*)av_malloc(out_size);

    // AVFrame保存原始图片数据
    AVFrame* frm = av_frame_alloc();
    AVPacket* pkt = av_packet_alloc();

    while(1)
    {
        ret = av_read_frame(ic, pkt);
        if(ret < 0)
        {
            qDebug() << "break here" << ret;
            break;
        }

        if(pkt->stream_index != aIndex)
        {
            av_packet_unref(pkt);
            continue;
        }

        avcodec_send_packet(aCtx, pkt);
        avcodec_receive_frame(aCtx, frm);
        av_packet_unref(pkt);

        ret = swr_convert(swr, &play_buf, out_size,
                          (const uint8_t**)frm->data, frm->nb_samples);

        int buflen = ret * 4;
        int writelen = 0;
        while(1)
        {
            ret = io->write((char*)play_buf + writelen, buflen);
            if(ret > 0)
            {
                buflen -= ret;
                writelen += ret;
                if(buflen == 0)
                    break;
            }
            else if(ret < 0)
            {
                qDebug() << "exit here";
                exit(1);
            }

            io->waitForBytesWritten(10);
        }
    }

    av_free(play_buf);
    av_frame_free(&frm);
    avformat_free_context(ic);
    av_packet_free(&pkt);
    swr_free(&swr);
    delete output;

    return app.exec();
}
