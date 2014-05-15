#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
//#include <QUrl>
#include <QDebug>
#include <QSettings>
#include "networkconnection.h"
#include "remote.h"
#include "timelapse.h"
#include "cameraremotedefinitions.h"
#include "QGraphicsOpacityEffect"
#include <QDesktopWidget>
#include <QButtonGroup>
#include <QScreen>
#include <QTimer>
//#include <QStandardPaths>
#include <QMouseEvent>
#include <QPoint>
#include <QTouchEvent>
#include "iostream"
#include "iomanip"

using namespace std;

#define LOG_MAINWINDOW
#ifdef LOG_MAINWINDOW
#   define LOG_MAINWINDOW_DEBUG qDebug()
#else
#   define LOG_MAINWINDOW_DEBUG nullDebug()
#endif

#define LOG_SCREENDESIGN
#ifdef LOG_SCREENDESIGN
#   define LOG_SCREENDESIGN_DEBUG qDebug()
#else
#   define LOG_SCREENDESIGN_DEBUG nullDebug()
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

     ui->setupUi(this);
    //! Duration of ZoomButton Pressed
    //! for Future use
    pressedBegin = 0;
    pressedEnd = 0;


    //pictureLocation = QDir::homePath ();
    //pictureLocation = QStandardPaths::displayName(QStandardPaths::PicturesLocation);
    //pictureLocation = QStandardPaths::locate(QStandardPaths::PicturesLocation,"Pictures",QStandardPaths::LocateDirectory);
    //pictureLocation.append("/Pictures/");
    //qDebug() << "homeLocation: " << QStandardPaths::displayName(QStandardPaths::HomeLocation);
    //qDebug() << "pictureLocation: " << pictureLocation;
    previewPath = pictureLocation;


    int dpi=QPaintDevice::physicalDpiX();
    qDebug() << "physical DPI of Screen: " << dpi;
    fontsize=dpi/8;                                //change to your liking
    LOG_SCREENDESIGN_DEBUG << "fontsize: " << fontsize;
    statusBarSize = fontsize/2;
    myf=QApplication::font();
    myf.setPixelSize(fontsize);
    QApplication::setFont(myf);
    pushbuttonsize = fontsize*6;
    //setWindowFlags(Qt::FramelessWindowHint);
    QFont statusfont = myf;
    statusfont.setPixelSize(myf.pixelSize()/2);
    this->statusBar()->setFont(statusfont);
    this->statusBar()->setMaximumHeight(statusBarSize);

    QScreen* screen = QApplication::primaryScreen();
    LOG_SCREENDESIGN_DEBUG << "Konstructor screen->screenGeometry():     " << screen->geometry();
    LOG_SCREENDESIGN_DEBUG << "Konstructor screen->availableGeometry();: " << screen->availableGeometry();

    connect( screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(on_orientationChanged(Qt::ScreenOrientation)) );
    connect( screen, SIGNAL(primaryOrientationChanged(Qt::ScreenOrientation)), this, SLOT(on_primaryOrientationChanged(Qt::ScreenOrientation)) );
    connect( screen, SIGNAL(geometryChanged(QRect)), this, SLOT(on_GeometryChanged(QRect)) );





    buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->pushButton_1);
    buttonGroup->addButton(ui->pushButton_2);
    buttonGroup->addButton(ui->pushButton_3);
    buttonGroup->setId(ui->pushButton_1,0);
    buttonGroup->setId(ui->pushButton_2,1);
    buttonGroup->setId(ui->pushButton_3,2);
    connect(buttonGroup,SIGNAL(buttonClicked(int)),
            this,SLOT(on_buttonGroup_buttonClicked(int)));

    // to get a geometryChangedEvent
    ui->pushButton_1->setVisible(false);
    ui->stackedWidget->setCurrentIndex(0);

    //QStringList testlist;
    //testlist << "test1" <<"test2"<<"test3" << "test4" << "test5" << "test5" << "test6" << "test7" << "test8" << "test9";
    //ui->zoomComboBox->model()->setData(ui->zoomComboBox->model()->index(0, 0), QSize(1000, 1000), Qt::SizeHintRole);
    QStringList test;
    test << "test1" << "test2";
    ui->zoomPositionLabel->setText("test");


    //! [Create Object Connection to WiFi]
    networkConnection = new NetworkConnection;

    connect(networkConnection,SIGNAL(publishUrl(QString)),
            ui->urlLineEdit,SLOT(setText(QString)));
    connect(networkConnection,SIGNAL(publishConnectionStatus(int,QString)),
            this,SLOT(onCameraStatusChanged(int,QString)));
    connect(networkConnection,SIGNAL(publishDeviceFound(QStringList)),
            this,SLOT(addConfigurationComboBoxItems(QStringList)));

    networkConnection->init();


    //! [Create Object QGraphicsScene for Preview]
    previewScene = new QGraphicsScene;
    liveviewScene = new QGraphicsScene;



    //! [Create Object Camera Remote]
    remote = new Remote(networkConnection,this);
    connect(remote, SIGNAL(publishLoadPreview(QNetworkReply*,QString)),
            this, SLOT(drawPreview(QNetworkReply*,QString)));

    connect(remote,SIGNAL(publishLiveViewBytes(QByteArray)),
            this,SLOT(drawLiveView(QByteArray)));

    connect(remote,SIGNAL(publishAvailableIsoSpeedRates(QStringList)),
        this,SLOT(addIsoSpeedRateComboBoxItems(QStringList)));

    connect(remote,SIGNAL(publishAvailableFNumber(QStringList)),
        this,SLOT(addfNumberComboBoxItems(QStringList)));

    connect(remote,SIGNAL(publishAvailableShutterSpeed(QStringList)),
        this,SLOT(addshutterSpeedComboBox_2Items(QStringList)));

    connect(remote,SIGNAL(publishAvailableWhiteBalanceModes(QStringList)),
        this,SLOT(addwhiteBalanceComboBoxItems(QStringList)));

    connect(remote,SIGNAL(publishAvailableExposureModes(QStringList)),
            this,SLOT(addexposureModeComboBoxItems(QStringList)));

    connect(remote,SIGNAL(publishAvailablePostviewImageSizeCandidates(QStringList)),
            this,SLOT(addPostViewImageSizeComboBoxItems(QStringList)));
    connect(remote,SIGNAL(publishAvailablselfTimerCandidates(QStringList)),
            this,SLOT(addSelfTimerComboBoxItems(QStringList)));



    connect(remote,SIGNAL(publishCurrentIsoSpeedRates(QString)),
            ui->isoSpeedRateComboBox,SLOT(setCurrentText(QString)));
    connect(remote,SIGNAL(publishCurrentShutterSpeed(QString)),
            ui->shutterSpeedComboBox,SLOT(setCurrentText(QString)));
    connect(remote,SIGNAL(publishCurrentFNumber(QString)),
            ui->fNumberComboBox,SLOT(setCurrentText(QString)));
    connect(remote,SIGNAL(publishCurrentWhiteBalanceModes(QString)),
            ui->whiteBalanceComboBox,SLOT(setCurrentText(QString)));
    connect(remote,SIGNAL(publishCurrentExposureMode(QString)),
            ui->exposureModeComboBox,SLOT(setCurrentText(QString)));
    connect(remote,SIGNAL(publishLiveViewStatus(bool)),
            ui->startLiveViewPushButton,SLOT(setChecked(bool)));
    connect(remote,SIGNAL(publishCurrentSelfTimer(QString)),
            ui->selfTimerComboBox,SLOT(setCurrentText(QString)));
    connect(remote,SIGNAL(publishCurrentPostviewImageSize(QString)),
            ui->postViewImageSizeComboBox,SLOT(setCurrentText(QString)));
    connect(remote,SIGNAL(publishZoomPosition(int)),
            ui->zoomPositionLabel,SLOT(setNum(int)));

    connect(networkConnection,SIGNAL(publishConnectionStatus(int,QString)),
            remote,SLOT(setConnectionStatus(int,QString)));

    //! [Create Object Timelapse Control]
    timelapse = new Timelapse;
    connect(timelapse,SIGNAL(publishTakePicture()),
            this,SLOT(on_takePicturePushButton_clicked()));

    QTime defaultduration(0,1,0,0);
    QTime defaultinterval(0,0,1,0);
    ui->durationTimeEdit->setTime(defaultduration);
    ui->intervalTimeEdit->setTime(defaultinterval);
    timelapse->setDuration(defaultduration);
    timelapse->setInterval(defaultinterval);


    connect(ui->loadPreviewPicCheckBox,SIGNAL(stateChanged(int)),
            remote,SLOT(setLoadPreviewPic(int)));
    ui->loadPreviewPicCheckBox->setChecked(true);



    QStringList availableNetworks = networkConnection->getAvailableNetWorks();
    LOG_MAINWINDOW_DEBUG << "availableNetworks: " << availableNetworks;
    ui->configurationComboBox->addItems(availableNetworks);
    ui->configurationComboBox->setCurrentText(networkConnection->getActiveConfiguration().name());

    remote->setDevice(friendlyName);
    remote->initialEvent();

    ui->toolBar->setVisible(false);
    ui->mainToolBar->setVisible(false);

    ui->LiveviewGraphicsView->installEventFilter(this);
