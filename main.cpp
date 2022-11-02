#include "mainwindow.h"

#include <QApplication>
#include <QDir>

int main(int argc, char *argv[]) {
    char* const names[] = {
        "1.pgm",
        "2.pgm"
    };
    str_array ethalons = { names,2 };

    QApplication a(argc, argv);
    QDir::setCurrent("C:\\Users\\admin\\Downloads\\archive");
    MainWindow w(ethalons);
    w.show();
    return a.exec();
}
