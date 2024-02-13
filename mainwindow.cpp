#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mySettingsDialog = new settingsDialog(this);

    senderSocket = new QUdpSocket(this);
    socket = new QUdpSocket(this);
    file = new QFile(this);
    lastUpdatedFormatFile = new QFile(this);
    format = new QAudioFormat();//Define the type of audio processing
    infoInput = new QAudioDevice(QMediaDevices::defaultAudioInput());
    infoOutput = new QAudioDevice(QMediaDevices::defaultAudioOutput());
    input = new QAudioSource(*format,this);
    output = new QAudioSink(*format,this);

    MainWindow::makeUIElementsInvisible();

    connect(ui->comboBox_cast,SIGNAL(activated(int)),this,SLOT(on_comboBox_senderReceiver_activated()));
    connect(ui->comboBox_liveFileStream,SIGNAL(activated(int)),this,SLOT(on_comboBox_senderReceiver_activated()));

    QList<QHostAddress> list = QNetworkInterface::allAddresses();

    for( int nIter=0; nIter<list.count(); nIter++)

    {
        if(!list[nIter].isLoopback())
            if (list[nIter].protocol() == QAbstractSocket::IPv4Protocol )
            {
                qDebug() << list[nIter].toString();
                LocalIP = list[nIter].toString();
                ui->lineEdit_IP->setText(LocalIP);
                ui->lineEdit_IP->setModified(true);
            }

    }



}
MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::lastUpdatedFormatFileRead()
{
    lastUpdatedFormatFile->setFileName(QCoreApplication::applicationDirPath() + "/lastUpdatedFormatSettings.txt");
    QString keepFormatInfo;
    if (!lastUpdatedFormatFile->open(QIODevice::ReadOnly))
    {
        QMessageBox msgBox;
        msgBox.setText("There are no predefined format settings. Please specify the format with the settings button.");
        msgBox.exec();
        return 0;
    }
    else{
        keepFormatInfo.append(lastUpdatedFormatFile->readLine());
        pieces = keepFormatInfo.split(" ", Qt::SkipEmptyParts);              //VER ESTO QUE HACE Y REEMPLAZAR
        lastUpdatedFormatFile->close();
        return 1;
    }
}



void MainWindow::setAudioFormat()
{
    if(MainWindow::lastUpdatedFormatFileRead()==1){
        format->setSampleRate(8000);            //HARDCODEADO
        format->setChannelCount(1);             //HARDCODEADO
        //format->setSampleSize(pieces[2].toInt());//Set the sample size, 8 is also OK, but the sender and receiver must match
        format->setSampleFormat(QAudioFormat::UInt8);//Set the sample size, 8 is also OK, but the sender and receiver must match

        //format->setCodec("audio/pcm");//Set to PCM encoding           //VER QUE CODIFICACION SE PUEDE USAR Y SI APLICA TODO LO QUE SIGUE
        if(pieces[4]=="SignedInt"){
            format->setSampleFormat(QAudioFormat::Int16);
        }
        else if(pieces[4]=="UnSignedInt"){
            format->setSampleFormat(QAudioFormat::Unknown);
        }
        else if(pieces[4]=="Float"){
            format->setSampleFormat(QAudioFormat::Float);
        }

        //VER DONDE ESTA ESTO EN QT6
     /*   if(pieces[5]=="LittleEndian"){
            format->setByteOrder(QAudioFormat::LittleEndian);//Set the data type of Xiaowei
        }
        else if(pieces[5]=="BigEndian"){
            format->setByteOrder(QAudioFormat::BigEndian);//Set the data type of Xiaowei
        }
    */
        if (!infoInput->isFormatSupported(*format)){
            *format = infoInput->preferredFormat();
        }

        if (!infoOutput->isFormatSupported(*format)){
            *format = infoOutput->preferredFormat();
        }

        input = new QAudioSource(*format,this);
        output = new QAudioSink(*format,this);
    }
}

//Receive audio data from socket and play
void MainWindow::readyRead()
{
    qDebug()<<"Audio is being received..."<<Qt::endl;
    audioRecv ap;
    memset(&ap,0,sizeof(ap));
    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram((char*)&ap,sizeof(ap),&sender,&senderPort);
    IODevice->write(ap.audioDataRecv,ap.lensRecv);
    ui->textBrowser->setPlainText("Message from: "+sender.toString()+"\n"+
                                  "Message port: "+QString::number(senderPort)+"\n"+
                                  "Message size:"+QString::number(ap.lensRecv)+"\n"+
                                  "Message data:"+ap.audioDataRecv+"\n");
}