#if ! defined (Q_OS_IOS) && ! defined (Q_OS_ANDROID)
    this->installEventFilter(this);
    on_GeometryChanged(screen->availableGeometry());
#else
    ui->LiveviewGraphicsView->setAttribute(Qt::WA_AcceptTouchEvents);
#endif

    readSettings();
}

MainWindow::~MainWindow()
{
    delete timelapse;
    delete remote;
    delete previewScene;
    delete liveviewScene;
    delete ui;
    delete networkConnection;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    writeSettings();
    LOG_MAINWINDOW_DEBUG << "Leave Application";
    remote->stopLiveview();
    remote->stopRecMode();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event){
    //qDebug() << "event: " << event->type() << "object " << object->objectName();
#if ! defined (Q_OS_IOS) && ! defined (Q_OS_ANDROID)
    if (event->type() == QEvent::Resize){
        //qDebug() << "event: " << event->type() << "object " << object->objectName();

        //QScreen* screen = QApplication::primaryScreen();
        //QRect geo = screen->geometry();



        if(!(this->size() == currentsize)){
            QSize orientedSize = this->size();
            qDebug() << "resize: " << size();
            QSize orientedInnerSize;
            orientedInnerSize.setHeight(size().height()-statusBarSize);
            orientedInnerSize.setWidth(size().width()- size().width()/60);
            QSize viewSize;
            viewSize.setHeight((orientedInnerSize.height()-pushbuttonsize));
            viewSize.setWidth(orientedInnerSize.width());
            resizeWindow(orientedSize,orientedInnerSize,viewSize);
        }
        currentsize = this->size();
        return QMainWindow::eventFilter(object, event);
    }
#endif

    if(object->objectName() == "LiveviewGraphicsView"){
#if ! defined (Q_OS_IOS) && ! defined (Q_OS_ANDROID)
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint point = mouseEvent->pos();
            qDebug() << "MouseButtonPress event: " << event->type() << "object " << object->objectName() <<"pos: " << point ;
            //return true;
            on_liveViewImageTouched(point);
            return QMainWindow::eventFilter(object, event);
        }
#else

        if (event->type() == QEvent::TouchBegin){
            QTouchEvent *touchBeginEvent = static_cast<QTouchEvent*>(event);

            QList<QTouchEvent::TouchPoint> touchpoints = touchBeginEvent->touchPoints();
            QTouchEvent::TouchPoint touchpoint;
            //touchpoint.lastPos();
            //QPointF pointf = touchBeginEvent
            foreach(touchpoint,touchpoints){
                qDebug() << "TouchBegin event: " << event->type() << "object " << object->objectName() <<"pos: " << touchpoint.lastPos() ;
            }
            //return true;
            on_liveViewImageTouched(touchpoint.lastPos());
            return QMainWindow::eventFilter(object, event);
        }
#endif

    }
    //return true;
}

