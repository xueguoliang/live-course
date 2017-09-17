#include "data.h"
#include <QApplication>
#include "surface.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    av_register_all();
    avcodec_register_all();

    Surface surface;
    surface.show();

    return app.exec();
}
