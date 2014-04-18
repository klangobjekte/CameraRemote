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



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //! [Create Object Connection to WiFi]
    networkConnection = new NetworkConnection;


    connect(networkConnection,SIGNAL(publishUrl(QString)),
            ui->urlLineEdit,SLOT(setText(QString)));
    connect(networkConnection,SIGNAL(publishPort(QString)),
            ui->portLineEdit,SLOT(setText(QString)));
    networkConnection->setUrl("http://192.168.122.1");
    QString port;
    networkConnection->setPort(port.setNum(8080));




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

    connect(remote,SIGNAL(publishCurrentIsoSpeedRates(QString)),
            ui->isoSpeedRateComboBox,SLOT(setCurrentText(QString)));
    connect(remote,SIGNAL(publishCurrentShutterSpeed(QString)),
            ui->shutterSpeedComboBox,SLOT(setCurrentText(QString)));
    connect(remote,SIGNAL(publishCurrentFNumber(QString)),
            ui->fNumberComboBox,SLOT(setCurrentText(QString)));
    connect(remote,SIGNAL(publishCurrentWhiteBalanceModes(QString)),
            ui->whiteBalanceComboBox,SLOT(setCurrentText(QString)));

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
    connect(ui->loadPreviewPicCheckBox,SIGNAL(clicked(bool)),
            remote,SLOT(setLoadPreviewPic(bool)));

    readSettings();
    QStringList availableNetworks = networkConnection->getAvailableNetWorks();
    qDebug() << "availableNetworks: " << availableNetworks;
    ui->configurationComboBox->addItems(availableNetworks);


}

MainWindow::~MainWindow()
{
    delete timelapse;
    delete remote;
    delete previewScene;
    delete liveviewScene;
    delete networkConnection;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    qDebug() << "Leave Application";
    remote->stopLiveview();
    remote->stopRecMode();
}



void MainWindow::on_configurationComboBox_currentIndexChanged(QString text){
    networkConnection->setActiveNetwork(text);
}

void MainWindow::on_startRecModePushButton_clicked(){
    remote->startRecMode();
    //remote->startLiveview();
    //remote->getEvent("false");
    //remote->getAvailableApiList();
}

void MainWindow::on_stopRecModePushButton_clicked(){
    remote->stopRecMode();
    liveviewScene->clear();
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

void MainWindow::on_urlLineEdit_textChanged(QString url){
    networkConnection->setUrl(url);
    //remote->setUrl(url);

}

void MainWindow::on_portLineEdit_textChanged(QString port){
    networkConnection->setPort(port);
    ui->urlLineEdit->setText(networkConnection->getUrl().toString());
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
    index.append(text);
    index.append("\"");
    if(!init_on_whiteBalanceComboBox_currentTextChanged)
        remote->commandFabrikMethod("setWhiteBalance",remote->getMethods().value("setWhiteBalance"),index);
    init_on_whiteBalanceComboBox_currentTextChanged = false;
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

    //QSettings settings;
    QSettings settings("KlangObjekte.", "CameraRemote");
    networkConnection->setUrl(settings.value("url",QString("192.168.122.1")).toString());
    networkConnection->setPort(settings.value("port",8080).toString());

    QColor wColor = settings.value("waveformColor",QColor(200, 200, 200)).value< QColor >();
    QColor bColor = settings.value("backgroundColor",QColor(85, 85, 127)).value< QColor >();
    QPoint pos = settings.value("pos", QPoint(200,200)).toPoint();
    QSize size = settings.value("size", QSize(300, 200)).toSize();
    //! gibt geometry einProblem mit den Toolbars??
    //restoreGeometry(settings.value("geometry").toByteArray());
    //other->move(x() +40,y() +40);
    restoreState(settings.value("state").toByteArray());
    /*
    if(pos.x() == 200){
        pos.setX(pos.x()+binPositionOffset);
        pos.setY(pos.y()+binPositionOffset);
        binPositionOffset += 40;
        move(pos);
    }
    else{
        move(pos);
    }
    */
    //move(x() +40,y() +40);
    resize(size);



}

void MainWindow::writeSettings()
{

    //qDebug() << "write Settings: " << windowName;

    QSettings settings("KlangObjekte.", "CameraRemote");
    //QSettings settings;
    //! [] Main
    settings.setValue("url",networkConnection->getUrl().path());
    settings.setValue("port",networkConnection->getUrl().port());
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    //! gibt geometry einProblem mit den Toolbars??
    //settings.setValue("geometry", saveGeometry());
    //! die Breite und Position der DockWindows!!
    //! wenn sie nicht gefloated sind!!
    settings.setValue("state", saveState());


}