void MainWindow::on_buttonGroup_buttonClicked(int index){
            ui->stackedWidget->setCurrentIndex(index);
            switch (index) {
            case 0:
                ui->pushButton_1->setVisible(false);
                ui->pushButton_2->setVisible(true);
                ui->pushButton_3->setVisible(true);
                break;
            case 1:
                ui->pushButton_2->setVisible(false);
                ui->pushButton_1->setVisible(true);
                ui->pushButton_3->setVisible(true);
                break;

               case 2:
                ui->pushButton_3->setVisible(false);
                ui->pushButton_1->setVisible(true);
                ui->pushButton_2->setVisible(false);
                break;
            default:
                break;
            }
}

void MainWindow::on_primaryOrientationChanged(Qt::ScreenOrientation orientation){
    LOG_SCREENDESIGN_DEBUG << "on_primaryOrientationChanged " << orientation;
    setLayoutDimensions(orientation);
}

void MainWindow::on_orientationChanged(Qt::ScreenOrientation orientation){
    LOG_SCREENDESIGN_DEBUG << "on_orientationChanged " << orientation;
    setLayoutDimensions(orientation);
}

void MainWindow::on_GeometryChanged(QRect geo){
    LOG_SCREENDESIGN_DEBUG << "on_GeometryChanged " << geo;

    if(geo.width()>geo.height()){
        setLayoutDimensions(Qt::LandscapeOrientation);
    }
    else{
        setLayoutDimensions(Qt::PortraitOrientation);
    }


}

void MainWindow::setLayoutDimensions(Qt::ScreenOrientation orientation){
    QScreen* screen = QApplication::primaryScreen();

#if ! defined (Q_OS_IOS) && ! defined (Q_OS_ANDROID)
    QRect geo = screen->availableGeometry();
#else
    QRect geo = screen->geometry();
#endif
    LOG_SCREENDESIGN_DEBUG << "screen->screenGeometry():     " << screen->geometry();
    LOG_SCREENDESIGN_DEBUG << "screen->availableGeometry();: " << screen->availableGeometry();

    if(orientation == Qt::PortraitOrientation){
        setupPortraitScreen(geo);
    }
    else if(orientation == Qt::LandscapeOrientation){
        setupLandscapeScreen(geo);
    }
    LOG_SCREENDESIGN_DEBUG << "setLayoutDimensions orientation: " << orientation;
}



void MainWindow::setupPortraitScreen(QRect geo){
     LOG_SCREENDESIGN_DEBUG << "++++++++ setupPortraitScreen ++++++++";
     for(int y= 0; y< ui->centralGridLayout->rowCount();y++){
         for(int i= 0;i< ui->centralGridLayout->columnCount();i++){
            QLayoutItem *item = ui->centralGridLayout->itemAtPosition(y,i);
             ui->centralGridLayout->removeItem(item);
         }
     }

    ui->centralWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    ui->centralGridLayoutWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    ui->stackedWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui->LiveviewGraphicsView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->previewGraphicsView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    ui->pushButton_1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui->pushButton_2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui->pushButton_3->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui->zoomInPushButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui->zoomOutPushButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui->takePicturePushButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    ui->fNumberComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    ui->shutterSpeedComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    ui->isoSpeedRateComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    ui->whiteBalanceComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    ui->zoomPositionLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);




    ui->centralGridLayout->addWidget(ui->stackedWidget,0,0,1,6);
    ui->centralGridLayout->addWidget(ui->pushButton_1,1,0);
    ui->centralGridLayout->addWidget(ui->pushButton_2,1,1);
    ui->centralGridLayout->addWidget(ui->pushButton_3,1,2);
    ui->centralGridLayout->addWidget(ui->zoomOutPushButton,1,3);
    ui->centralGridLayout->addWidget(ui->zoomInPushButton,1,4);

    ui->centralGridLayout->addWidget(ui->takePicturePushButton,1,5);


    QSize orientedSize;
    QSize orientedInnerSize;


    orientedInnerSize.setWidth(geo.width());
    orientedInnerSize.setHeight(geo.height()-statusBarSize);
    orientedSize.setWidth(geo.width());
    orientedSize.setHeight(geo.height());
    QSize viewSize;
    viewSize.setHeight((orientedInnerSize.height()-pushbuttonsize));
    viewSize.setWidth(geo.width());
    resizeWindow(orientedSize,orientedInnerSize,viewSize);

}

