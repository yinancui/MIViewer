#include <QtGui/QApplication>
#include "miviewer.h"
#include "gobackform.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MIViewer w;
    w.show();

    return a.exec();
}
