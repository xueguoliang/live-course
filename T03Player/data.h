#ifndef DATA_H
#define DATA_H

#include <QObject>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

#include <QList>
#include <QMutex>
#include <QAudioFormat>
#include <QImage>
#include <QAudioOutput>

class Data : public QObject
{
    Q_OBJECT
public:
    explicit Data(QObject *parent = 0);
    ~Data();

    void init();

    AVFormatContext* ic;
    AVCodecContext* vCtx;
    AVCodecContext* aCtx;
    SwsContext* sws;
    SwrContext* swr;

    QAudioOutput* output;
    QIODevice* io;

    QMutex mutex;
    QList<AVPacket*> pkts;
    void addPacket(AVPacket* pkt);
    AVPacket* getPacket();
    int getPacketCount();

    QAudioFormat format;

    int vIndex;
    int aIndex;

    QString filename;
    bool quit;
signals:

public slots:

};

#endif // DATA_H