void MainWindow::setupLandscapeScreen(QRect geo){
    LOG_SCREENDESIGN_DEBUG << "++++++++ setupLandscapeScreen ++++++++";



    QSize orientedSize;
    QSize orientedInnerSize;
    QSize viewSize;
#if ! defined (Q_OS_IOS) && ! defined (Q_OS_ANDROID)
    for(int y= 0; y< ui->centralGridLayout->rowCount();y++){
        for(int i= 0;i< ui->centralGridLayout->columnCount();i++){
           QLayoutItem *item = ui->centralGridLayout->itemAtPosition(y,i);
           ui->centralGridLayout->removeItem(item);
        }
    }

    ui->centralWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    ui->centralGridLayoutWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);



    ui->centralGridLayout->addWidget(ui->stackedWidget,0,0,1,6);
    ui->centralGridLayout->addWidget(ui->pushButton_1,1,0);
    ui->centralGridLayout->addWidget(ui->pushButton_2,1,1);
    ui->centralGridLayout->addWidget(ui->pushButton_3,1,2);
    ui->centralGridLayout->addWidget(ui->zoomOutPushButton,1,3);
    ui->centralGridLayout->addWidget(ui->zoomInPushButton,1,4);

    ui->centralGridLayout->addWidget(ui->takePicturePushButton,1,5);


    orientedInnerSize.setWidth(geo.width()-geo.width()/60);
    orientedInnerSize.setHeight(geo.height() - statusBarSize);
    orientedSize.setWidth(geo.width());
    orientedSize.setHeight(geo.height());
    viewSize.setHeight((orientedInnerSize.height()-pushbuttonsize));
    viewSize.setWidth(orientedInnerSize.width());

#else


    for(int y= 0; y< ui->centralGridLayout->rowCount();y++){
        for(int i= 0;i< ui->centralGridLayout->columnCount();i++){
           QLayoutItem *item = ui->centralGridLayout->itemAtPosition(y,i);
           ui->centralGridLayout->removeItem(item);
        }
    }

    qDebug() << "ui->centralGridLayout->rowCount(): " << ui->centralGridLayout->rowCount();
    qDebug() << "ui->centralGridLayout->columnCount(): " << ui->centralGridLayout->columnCount();


    ui->centralWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->centralGridLayoutWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->stackedWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    ui->LiveviewGraphicsView->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    ui->previewGraphicsView->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);


    ui->pushButton_1->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    ui->pushButton_2->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    ui->pushButton_3->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    ui->zoomInPushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    ui->zoomOutPushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    ui->takePicturePushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    /*
    ui->fNumberComboBox->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    ui->shutterSpeedComboBox->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    ui->isoSpeedRateComboBox->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    ui->whiteBalanceComboBox->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    ui->zoomPositionLabel->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    */

    ui->centralGridLayout->addWidget(ui->stackedWidget,0,0,6,1);
    ui->centralGridLayout->addWidget(ui->takePicturePushButton,0,1);

    ui->centralGridLayout->addWidget(ui->pushButton_3,1,1);
    ui->centralGridLayout->addWidget(ui->zoomOutPushButton,2,1);
    ui->centralGridLayout->addWidget(ui->zoomInPushButton,3,1);

    ui->centralGridLayout->addWidget(ui->pushButton_1,4,1);
    ui->centralGridLayout->addWidget(ui->pushButton_2,5,1);




    orientedInnerSize.setWidth(geo.width() - geo.width()/60);
    orientedInnerSize.setHeight(geo.height() - statusBarSize);
    orientedSize.setWidth(geo.width());
    orientedSize.setHeight(geo.height());
    viewSize.setHeight((orientedInnerSize.height()));
    viewSize.setWidth(orientedInnerSize.width());





#endif
     resizeWindow(orientedSize,orientedInnerSize,viewSize);
}

void MainWindow::resizeWindow(QSize  orientedSize,
                              QSize innerorientedSize,
                              QSize viewSize){
    LOG_SCREENDESIGN_DEBUG << "resizeWindow orientedSize width     : " <<  orientedSize.width() << " height: " << orientedSize.height();
    LOG_SCREENDESIGN_DEBUG << "resizeWindow innerorientedSize width: " <<  innerorientedSize.width() << " height: " << innerorientedSize.height();
    LOG_SCREENDESIGN_DEBUG << "resizeWindow viewSize width         : " <<  viewSize.width() << " height: " << viewSize.height();
    LOG_SCREENDESIGN_DEBUG << "resizeWindow fontsize               : " <<  fontsize;// << " height: " << viewSize.height();
    LOG_SCREENDESIGN_DEBUG << "resizeWindow pushbuttonsize         : " <<  pushbuttonsize;// << " height: " << viewSize.height();
    LOG_SCREENDESIGN_DEBUG << "resizeWindow statusBarSize          : " <<  statusBarSize;// << " height: " << viewSize.height();


    QFont statusfont = myf;
    statusfont.setPixelSize(myf.pixelSize()/2);
    this->statusBar()->setFont(statusfont);
    this->statusBar()->setMaximumHeight(statusBarSize);
    this->statusBar()->resize(statusBar()->width(),statusBarSize);



    ui->LiveviewGraphicsView->resize(innerorientedSize.width(),viewSize.height()-fontsize);
    ui->previewGraphicsView->resize(innerorientedSize.width(),viewSize.height()-fontsize);

    if(!previewimg.isNull())
        previewimg = previewimg.scaled(ui->previewGraphicsView->size(),Qt::KeepAspectRatio);
    if(!liveviewimg.isNull())
        liveviewimg = liveviewimg.scaled(ui->LiveviewGraphicsView->size(),Qt::KeepAspectRatio);

    dialogsize = innerorientedSize;


    ui->controlGridLayoutWidget->resize(innerorientedSize.width(),viewSize.height()-fontsize);

    ui->controlGridLayout->setRowMinimumHeight(1,fontsize+2);
    ui->controlGridLayout->setVerticalSpacing(0);


    ui->stackedWidget->resize(innerorientedSize.width(),
            viewSize.height());

    ui->page->resize(innerorientedSize.width(),
            viewSize.height());

    ui->page_2->resize(innerorientedSize.width(),
            viewSize.height());


    ui->settingsGridLayoutWidget->resize(innerorientedSize.width(),
            viewSize.height());


    ui->previewVerticalLayoutWidget->resize(innerorientedSize.width(),viewSize.height());









    //centralGridLayoutWidget
    ui->centralGridLayout->setRowMinimumHeight(1,pushbuttonsize);
    ui->centralGridLayout->setVerticalSpacing(0);
    ui->centralGridLayoutWidget->resize(innerorientedSize);
    ui->centralWidget->resize(innerorientedSize);





    ui->pushButton_1->resize(ui->pushButton_1->width(),pushbuttonsize);
    ui->pushButton_2->resize(ui->pushButton_1->width(),pushbuttonsize);
    ui->pushButton_3->resize(ui->pushButton_1->width(),pushbuttonsize);
    ui->zoomInPushButton->resize(ui->zoomInPushButton->width(),pushbuttonsize);
    ui->zoomOutPushButton->resize(ui->zoomOutPushButton->width(),pushbuttonsize);
    ui->takePicturePushButton->resize(ui->takePicturePushButton->width(),pushbuttonsize);
    ui->zoomPositionLabel->resize(ui->zoomPositionLabel->width(),pushbuttonsize);

   this->resize(orientedSize);
}


