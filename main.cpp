#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include "cameraremotedefinitions.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    //QFile file(qsspath + "default.qss");
    QFile file("./default.qss");
    file.open(QFile::ReadWrite);
    QString styleSheet = QLatin1String(file.readAll());
    app.setStyleSheet(styleSheet);


    MainWindow w;
    w.show();
    return app.exec();
}
