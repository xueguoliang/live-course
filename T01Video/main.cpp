#include <QApplication>
#include "VideoShow.h"
#include <QDebug>
#include <QThread>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
//#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
//#include "libswresample/swresample.h"
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    VideoShow win;
    win.show();

    av_register_all();
    avcodec_register_all();

    // 打开文件
    AVFormatContext* ic = NULL;
    int ret = avformat_open_input(&ic, "1.mp4", NULL, NULL);
    if(ret != 0)
    {
        qDebug() << "error open file";
        exit(1);
    }

    // 查看文件视频长度有多少时间，单位是1/AV_TIME_BASE秒
    qDebug() << ic->duration;

    // 从视频文件中，读取一些文件内容，得到视频文件的属性信息
    ret = avformat_find_stream_info(ic, NULL);

    if(ret != 0)
    {
        qDebug() << "find stream info error";
        exit(1);
    }

    qDebug() << "stream number is" << ic->nb_streams;

    // 得到的信息有流数量，流详情和解码上下文
    // 这里解码，其实就是解压缩，解码上下文保存着
    // 解码参数，这些参数和视频文件保存在一起
    // 可以称之为“文件头”
    AVCodecContext* vCtx = NULL;
    int vIndex;
    for(vIndex=0; vIndex<ic->nb_streams; ++vIndex)
    {
        // AVStream保存某个流的参数
        // 流可能是视频流，音频流或者字幕流
        // 这个文件并没有字幕，所以只有两个流
        AVStream* stream = ic->streams[vIndex];
        // 如果解码类型是VIDEO，说明stream指向视频流
        if(stream->codec->codec_type != AVMEDIA_TYPE_VIDEO)
            continue;
        // 得到解码上下文指针
        vCtx = stream->codec;
        // 打开解码codec
        // AVCodec和AVCodecContenxt的区别在于
        // AVCodec保存方法，AVCodecContext保存参数
        // AVCodecContext保存了AVCodec
        AVCodec* codec = avcodec_find_decoder(vCtx->codec_id);
        avcodec_open2(vCtx, codec, NULL);
        break;
    }

    qDebug() << vCtx->codec->name;
    exit(1);

    // AVFrame保存原始图片数据
    AVFrame* frm = av_frame_alloc();
    AVPacket* pkt = av_packet_alloc();

    // SwsContent保存着图片转换的参数
    // 主要包括：
    // 输入的图片像素格式和尺寸
    // 输出的图片像素格式和尺寸
    // 当需要缩放时，使用的算法
    // 以及一些filter设置（filter在换转时，能加水印啥的，不过我们不做这个处理）
    SwsContext* sws = sws_getCachedContext(NULL,
                                           vCtx->width,
                                           vCtx->height,
                                           vCtx->pix_fmt,
                                           vCtx->width,
                                           vCtx->height,
                                           AV_PIX_FMT_RGBA,
                                           SWS_BICUBIC, NULL, NULL, NULL);

    while(1)
    {
        ret = av_read_frame(ic, pkt);
        if(ret < 0)
        {
            qDebug() << "break here" << ret;
            break;
        }

        if(pkt->stream_index != vIndex)
        {
            av_packet_unref(pkt);
            continue;
        }

        //    qDebug() << vCtx->pkt_timebase.num;
        //    qDebug() << vCtx->pkt_timebase.den;
        // qDebug() << pkt->duration;
        int duration = pkt->duration * 1000 * vCtx->pkt_timebase.num / vCtx->pkt_timebase.den;
        qDebug() << "duration time is:" << duration;
        // 把压缩数据（pkt）send过去，然后receive一个原始格式的图像（frm）
        // 完成了解压缩过程
        avcodec_send_packet(vCtx, pkt);
        avcodec_receive_frame(vCtx, frm);
        av_packet_unref(pkt);

        // 得到的frm的数据格式，是yuv格式的，不是RGB格式
        // 这里yuv和RGB是像素格式（还记得么），所以要转换
        QImage image(vCtx->width, vCtx->height, QImage::Format_RGBA8888);
        uint8_t* dst[1] = {image.bits()};
        int dstStride[1] = {4*image.width()};
        sws_scale(sws, frm->data, frm->linesize,
                  0, frm->height, dst, dstStride);

        // 转换完毕之后，直接在窗口显示
        win.image = image;
        win.update();

        qDebug() << "pkt pts:"<< pkt->pts << "pkt dts:" <<pkt->dts;
        qDebug() << "frm pts:" << frm->pts << "frm dts:" << frm->pkt_dts;
        app.processEvents();

        QThread::msleep(duration);
    }

    av_packet_free(&pkt);
    av_frame_free(&frm);
    avformat_free_context(ic);
    av_packet_free(&pkt);
    sws_freeContext(sws);

    return app.exec();
}