void MainWindow::onCameraStatusChanged(int status,QString message){
    QString temp4("Connected: ");
    QString temp6("Camera Detected: ");
    switch(status){
    case _CONNECTIONSTATE_DISCONNECTED:
        statusBar()->showMessage(tr("Disconnected"));
        ui->fNumberComboBox->clear();
        ui->shutterSpeedComboBox->clear();
        ui->whiteBalanceComboBox->clear();
        ui->isoSpeedRateComboBox->clear();
        ui->exposureModeComboBox->clear();
        ui->startLiveViewPushButton->setChecked(false);
        ui->startRecModePushButton->setChecked(false);
        break;
    case _CONNECTIONSTATE_WATING:
        statusBar()->showMessage(tr("Listening..."));
        ui->fNumberComboBox->clear();
        ui->shutterSpeedComboBox->clear();
        ui->whiteBalanceComboBox->clear();
        ui->isoSpeedRateComboBox->clear();
        ui->exposureModeComboBox->clear();
        ui->startLiveViewPushButton->setChecked(false);
        ui->startRecModePushButton->setChecked(false);
        break;
    case _CONNECTIONSTATE_CONNECTING:
        statusBar()->showMessage(tr("Connecting..."));
        break;
    case _CONNECTIONSTATE_CONNECTET:
        temp4.append(friendlyName);
        statusBar()->showMessage(temp4);
        ui->startRecModePushButton->setChecked(true);
        ui->startRecModePushButton->setText("Disconnect");
        break;
    case _CONNECTIONSTATE_SSDP_ALIVE_RECEIVED:
        if(!remote->getConnectionStatus())
            statusBar()->showMessage(tr("Device Detected"));
        break;
    case _CONNECTIONSTATE_CAMERA_DETECTED:
        if(!remote->getConnectionStatus()){
            friendlyName = message;
            temp6.append(friendlyName);
            statusBar()->showMessage(temp6);
        }
        break;
    }
}

void MainWindow::on_configurationComboBox_activated(QString text){
    networkConnection->setActiveNetwork(text);
    remote->setActiveNetworkConnection();
}

void MainWindow::on_startRecModePushButton_clicked(bool checked){
    if(checked){
        remote->initActEnabelMethods();
        //remote->startRecMode();
        ui->startRecModePushButton->setText("Disconnect");
    }
    else{
        remote->stopLiveview();
        remote->stopRecMode();
        liveviewScene->clear();
        previewScene->clear();
        ui->startRecModePushButton->setText("Connect");
    }
}

void MainWindow::on_takePicturePushButton_clicked(){
    remote->actTakePicture();
}

void MainWindow::on_startLiveViewPushButton_clicked(bool checked){
    if(checked){
        remote->getAvailableLiveviewSize(remote->getMethods().value("getAvailableLiveviewSize"));
        remote->startLiveview();
    }
    else{
        remote->stopLiveview();
        liveviewScene->clear();
        remote->setLiveViewStartToManual();
    }
}

void MainWindow::on_startTimerPushButton_clicked(bool checked){
    if(checked)
        timelapse->start();
    else
        timelapse->stop();
}

void MainWindow::on_intervalTimeEdit_timeChanged(const QTime &time){
    timelapse->setInterval(time);
}

void MainWindow::on_durationTimeEdit_timeChanged(const QTime &time){
    timelapse->setDuration(time);
}

void MainWindow::on_chooseFolderPushButton_clicked(){
    QFileDialog dialog;
    //dialog.setFixedSize(dialogsize);
     dialog.setWindowState(dialog.windowState() | Qt::WindowMaximized);
    previewPath = QFileDialog::getExistingDirectory(this,"select Locationd",previewPath);
}

void MainWindow::on_urlLineEdit_textEdited(QString url){
    networkConnection->setUrl(url);
    writeSettings();
    //remote->setUrl(url);
}

void MainWindow::on_isoSpeedRateComboBox_activated(QString text){
    QByteArray index;
    index.append("\"");
    index.append(text);
    index.append("\"");
    remote->commandFabrikMethod("setIsoSpeedRate",remote->getMethods().value("setIsoSpeedRate"),index);
}

void MainWindow::on_shutterSpeedComboBox_activated(QString text){
    text.remove("\"");
    QByteArray index;
    index.append("\"");
    index.append(text);
    index.append("\"");
    remote->commandFabrikMethod("setShutterSpeed",remote->getMethods().value("setShutterSpeed"),index);
}

