#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QUrl>

class NetworkConnection;
class Timelapse;
class Remote;
class QNetworkAccessManager;
class QNetworkReply;
class QGraphicsView;
class QGraphicsScene;
class QLabel;
//class QUrl;

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
    void onCameraStatusChanged(int status,QString message = QString());



private slots:
    void closeEvent(QCloseEvent *event);
    void on_configurationComboBox_currentIndexChanged(QString text);
    void on_startRecModePushButton_clicked(bool checked);
    void on_takePicturePushButton_clicked();
    void on_startLiveViewPushButton_clicked(bool checked);
    void on_startTimerPushButton_clicked(bool checked);
    void on_durationTimeEdit_timeChanged(const QTime &time);
    void on_intervalTimeEdit_timeChanged(const QTime &time);
    void on_chooseFolderPushButton_clicked();
    void on_urlLineEdit_textEdited(QString url);
    void on_isoSpeedRateComboBox_activated(QString text);
    void on_shutterSpeedComboBox_activated(QString text);
    void on_fNumberComboBox_activated(QString text);
    void on_whiteBalanceComboBox_activated(QString text);
    void on_exposureModeComboBox_activated(QString text);
    void on_postViewImageSizeComboBox_activated(QString text);
    void on_selfTimerComboBox_activated(QString text);
    void on_zoomComboBox_activated(QString text);

    void addIsoSpeedRateComboBoxItems(QStringList items);
    void addfNumberComboBoxItems(QStringList items);
    void addwhiteBalanceComboBoxItems(QStringList items);
    void addshutterSpeedComboBox_2Items(QStringList items);
    void addexposureModeComboBoxItems(QStringList items);
    void addSelfTimerComboBoxItems(QStringList items);
    void addPostViewImageSizeComboBoxItems(QStringList items);
    void addZoomComboBoxItems(QStringList items);


    void isoSpeedRateComboBox_setCurrentText(QString text);
    void shutterSpeedComboBox_setCurrentText(QString text);
    void fNumberComboBox_setCurrentText(QString text);
    void whiteBalanceComboBox_setCurrentText(QString text);
    void exposureModeComboBox_setCurrentText(QString text);
    void startLiveViewPushButton_setChecked(bool status);
    void selfTimerComboBox_setCurrentText(QString text);
    void postViewImageSizeComboBox_setCurrentText(QString text);



    void drawPreview(QNetworkReply *reply,QString previePicName);
    void drawLiveView(QByteArray data);

private:
    Ui::MainWindow *ui;
    NetworkConnection *networkConnection;
    Remote *remote;
    Timelapse *timelapse;
    //QLabel *label;
    QGraphicsScene *previewScene;
    QGraphicsScene *liveviewScene;
    //QGraphicsView * view;
    QString previewPath;
    QString friendlyName;

    void savePreviewFile(QByteArray bytes,QString previePicName);
    void readSettings();
    void writeSettings();
};

#endif // MAINWINDOW_H