//Read audio data from file and send it
void MainWindow::onReadyReadFileStream()
{
    qDebug()<<"It's sending audio!"<<Qt::endl;
    audioSend ap;
    memset(&ap,0,sizeof(ap));
    QByteArray dummy;
    dummy = IODevice->readAll();
    ap.lensSend = file->read(ap.audioDataSend,format->bytesForFrames(format->framesForDuration(40000)));//Read audio
    //qDebug() << ap.lensSend;
    ui->textBrowser->setPlainText(ap.audioDataSend);
    senderSocket->writeDatagram((const char*)&ap,sizeof(ap),*targetAddress,*targetPort);
    //Send this structure to the target host, the port and the IP are declared in the if statements
}

//Read audio data from inputDevice and send it
void MainWindow::onReadyReadLiveStream()
{
    qDebug()<<"It's sending audio!"<<Qt::endl;
    audioSend ap;
    memset(&ap,0,sizeof(ap));
    ap.lensSend = IODevice->read(ap.audioDataSend,format->bytesForFrames(format->framesForDuration(40000)));//Read audio
    ui->textBrowser->setPlainText(ap.audioDataSend);
    senderSocket->writeDatagram((const char*)&ap,sizeof(ap),*targetAddress,*targetPort);
    //Send this structure to the target host, the port and the IP are declared in the if statements
}

void MainWindow::makeUIElementsInvisible()
{
    ui->pushButton_start->setVisible(false);
    ui->pushButton_chooseFile->setVisible(false);
    ui->pushButton_pause->setVisible(false);
    ui->comboBox_senderReceiver->setVisible(false);
    ui->comboBox_cast->setVisible(false);
    ui->lineEdit_IP->setVisible(false);
}

//İf file could not open, then return a message
bool MainWindow::fileOpen()
{
    if (!file->open(QIODevice::ReadOnly))
    {
        ui->pushButton_start->setChecked(false);
        QMessageBox msgBox;
        msgBox.setText("Choose a file to start streaming");
        msgBox.exec();
        return 0;
    }
    return 1;
}

QString MainWindow::getIPAddressFromUser()
{
    if(!ui->lineEdit_IP->isModified())
    {
        ui->pushButton_start->setChecked(false);
        QMessageBox msgBox;
        msgBox.setText("Enter a valid target IP to start streaming");
        msgBox.exec();
        return 0;
    }

    QString enteredIP = ui->lineEdit_IP->text();
    return enteredIP;
}


//Set IP and Port for Multicast or Unicast sender
void MainWindow::setIPAdressAndPortNumber(QString giveThisTargetAddress, quint16 giveThisTargetPort)
{
    targetAddress = new QHostAddress(giveThisTargetAddress);
    targetPort = new quint16(giveThisTargetPort);
}

//Stop everyting until restart
void MainWindow::stopStream()
{
    //disconnect(inputDevice, SIGNAL(readyRead()), 0, 0);
    //disconnect(socket, SIGNAL(readyRead()),this,SLOT(readyRead()));
    if(file->isOpen()){
        file->close();
    }
    //else if(IODevice->isOpen()){
    //    IODevice->close();
    //}
    else if(socket->state()==QAbstractSocket::BoundState){
        socket->leaveMulticastGroup(QHostAddress("224.0.0.2"));//Leave the multicast group ip: 224.0.0.2
    }
    else if(socket->isOpen()){
        socket->disconnect();
        socket->disconnectFromHost();
        socket->close();
        socket->deleteLater();
    }
    else if(senderSocket->isOpen()){
        senderSocket->disconnect();
        senderSocket->disconnectFromHost();
        senderSocket->close();
        senderSocket->deleteLater();
    }
    else if(!(input->state()==QAudio::StoppedState)){
        input->stop();
    }
    else if(!(output->state()==QAudio::StoppedState)){
        output->stop();
    }
    return;
}

