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


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->portLineEdit->setVisible(false);
    ui->protLabel->setVisible(false);

    QDesktopWidget *desktopwidget = QApplication::desktop();
    qDebug() << "sreenGeometry:     " << desktopwidget->screenGeometry();
    qDebug() << "availableGeometry: " << desktopwidget->availableGeometry();
    QRect geo = desktopwidget->availableGeometry();
    this->setMaximumWidth(geo.width());
    this->setMaximumHeight(geo.height());
    this->setGeometry(geo);

    QGraphicsOpacityEffect * whiteBalanceComboBoxeffect = new QGraphicsOpacityEffect(ui->whiteBalanceComboBox);
    QGraphicsOpacityEffect * fNumberComboBoxeffect = new QGraphicsOpacityEffect(ui->fNumberComboBox);
    QGraphicsOpacityEffect * shutterSpeedComboBoxeffect = new QGraphicsOpacityEffect(ui->shutterSpeedComboBox);
    QGraphicsOpacityEffect * isoSpeedRateComboBoxeffect = new QGraphicsOpacityEffect(ui->isoSpeedRateComboBox);
    QGraphicsOpacityEffect * exposureModeComboBoxeffect = new QGraphicsOpacityEffect(ui->exposureModeComboBox);
    QGraphicsOpacityEffect * postViewImageSizeComboBoxeffect = new QGraphicsOpacityEffect(ui->postViewImageSizeComboBox);
    QGraphicsOpacityEffect * selfTimerComboBoxeffect = new QGraphicsOpacityEffect(ui->selfTimerComboBox);
    QGraphicsOpacityEffect * zoomComboBoxeffect = new QGraphicsOpacityEffect(ui->zoomComboBox);


    float opacity = 0.9;
    whiteBalanceComboBoxeffect->setOpacity(opacity);
    fNumberComboBoxeffect->setOpacity(opacity);
    shutterSpeedComboBoxeffect->setOpacity(opacity);
    isoSpeedRateComboBoxeffect->setOpacity(opacity);
    exposureModeComboBoxeffect->setOpacity(opacity);
    postViewImageSizeComboBoxeffect->setOpacity(opacity);
    selfTimerComboBoxeffect->setOpacity(opacity);
    zoomComboBoxeffect->setOpacity(opacity);

    ui->whiteBalanceComboBox->setGraphicsEffect(whiteBalanceComboBoxeffect);
    ui->fNumberComboBox->setGraphicsEffect(fNumberComboBoxeffect);
    ui->shutterSpeedComboBox->setGraphicsEffect(shutterSpeedComboBoxeffect);
    ui->isoSpeedRateComboBox->setGraphicsEffect(isoSpeedRateComboBoxeffect);
    ui->exposureModeComboBox->setGraphicsEffect(exposureModeComboBoxeffect);
    ui->postViewImageSizeComboBox->setGraphicsEffect(postViewImageSizeComboBoxeffect);
    ui->selfTimerComboBox->setGraphicsEffect(selfTimerComboBoxeffect);
    ui->zoomComboBox->setGraphicsEffect(zoomComboBoxeffect);



    //! [Create Object Connection to WiFi]
    networkConnection = new NetworkConnection;


    connect(networkConnection,SIGNAL(publishUrl(QString)),
            ui->urlLineEdit,SLOT(setText(QString)));
    connect(networkConnection,SIGNAL(publishPort(QString)),
            ui->portLineEdit,SLOT(setText(QString)));
    connect(networkConnection,SIGNAL(publishConnectionStatus(int,QString)),
            this,SLOT(onCameraStatusChanged(int,QString)));
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

    readSettings();
    QStringList availableNetworks = networkConnection->getAvailableNetWorks();
    qDebug() << "availableNetworks: " << availableNetworks;
    ui->configurationComboBox->addItems(availableNetworks);

    remote->setDevice(friendlyName);
    remote->initialEvent();


    this->resize(geo.width(),geo.height());
    //ui->centralWidget->resize(geo.width(),geo.height());
    ui->toolBar->setVisible(false);
    ui->mainToolBar->setVisible(false);



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
    qDebug() << "Leave Application";
    remote->stopLiveview();
    remote->stopRecMode();
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

void MainWindow::on_configurationComboBox_currentIndexChanged(QString text){
    networkConnection->setActiveNetwork(text);
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
    previewPath = QFileDialog::getExistingDirectory();
}

void MainWindow::on_urlLineEdit_textEdited(QString url){
    networkConnection->setUrl(url);
    //remote->setUrl(url);

}

void MainWindow::on_portLineEdit_textEdited(QString port){
    networkConnection->setPort(port);
    //ui->urlLineEdit->setText(networkConnection->getUrl().toString());
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

void MainWindow::on_zoomComboBox_activated(QString text){
    QByteArray param;
    param.append("\"");
    param.append(text);
    param.append("\"");
    remote->commandFabrikMethod("actZoom",remote->getMethods().value("actZoom"),param);

}

void MainWindow::on_selfTimerComboBox_activated(QString text){

    int para = text.toInt();
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

void MainWindow::addZoomComboBoxItems(QStringList items){
    foreach (QString item, items) {
        if(ui->zoomComboBox->findText(item) == -1){
            ui->zoomComboBox->addItem(item);
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
    //qDebug() << "drawPreview";
    QByteArray bytes = reply->readAll();
    QImage img;
    img.loadFromData(bytes);
    QSize size = img.size();
    QSize newSize = size/5;
    img = img.scaled(newSize);
    QPixmap pixmap = QPixmap::fromImage(img);
    previewScene->clear();
    previewScene->addPixmap(pixmap);
    ui->previewGraphicsView->setScene(previewScene);
    //ui->label->setPixmap(QPixmap::fromImage(img));
    savePreviewFile(bytes,previePicName);

}

void MainWindow::savePreviewFile(QByteArray bytes,QString previePicName){
    if(!previewPath.isEmpty()){
        QString path = previewPath;
        path.append(previePicName);
        //qDebug() << "path: " << path;
        QFile file( path );
        if( !file.open( QIODevice::WriteOnly ) ){
            qDebug() << "Unable to open";
            qDebug() << file.errorString();
            return;
        }
        if(file.write( bytes ) < 0 ){
            qDebug() << "Unable to write";
            qDebug() << file.errorString();
        }
        file.close();
    }
}

void MainWindow::drawLiveView(QByteArray bytes){
    //qDebug() << "drawLiveView";
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
    qDebug() << "number: " << number;
#endif
        QImage img;
        img.loadFromData(bytes);
        QSize size = img.size();
        QSize newSize = size/1.30;
        //QSize newSize = size;
        img = img.scaled(newSize);
        QPixmap pixmap = QPixmap::fromImage(img);
        liveviewScene->clear();
        liveviewScene->addPixmap(pixmap);
        //ui->label->setPixmap(QPixmap::fromImage(img));
        ui->LiveviewGraphicsView->setScene(liveviewScene);
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
    QSize size = settings.value("size", QSize(300, 200)).toSize();
    //! gibt geometry einProblem mit den Toolbars??
    //restoreGeometry(settings.value("geometry").toByteArray());

    restoreState(settings.value("state").toByteArray());
    resize(size);
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
    settings.setValue("size", size());
    //! gibt geometry einProblem mit den Toolbars??
    //settings.setValue("geometry", saveGeometry());
    //! die Breite und Position der DockWindows!!
    //! wenn sie nicht gefloated sind!!
    settings.setValue("state", saveState());


}