void MainWindow::on_fNumberComboBox_activated(QString text){
    QByteArray param;
    param.append("\"");
    param.append(text);
    param.append("\"");
    remote->commandFabrikMethod("setFNumber",remote->getMethods().value("setFNumber"),param);
}

void MainWindow::on_whiteBalanceComboBox_activated(QString text){
    QByteArray index;
    index.append("\"");
    index.append(text);
    index.append("\",");
    index.append("true");
    index.append(",");
    index.append("0");
    remote->commandFabrikMethod("setWhiteBalance",remote->getMethods().value("setWhiteBalance"),index);
}

void MainWindow::on_exposureModeComboBox_activated(QString text){
    QByteArray param;
    param.append("\"");
    param.append(text);
    param.append("\"");
    remote->commandFabrikMethod("setExposureMode",remote->getMethods().value("setExposureMode"),param);
}

void MainWindow::on_selfTimerComboBox_activated(QString text){
    //int para = text.toInt();
    QByteArray param;
    //param.append("\"");
    param.append(text);
    //param.append("\"");
    //param.append(parachar);
    //param = para;
    remote->commandFabrikMethod("setSelfTimer",remote->getMethods().value("setSelfTimer"),param);
}

void MainWindow::on_postViewImageSizeComboBox_activated(QString text){
    QByteArray param;
    param.append("\"");
    param.append(text);
    param.append("\"");
    remote->commandFabrikMethod("setPostviewImageSize",remote->getMethods().value("setPostviewImageSize"),param);
}

void MainWindow::on_liveViewImageTouched(QPointF pos){

    QSize size = ui->LiveviewGraphicsView->size();//.scaled( liveviewimgsize,Qt::KeepAspectRatio);
    //size.height();
    //size.width();
    float hsizefaktor = (float)size.height()/(float)liveviewimgsize.height();
    float wsizefaktor = (float)size.width()/(float)liveviewimgsize.width();
    qDebug() << "on_liveViewImageTouched ui->LiveviewGraphicsView->size(): " << size<< "liveviewimgsize: " << liveviewimgsize;// << "sizefaktor: " << sizefaktor;

    //float xaxis = pos.x()/10;
    //float yaxis = pos.y()/10;
    float xaxis = pos.x()/hsizefaktor/10*2;
    float yaxis = pos.y()/wsizefaktor/10*2;
    QString sxaxis;
    sxaxis.setNum(xaxis);
    QString syaxis;
    syaxis.setNum(yaxis);
    qDebug() << "on_liveViewImageTouched: " << sxaxis << " " << syaxis << "hsizefaktor: " << hsizefaktor;
    QByteArray param;
    //param.append("\"");
    param.append(sxaxis);
    //param.append("\"");
    param.append(", ");
    //param.append("\"");
    param.append(syaxis);
    //param.append("\"");
    remote->commandFabrikMethod("setTouchAFPosition",remote->getMethods().value("setTouchAFPosition"),param);

}

void MainWindow::on_zoomInPushButton_pressed()
{
    //clock_t end = clock();

    pressedBegin = clock();
    double elapsed_secs = double(pressedEnd - pressedBegin) / CLOCKS_PER_SEC;
    if(elapsed_secs > 0){
        cout  << setprecision(8) << " ++++++++++++ TIME elapsed+++++++:" << elapsed_secs << endl;
    }


    QByteArray param;
    param.append("\"");
    param.append("in");
    param.append("\"");
    param.append(",");
    param.append("\"");
    param.append("1shot");
    param.append("\"");
    remote->commandFabrikMethod("actZoom",remote->getMethods().value("actZoom"),param);
}

void MainWindow::on_zoomInPushButton_released()
{
    pressedEnd = clock();
    QByteArray param;
    param.append("\"");
    param.append("in");
    param.append("\"");
    param.append(",");
    param.append("\"");
    param.append("stop");
    param.append("\"");
    remote->commandFabrikMethod("actZoom",remote->getMethods().value("actZoom"),param);
    double elapsed_secs = double(pressedEnd - pressedBegin) / CLOCKS_PER_SEC;
    if(elapsed_secs > 0){
        cout  << setprecision(8) << " ++++++++++++ TIME elapsed+++++++:" << elapsed_secs << endl;
    }

}

void MainWindow::on_zoomOutPushButton_pressed()
{
    pressedBegin = clock();
    double elapsed_secs = double(pressedEnd - pressedBegin) / CLOCKS_PER_SEC;
    if(elapsed_secs > 0){
        cout  << setprecision(8) << " ++++++++++++ TIME elapsed+++++++:" << elapsed_secs << endl;
    }
    QByteArray param;
    param.append("\"");
    param.append("out");
    param.append("\"");
    param.append(",");
    param.append("\"");
    param.append("1shot");
    param.append("\"");
    remote->commandFabrikMethod("actZoom",remote->getMethods().value("actZoom"),param);
}

void MainWindow::on_zoomOutPushButton_released()
{
    pressedEnd = clock();
    double elapsed_secs = double(pressedEnd - pressedBegin) / CLOCKS_PER_SEC;
    if(elapsed_secs > 0){
        cout  << setprecision(8) << " ++++++++++++ TIME elapsed+++++++:" << elapsed_secs << endl;
    }
    QByteArray param;
    param.append("\"");
    param.append("out");
    param.append("\"");
    param.append(",");
    param.append("\"");
    param.append("stop");
    param.append("\"");
    remote->commandFabrikMethod("actZoom",remote->getMethods().value("actZoom"),param);
}




void MainWindow::addIsoSpeedRateComboBoxItems(QStringList items){
    foreach (QString item, items) {
        if(ui->isoSpeedRateComboBox->findText(item) == -1){
            ui->isoSpeedRateComboBox->addItem(item);
        }
    }
}