//"İf statements" are declared here for different combinations of comboBoxes and buttons
void MainWindow::on_pushButton_start_clicked(bool checked)
{
    if (ui->comboBox_senderReceiver->currentText() == "Sender"
            && (ui->comboBox_cast->currentText() == "Unicast")
            && ui->comboBox_liveFileStream->currentText() == "File Stream"
            && ui->pushButton_start->isChecked()==true
            && MainWindow::fileOpen()==1
            && MainWindow::lastUpdatedFormatFileRead()==1)
    {
        MainWindow::setAudioFormat();
        MainWindow::setIPAdressAndPortNumber(MainWindow::getIPAddressFromUser(),45000);
        IODevice = input->start();//input starts to read the input audio signal and writes it into QIODevice, here is inputDevice
        connect(IODevice,SIGNAL(readyRead()),this,SLOT(onReadyReadFileStream()));//Slot function, when inputDevice receives the audio data written by input,
        ui->pushButton_start->setText("Stop");                                  //it calls the onReadyRead function to send the data to the target host
    }
    else if (ui->comboBox_senderReceiver->currentText() == "Sender"
             && (ui->comboBox_cast->currentText() == "Multicast")
             && ui->comboBox_liveFileStream->currentText() == "File Stream"
             && ui->pushButton_start->isChecked()==true
             && MainWindow::fileOpen()==1
             && MainWindow::lastUpdatedFormatFileRead()==1)
    {
        MainWindow::setAudioFormat();
        MainWindow::setIPAdressAndPortNumber("224.0.0.2",9999);
        IODevice = input->start();//input starts to read the input audio signal and writes it into QIODevice, here is inputDevice
        connect(IODevice,SIGNAL(readyRead()),this,SLOT(onReadyReadFileStream()));//Slot function, when inputDevice receives the audio data written by input,
        ui->pushButton_start->setText("Stop");                                  //it calls the onReadyRead function to send the data to the target host
    }
    else if (ui->comboBox_senderReceiver->currentText() == "Sender"
            && (ui->comboBox_cast->currentText() == "Unicast")
            && ui->comboBox_liveFileStream->currentText() == "Live Stream"
            && ui->pushButton_start->isChecked()==true
            && MainWindow::lastUpdatedFormatFileRead()==1)
    {
        //MainWindow::fileOpen();
        MainWindow::setAudioFormat();
        MainWindow::setIPAdressAndPortNumber(MainWindow::getIPAddressFromUser(),45000);
        IODevice = input->start();//input starts to read the input audio signal and writes it into QIODevice, here is inputDevice
        connect(IODevice,SIGNAL(readyRead()),this,SLOT(onReadyReadLiveStream()));//Slot function, when inputDevice receives the audio data written by input,
        ui->pushButton_start->setText("Stop");                                  //it calls the onReadyRead function to send the data to the target host
    }
    else if (ui->comboBox_senderReceiver->currentText() == "Sender"
             && (ui->comboBox_cast->currentText() == "Multicast")
             && ui->comboBox_liveFileStream->currentText() == "Live Stream"
             && ui->pushButton_start->isChecked()==true
             && MainWindow::lastUpdatedFormatFileRead()==1)
    {
        //MainWindow::fileOpen();
        MainWindow::setAudioFormat();
        MainWindow::setIPAdressAndPortNumber("224.0.0.2",9999);
        IODevice = input->start();//input starts to read the input audio signal and writes it into QIODevice, here is inputDevice
        connect(IODevice,SIGNAL(readyRead()),this,SLOT(onReadyReadLiveStream()));//Slot function, when inputDevice receives the audio data written by input,
        ui->pushButton_start->setText("Stop");                                  //it calls the onReadyRead function to send the data to the target host
    }
    else if (ui->comboBox_senderReceiver->currentText() == "Receiver"
             && ui->comboBox_cast->currentText() == "Unicast"
             && ui->pushButton_start->isChecked()==true
             && MainWindow::lastUpdatedFormatFileRead()==1)
    {
        MainWindow::setAudioFormat();
        IODevice = output->start();//Start playing
        socket->bind(QHostAddress::AnyIPv4,45000);
        connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));//Slot function, when socket has data sent by onReadyRead function,
        ui->pushButton_start->setText("Stop");                           //it calls the readyRead function to receive the data
    }
    else if (ui->comboBox_senderReceiver->currentText() == "Receiver"
             && ui->comboBox_cast->currentText() == "Multicast"
             && ui->pushButton_start->isChecked()==true
             && MainWindow::lastUpdatedFormatFileRead()==1)
    {
        MainWindow::setAudioFormat();
        IODevice = output->start();//Start playing
        socket->bind(QHostAddress(QHostAddress::AnyIPv4),9999,QUdpSocket::ReuseAddressHint|QUdpSocket::ShareAddress);
        socket->joinMulticastGroup(QHostAddress("224.0.0.2"));//Join the multicast group ip: 224.0.0.2
        connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));//Slot function, when socket has data sent by onReadyRead function,
        ui->pushButton_start->setText("Stop");                           //it calls the readyRead function to receive the data
    }
    else if (ui->comboBox_senderReceiver->currentText() == "Choose Type"
             && ui->pushButton_start->isChecked()==true)
    {
        ui->pushButton_start->setChecked(false);
        QMessageBox msgBox;
        msgBox.setText("Please choose type");
        msgBox.exec();
    }
    else if (ui->comboBox_cast->currentText() == "Choose Cast"
             && ui->pushButton_start->isChecked()==true)
    {
        ui->pushButton_start->setChecked(false);
        QMessageBox msgBox;
        msgBox.setText("Please choose cast");
        msgBox.exec();
    }
    else{
        ui->pushButton_start->setChecked(false);
        ui->pushButton_start->setText("Start");
        ui->pushButton_pause->setChecked(false);
        ui->pushButton_pause->setText("Pause");
        MainWindow::stopStream();
    }
    qDebug() << checked;
}

