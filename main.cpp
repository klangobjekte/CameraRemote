#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include "cameraremotedefinitions.h"
#include "../QsLog_2/QsLog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("CameraRemote");
    app.setOrganizationDomain("klangobjekte.com");
    app.setOrganizationName("KlangObjekte");
    app.setQuitOnLastWindowClosed(true);



    // init the logging mechanism
    QsLogging::Logger& logger = QsLogging::Logger::instance();

    // set minimum log level and file name
    logger.setLoggingLevel(QsLogging::TraceLevel);
    const QString sLogPath(QDir(app.applicationDirPath()).filePath("log.txt"));

    // Create log destinations
    QsLogging::DestinationPtr fileDestination(QsLogging::DestinationFactory::MakeFileDestination(sLogPath, true, 512, 2) );
    QsLogging::DestinationPtr debugDestination(QsLogging::DestinationFactory::MakeDebugOutputDestination() );

    // set log destinations on the logger
    logger.addDestination(debugDestination );
    logger.addDestination(fileDestination);

    // write an info message
    QLOG_INFO() << "Program started";
    QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
    QLOG_INFO() << "Path to Logging File: " << sLogPath;

    QLOG_TRACE() << "Here's a" << QString::fromUtf8("trace") << "message";
    QLOG_DEBUG() << "Here's a" << static_cast<int>(QsLogging::DebugLevel) << "message";
    QLOG_WARN()  << "Uh-oh!";
    qDebug() << "This message won't be picked up by the logger";
    QLOG_ERROR() << "An error has occurred";
    qWarning() << "Neither will this one";
    QLOG_FATAL() << "Fatal error!";

    logger.setLoggingLevel(QsLogging::OffLevel);
    for (int i = 0;i < 10000000;++i) {
        QLOG_ERROR() << QString::fromUtf8("logging is turned off");
    }

    //QFile file(qsspath + "default.qss");
    QFile file(":default.qss");
    file.open(QFile::ReadOnly);
    //QString styleSheet = QLatin1String(file.readAll());
    //app.setStyleSheet(styleSheet);

    MainWindow w;
    w.show();
    return app.exec();
}
