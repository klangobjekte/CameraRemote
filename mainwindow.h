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
class QButtonGroup;

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
    //bool eventFilter(QObject *object, QEvent *event);

public slots:
    void onConnectionStatusChanged(int status,QString message = QString());
    void onCameraStatusChanged(QString state);
    void setControlStates(bool on);


private slots:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    void on_buttonGroup_buttonClicked(int index);
    void on_orientationChanged(Qt::ScreenOrientation orientation);
    void on_primaryOrientationChanged(Qt::ScreenOrientation orientation);
    void on_GeometryChanged(QRect geo);
    void setupLandscapeScreen(QRect geo);
    void setupPortraitScreen(QRect geo);
    void setLayoutDimensions(Qt::ScreenOrientation orientation);
    void resizeWindow(QSize orientedSize, QSize innerorientedSize, QSize viewSize);
    void on_configurationComboBox_activated(QString text);
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
    void on_liveViewImageTouched(QPointF pos);


    void addIsoSpeedRateComboBoxItems(QStringList items);
    void addfNumberComboBoxItems(QStringList items);
    void addwhiteBalanceComboBoxItems(QStringList items);
    void addshutterSpeedComboBox_2Items(QStringList items);
    void addexposureModeComboBoxItems(QStringList items);
    void addSelfTimerComboBoxItems(QStringList items);
    void addPostViewImageSizeComboBoxItems(QStringList items);
    void addConfigurationComboBoxItems(QStringList items);


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

    void on_zoomInPushButton_pressed();

    void on_zoomInPushButton_released();

    void on_zoomOutPushButton_pressed();

    void on_zoomOutPushButton_released();


    void on_zoomPositionChanged(const int &text);

    void on_quitPushButton_clicked(bool checked);

private:

    Ui::MainWindow *ui;
    QTimer *zoomBoxTimer;

    NetworkConnection *networkConnection;
    Remote *remote;
    Timelapse *timelapse;
    double pressedEnd;
    double pressedBegin;
    //double elapsed_secs;
    //QLabel *label;
    QButtonGroup *buttonGroup;
    QImage previewimg;
    QImage liveviewimg;
    QSize dialogsize;
    QSize liveViewSize;

    QGraphicsScene *previewScene;
    QGraphicsScene *liveviewScene;
    //QGraphicsView * view;
    QString previewPath;
    QString friendlyName;
    QString pictureLocation;
    QString homepath;

    void savePreviewFile(QByteArray bytes,QString previePicName);
    void readSettings();
    void writeSettings();
    float fontsize;
    float statusBarSize;
    QFont myf;
    int pushbuttonsize;
    QSize liveviewimgsize;
    QSize currentsize;
    bool processingstate;
};

#endif // MAINWINDOW_H
