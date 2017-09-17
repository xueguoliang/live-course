#ifndef READER_H
#define READER_H

#include <QThread>
#include "data.h"

/* reader 负责读取音频和视频数据 */
class Reader : public QThread
{
    Q_OBJECT
public:
    explicit Reader(QObject *parent = 0);

    void run();
    Data* data;

signals:

public slots:

};

#endif // READER_H
