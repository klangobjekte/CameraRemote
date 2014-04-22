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
    void on_urlLineEdit_textEdited(QString url);
    void on_portLineEdit_textEdited(QString port);
    void on_isoSpeedRateComboBox_currentTextChanged(QString text);
    void on_shutterSpeedComboBox_currentTextChanged(QString text);
    void on_fNumberComboBox_currentTextChanged(QString text);
    void on_whiteBalanceComboBox_currentTextChanged(QString text);
    void on_exposureModeComboBox_currentTextChanged(QString text);

    void addIsoSpeedRateComboBoxItems(QStringList items);
    void addfNumberComboBoxItems(QStringList items);
    void addwhiteBalanceComboBoxItems(QStringList items);
    void addshutterSpeedComboBox_2Items(QStringList items);
    void addexposureModeComboBoxItems(QStringList items);

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