void MainWindow::addfNumberComboBoxItems(QStringList items){
    foreach (QString item, items) {
        if(ui->fNumberComboBox->findText(item) == -1){
            ui->fNumberComboBox->addItem(item);
        }
    }
}

void MainWindow::addshutterSpeedComboBox_2Items(QStringList items){
    foreach (QString item, items) {
        if(ui->shutterSpeedComboBox->findText(item) == -1){
            ui->shutterSpeedComboBox->addItem(item);
        }
    }
}

void MainWindow::addwhiteBalanceComboBoxItems(QStringList items){
    foreach (QString item, items) {
        if(ui->whiteBalanceComboBox->findText(item) == -1){
            ui->whiteBalanceComboBox->addItem(item);
        }
    }
}

void MainWindow::addexposureModeComboBoxItems(QStringList items){
    foreach (QString item, items) {
        if(ui->exposureModeComboBox->findText(item) == -1){
            ui->exposureModeComboBox->addItem(item);
        }
    }
}

void MainWindow::addSelfTimerComboBoxItems(QStringList items){
    foreach (QString item, items) {
        if(ui->selfTimerComboBox->findText(item) == -1){
            ui->selfTimerComboBox->addItem(item);
        }
    }
}



void MainWindow::addPostViewImageSizeComboBoxItems(QStringList items){
    foreach (QString item, items) {
        if(ui->postViewImageSizeComboBox->findText(item) == -1){
            ui->postViewImageSizeComboBox->addItem(item);
        }
    }
}


void MainWindow::addConfigurationComboBoxItems(QStringList items){
    foreach (QString item, items) {
        if(ui->configurationComboBox->findText(item) == -1){
            ui->configurationComboBox->addItem(item);
        }
    }

}

void MainWindow::isoSpeedRateComboBox_setCurrentText(QString text){
    //ui->isoSpeedRateComboBox
}

void MainWindow::shutterSpeedComboBox_setCurrentText(QString text){
    //ui->shutterSpeedComboBox
}

void MainWindow::fNumberComboBox_setCurrentText(QString text){
    //ui->fNumberComboBox
}

void MainWindow::whiteBalanceComboBox_setCurrentText(QString text){
    //ui->whiteBalanceComboBox
}

void MainWindow::exposureModeComboBox_setCurrentText(QString text){
    //ui->exposureModeComboBox
}

void MainWindow::startLiveViewPushButton_setChecked(bool status){
    //ui->startLiveViewPushButton
}

void MainWindow::selfTimerComboBox_setCurrentText(QString text){
    //ui->selfTimerComboBox
}

void MainWindow::postViewImageSizeComboBox_setCurrentText(QString text){
    //ui->postViewImageSizeComboBox
}



void MainWindow::drawPreview(QNetworkReply *reply,QString previePicName){
    //LOG_SCREENDESIGN_DEBUG << "drawPreview";
    QByteArray bytes = reply->readAll();
    //QImage img;

    previewimg.loadFromData(bytes);
    if(!previewimg.isNull()){
        QSize size = previewimg.size();
        LOG_SCREENDESIGN_DEBUG << "Preview image Size: " << size;
        QSize widgetSize = ui->previewGraphicsView->size();
        LOG_SCREENDESIGN_DEBUG << "previewGraphicsView Size: " << widgetSize;
        previewimg = previewimg.scaled(widgetSize,Qt::KeepAspectRatio);
        QPixmap pixmap = QPixmap::fromImage(previewimg);
        previewScene->clear();
        previewScene->addPixmap(pixmap);
        ui->previewGraphicsView->setScene(previewScene);

        savePreviewFile(bytes,previePicName);
    }
}

void MainWindow::savePreviewFile(QByteArray bytes,QString previePicName){
    if(!previewPath.isEmpty()){
        QString path = previewPath;
        path.append(previePicName);
        //LOG_MAINWINDOW_DEBUG << "path: " << path;
        QFile file( path );
        if( !file.open( QIODevice::WriteOnly ) ){
            LOG_MAINWINDOW_DEBUG << "Unable to open";
            LOG_MAINWINDOW_DEBUG << file.errorString();
            return;
        }
        if(file.write( bytes ) < 0 ){
            LOG_MAINWINDOW_DEBUG << "Unable to write";
            LOG_MAINWINDOW_DEBUG << file.errorString();
        }
        file.close();
    }
}

void MainWindow::drawLiveView(QByteArray bytes){
    //LOG_SCREENDESIGN_DEBUG << "drawLiveView";
#ifdef __STORE__SINGLE_PREVIEW_PICS
    static int number = 0;
    QFile file("testpic"+QString::number(number)+".jpeg");
    if (!file.open(QIODevice::WriteOnly)) {
        //return ;

    }
    else{
        QDataStream out(&file);
        //out.setVersion(QDataStream::Qt_4_3);
        out.writeRawData(bytes,bytes.size());
        //out << bytes;
        file.close();
    }
    number++;
    LOG_MAINWINDOW_DEBUG << "number: " << number;
#endif
        liveviewimg.loadFromData(bytes);
        if(!liveviewimg.isNull()){
            liveviewimgsize = liveviewimg.size();
            //LOG_SCREENDESIGN_DEBUG << "LiveView image Size: " << size;
            QSize widgetSize = ui->LiveviewGraphicsView->size();
            liveviewimg = liveviewimg.scaled(widgetSize,Qt::KeepAspectRatio);
            QPixmap pixmap = QPixmap::fromImage(liveviewimg);
            liveviewScene->clear();
            liveviewScene->addPixmap(pixmap);
            ui->LiveviewGraphicsView->setScene(liveviewScene);
        }
}



