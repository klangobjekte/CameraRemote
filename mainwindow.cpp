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



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->portLineEdit->setVisible(false);
    ui->protLabel->setVisible(false);


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
    //remote->initialCommands();
    remote->initialCommands();



    //statusBar()->showMessage("Statusbar");


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
        break;
    case _CONNECTIONSTATE_WATING:
        statusBar()->showMessage(tr("Listening..."));
        break;
    case _CONNECTIONSTATE_CONNECTING:
        statusBar()->showMessage(tr("Connecting..."));
        break;
    case _CONNECTIONSTATE_CONNECTET:
        temp4.append(friendlyName);
        statusBar()->showMessage(temp4);
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
            //remote->startDevice();
        }
        break;
    }
}

void MainWindow::on_configurationComboBox_currentIndexChanged(QString text){
    networkConnection->setActiveNetwork(text);
}

void MainWindow::on_startRecModePushButton_clicked(){
    remote->initActEnabelMethods();
    remote->startRecMode();
}

void MainWindow::on_stopRecModePushButton_clicked(){
    remote->stopLiveview();
    remote->stopRecMode();
    liveviewScene->clear();
    previewScene->clear();
}

void MainWindow::on_takePicturePushButton_clicked(){
    static int number = 1;
    remote->actTakePicture();
}

void MainWindow::on_startLiveViewPushButton_clicked(){
    remote->getAvailableLiveviewSize(remote->getMethods().value("getAvailableLiveviewSize"));
    remote->startLiveview();
}

void MainWindow::on_stopLiveViewPushButton_clicked(){
    remote->stopLiveview();
    liveviewScene->clear();
}

void MainWindow::on_startTimerPushButton_clicked(){
    timelapse->start();
}

void MainWindow::on_stopTimerPushButton_clicked(){
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

void MainWindow::on_isoSpeedRateComboBox_currentTextChanged(QString text){
    static bool init_on_isoSpeedRateComboBox_currentTextChanged = true;
    QByteArray index;
    index.append("\"");
    index.append(text);
    index.append("\"");
    if(!init_on_isoSpeedRateComboBox_currentTextChanged)
        remote->commandFabrikMethod("setIsoSpeedRate",remote->getMethods().value("setIsoSpeedRate"),index);
    init_on_isoSpeedRateComboBox_currentTextChanged = false;
}

void MainWindow::on_shutterSpeedComboBox_currentTextChanged(QString text){
    static bool on_shutterSpeedComboBox_currentTextChanged = true;
    text.remove("\"");
    QByteArray index;
    index.append("\"");
    index.append(text);
    index.append("\"");
    if(!on_shutterSpeedComboBox_currentTextChanged)
        remote->commandFabrikMethod("setShutterSpeed",remote->getMethods().value("setShutterSpeed"),index);
    on_shutterSpeedComboBox_currentTextChanged = false;
}

void MainWindow::on_fNumberComboBox_currentTextChanged(QString text){
    static bool init_on_fNumberComboBox_currentTextChanged = true;
    QByteArray param;
    param.append("\"");
    param.append(text);
    param.append("\"");
    if(!init_on_fNumberComboBox_currentTextChanged)
        remote->commandFabrikMethod("setFNumber",remote->getMethods().value("setFNumber"),param);
    init_on_fNumberComboBox_currentTextChanged = false;
}

void MainWindow::on_whiteBalanceComboBox_currentTextChanged(QString text){
    static bool init_on_whiteBalanceComboBox_currentTextChanged = true;
    QByteArray index;
    index.append("\"");
    //index.append("Auto");
    index.append(text);
    index.append("\"");
    if(!init_on_whiteBalanceComboBox_currentTextChanged)
        remote->commandFabrikMethod("setWhiteBalance",remote->getMethods().value("setWhiteBalance"),index);
    init_on_whiteBalanceComboBox_currentTextChanged = false;
}

void MainWindow::on_exposureModeComboBox_currentTextChanged(QString text){
    static bool init_on_fNumberComboBox_currentTextChanged = true;
    QByteArray param;
    param.append("\"");
    param.append(text);
    param.append("\"");
    if(!init_on_fNumberComboBox_currentTextChanged)
        remote->commandFabrikMethod("setExposureMode",remote->getMethods().value("setExposureMode"),param);
    init_on_fNumberComboBox_currentTextChanged = false;

}

void MainWindow::addIsoSpeedRateComboBoxItems(QStringList items){
    ui->isoSpeedRateComboBox->clear();
    ui->isoSpeedRateComboBox->addItems(items);
}

void MainWindow::addfNumberComboBoxItems(QStringList items){
    ui->fNumberComboBox->clear();
    ui->fNumberComboBox->addItems(items);
}

void MainWindow::addshutterSpeedComboBox_2Items(QStringList items){
    ui->shutterSpeedComboBox->clear();
    ui->shutterSpeedComboBox->addItems(items);
}

void MainWindow::addwhiteBalanceComboBoxItems(QStringList items){
    ui->whiteBalanceComboBox->clear();
    ui->whiteBalanceComboBox->addItems(items);
}

void MainWindow::addexposureModeComboBoxItems(QStringList items){
    ui->exposureModeComboBox->clear();
    ui->exposureModeComboBox->addItems(items);
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


