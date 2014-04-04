#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>

class Timelapse;
class Remote;
class QNetworkAccessManager;
class QNetworkReply;
class QGraphicsView;
class QGraphicsScene;
class QLabel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:



private slots:
    void on_startRecModePushButton_clicked();
    void on_stopRecModePushButton_clicked();
    void on_takePicturePushButton_clicked();
    void on_startLiveViewPushButton_clicked();
    void on_stopLiveViewPushButton_clicked();
    void on_startTimerPushButton_clicked();
    void on_stopTimerPushButton_clicked();
    void on_durationTimeEdit_timeChanged(const QTime &time);
    void on_intervalTimeEdit_timeChanged(const QTime &time);
    void on_chooseFolderPushButton_clicked();
    void on_urlLineEdit_textChanged(QString url);
    void on_portLineEdit_textChanged(QString port);


    void drawPreview(QNetworkReply *reply,QString previePicName);

private:
    Ui::MainWindow *ui;
    Remote *remote;
    Timelapse *timelapse;
    QLabel *label;
    QGraphicsScene *scene;
    QGraphicsView * view;
    QString previewPath;

    void savePreviewFile(QByteArray bytes,QString previePicName);
};

#endif // MAINWINDOW_H