//!  +++++++++++++++++++++++++ [ SETTINGS ] +++++++++++++++++++++++++++

void MainWindow::readSettings()
{
    QSettings settings("KlangObjekte.", "CameraRemote");
    networkConnection->setUrl(settings.value("url",QString("http://127.0.0.1")).toString());
    networkConnection->setPort(settings.value("port",8080).toString());
    friendlyName = settings.value("friendlyName",QString()).toString();
    previewPath = settings.value("previewPath").toString();
    ui->loadPreviewPicCheckBox->setChecked(settings.value("loadpreviePic",true).toBool());
    QPoint pos = settings.value("pos", QPoint(200,200)).toPoint();
    //QSize size = settings.value("size", QSize(300, 200)).toSize();
    //! gibt geometry einProblem mit den Toolbars??
    //restoreGeometry(settings.value("geometry").toByteArray());

    restoreState(settings.value("state").toByteArray());
    //resize(size);
    move(pos);
    //this->pos().setX(pos.x());
    //this->pos().setY(pos.y());
}

void MainWindow::writeSettings()
{
    QSettings settings("KlangObjekte.", "CameraRemote");
    settings.setValue("previewPath",previewPath);
    settings.setValue("loadpreviePic",ui->loadPreviewPicCheckBox->isChecked());
    settings.setValue("url",networkConnection->getUrl().toString());
    settings.setValue("port",networkConnection->getUrl().port());
    settings.setValue("friendlyName",friendlyName);
    settings.setValue("pos", pos());
    //settings.setValue("size", size());
    //! gibt geometry einProblem mit den Toolbars??
    //settings.setValue("geometry", saveGeometry());
    //! die Breite und Position der DockWindows!!
    //! wenn sie nicht gefloated sind!!
    settings.setValue("state", saveState());
}




/*
    ui->whiteBalanceComboBox->setMinimumHeight(fontsize);
    ui->fNumberComboBox->setMinimumHeight(fontsize);
    ui->shutterSpeedComboBox->setMinimumHeight(fontsize);
    ui->isoSpeedRateComboBox->setMinimumHeight(fontsize);
    //ui->exposureModeComboBox->setMaximumHeight(fontsize);
    ui->zoomPositionLabel->setMinimumHeight(fontsize);

    ui->whiteBalanceComboBox->setMaximumHeight(fontsize);
    ui->fNumberComboBox->setMaximumHeight(fontsize);
    ui->shutterSpeedComboBox->setMaximumHeight(fontsize);
    ui->whiteBalanceComboBox->setMaximumHeight(fontsize);
    ui->isoSpeedRateComboBox->setMaximumHeight(fontsize);
    //ui->exposureModeComboBox->setMaximumHeight(fontsize);
    ui->zoomPositionLabel->setMaximumHeight(fontsize);
*/



/*
    ui->pushButton_1->setMaximumHeight(pushbuttonsize);
    ui->pushButton_2->setMaximumHeight(pushbuttonsize);
    ui->pushButton_3->setMaximumHeight(pushbuttonsize);
    ui->takePicturePushButton->setMaximumHeight(pushbuttonsize);

    ui->pushButton_1->setIconSize(QSize(ui->takePicturePushButton->iconSize().width(),pushbuttonsize));
    ui->pushButton_2->setIconSize(QSize(ui->takePicturePushButton->iconSize().width(),pushbuttonsize));
    ui->pushButton_3->setIconSize(QSize(ui->takePicturePushButton->iconSize().width(),pushbuttonsize));
    ui->takePicturePushButton->setIconSize(QSize(ui->takePicturePushButton->iconSize().width(),pushbuttonsize));
*/

/*
    ui->whiteBalanceComboBox->resize(ui->whiteBalanceComboBox->width(),fontsize+1);
    ui->fNumberComboBox->resize(ui->fNumberComboBox->width() ,fontsize);
    ui->shutterSpeedComboBox->resize(ui->shutterSpeedComboBox->width(), fontsize+1);
    ui->isoSpeedRateComboBox->resize(ui->isoSpeedRateComboBox->width(), fontsize+1);
    //ui->exposureModeComboBox->resize(ui->exposureModeComboBox->width(), fontsize+1);
    ui->zoomPositionLabel->resize(ui->zoomPositionLabel->width(),fontsize+1);
    ui->pushButton_1->setIconSize(QSize(ui->pushButton_1->iconSize().width(),pushbuttonsize));
    ui->pushButton_2->setIconSize(QSize(ui->pushButton_2->iconSize().width(),pushbuttonsize));
    ui->pushButton_3->setIconSize(QSize(ui->pushButton_3->iconSize().width(),pushbuttonsize));
    ui->zoomInPushButton->setIconSize(QSize(ui->zoomInPushButton->iconSize().width(),pushbuttonsize));
    ui->zoomOutPushButton->setIconSize(QSize(ui->zoomOutPushButton->iconSize().width(),pushbuttonsize));
    ui->takePicturePushButton->setIconSize(QSize(ui->takePicturePushButton->iconSize().width(),pushbuttonsize));
    */
//int dpi=QPaintDevice::physicalDpiX();
//ui->pushButton_1->setMaximumHeight(pushbuttonsize);

/*
ui->pushButton_1->setStyleSheet("{height: 90px;}");
ui->pushButton_2->setStyleSheet("{height: 90px;}");
ui->pushButton_3->setStyleSheet("{height: 90px;}");
ui->zoomInPushButton->setStyleSheet("{height: 90px;}");
ui->zoomOutPushButton->setStyleSheet("{height: 90px;}");
ui->takePicturePushButton->setStyleSheet("{height: 90px;}");
*/
