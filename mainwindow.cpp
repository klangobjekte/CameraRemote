#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include<QUrl>
#include <QDebug>
#include "remote.h"
#include "timelapse.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene = new QGraphicsScene;
    remote = new Remote(this);
    connect(remote, SIGNAL(publishLoadPreview(QNetworkReply*,QString)),
            this, SLOT(drawPreview(QNetworkReply*,QString)));

    connect(remote,SIGNAL(publishLiveViewBytes(QByteArray)),
            this,SLOT(buildLiveStreamView(QByteArray)));

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

    ui->urlLineEdit->setText("http://192.168.122.1/sony/camera");
    ui->portLineEdit->setText("8080");


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startRecModePushButton_clicked(){
    remote->startRecMode();
}

void MainWindow::on_stopRecModePushButton_clicked(){
    remote->stopRecMode();
}

void MainWindow::on_takePicturePushButton_clicked(){
    static int number = 1;
    qDebug() << "Take Picture" << number++;
    remote->actTakePicture();
}

void MainWindow::on_startLiveViewPushButton_clicked(){
    remote->startLiveView();
}

void MainWindow::on_stopLiveViewPushButton_clicked(){
    remote->stopLiveView();
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
    remote->setUrl(url);

}

void MainWindow::on_portLineEdit_textChanged(QString port){
    remote->setPort(port);
}


void MainWindow::drawPreview(QNetworkReply *reply,QString previePicName){
    qDebug() << "drawPreview";
    QByteArray bytes = reply->readAll();
    QImage img;
    img.loadFromData(bytes);
    QSize size = img.size();
    QSize newSize = size/4;
    img = img.scaled(newSize);
    QPixmap pixmap = QPixmap::fromImage(img);
    scene->addPixmap(pixmap);
    ui->graphicsView->setScene(scene);
    //ui->label->setPixmap(QPixmap::fromImage(img));
    savePreviewFile(bytes,previePicName);

}

void MainWindow::savePreviewFile(QByteArray bytes,QString previePicName){
    if(!previewPath.isEmpty()){
        QString path = previewPath;
        path.append(previePicName);
        qDebug() << "path: " << path;
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



void MainWindow::buildLiveStreamView(QByteArray bytes){
    //qDebug() << "buildLiveStreamView";
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
        QSize newSize = size/2;
        img = img.scaled(newSize);
        QPixmap pixmap = QPixmap::fromImage(img);
        ui->label->setPixmap(QPixmap::fromImage(img));
}