//User can choose a local file with QFileDialog
void MainWindow::on_pushButton_chooseFile_clicked()
{
    QString filter = "Audio File (*.wav *.bin)";
    QString fileName = QFileDialog::getOpenFileName(this, "Open a file", "C://", filter);
    file->setFileName(fileName);
}

//Pause button: When clicked, the text turns into "Resume" and it stays like that until "Resume" button is clicked
void MainWindow::on_pushButton_pause_clicked()
{
    if (ui->pushButton_pause->isChecked()==true
            && ui->comboBox_senderReceiver->currentText() == "Sender"
            && (ui->comboBox_cast->currentText() == "Multicast"||"Unicast")){
        input->stop();
        ui->pushButton_pause->setText("Resume");
    }
    else if(ui->comboBox_senderReceiver->currentText() == "Sender"
            && (ui->comboBox_cast->currentText() == "Multicast"||"Unicast")
            && ui->pushButton_start->isChecked()==true
            && ui->pushButton_pause->isChecked()==false)
    {
        IODevice = input->start();
        connect(IODevice,SIGNAL(readyRead()),this,SLOT(onReadyReadFileStream()));
        ui->pushButton_pause->setText("Pause");
    }
    else if(ui->comboBox_senderReceiver->currentText() == "Receiver"
            && (ui->comboBox_cast->currentText() == "Multicast"||"Unicast")
            && ui->pushButton_start->isChecked()==true
            && ui->pushButton_pause->isChecked()==true)
    {
        output->suspend();
        ui->pushButton_pause->setText("Resume");
    }
    else if(ui->comboBox_senderReceiver->currentText() == "Receiver"
            && (ui->comboBox_cast->currentText() == "Multicast"||"Unicast")
            && ui->pushButton_start->isChecked()==true
            && ui->pushButton_pause->isChecked()==false)
    {
        output->resume();
        //connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
        ui->pushButton_pause->setText("Pause");
    }
    else{
        ui->pushButton_pause->setText("Pause");
    }
}

//When receiver selected, make "Choose File" button unvisible
//Also, when sender sender and multicast selected, make "Enter Target IP" linEdit area visible
void MainWindow::on_comboBox_senderReceiver_activated()
{
    if(ui->comboBox_liveFileStream->currentText() == "File Stream"){
        ui->pushButton_start->setVisible(true);
        ui->pushButton_chooseFile->setVisible(true);
        ui->pushButton_pause->setVisible(true);
        ui->comboBox_senderReceiver->setVisible(true);
        ui->comboBox_cast->setVisible(true);
        if(ui->comboBox_senderReceiver->currentText() == "Sender"){
            if(ui->comboBox_cast->currentText() == "Unicast"){
                ui->lineEdit_IP->setVisible(true);
            }
            else if (ui->comboBox_cast->currentText() == "Multicast"){
                ui->lineEdit_IP->setVisible(false);
            }
        }
        else if(ui->comboBox_senderReceiver->currentText() == "Receiver"){
            ui->pushButton_chooseFile->setVisible(false);
            ui->lineEdit_IP->setVisible(false);
        }
    }
    else if(ui->comboBox_liveFileStream->currentText() == "Live Stream"){
        ui->pushButton_start->setVisible(true);
        ui->pushButton_chooseFile->setVisible(false);
        ui->pushButton_pause->setVisible(false);
        ui->comboBox_senderReceiver->setVisible(true);
        ui->comboBox_cast->setVisible(true);
        if(ui->comboBox_senderReceiver->currentText() == "Sender"){
            if(ui->comboBox_cast->currentText() == "Unicast"){
                ui->lineEdit_IP->setVisible(true);
            }
            else if (ui->comboBox_cast->currentText() == "Multicast"){
                ui->lineEdit_IP->setVisible(false);
            }
        }
        else if(ui->comboBox_senderReceiver->currentText() == "Receiver"){
            ui->pushButton_chooseFile->setVisible(false);
            ui->lineEdit_IP->setVisible(false);
        }
    }
    else{
        MainWindow::makeUIElementsInvisible();
    }
}

void MainWindow::on_actionOptions_triggered()
{

    mySettingsDialog->show();
}




