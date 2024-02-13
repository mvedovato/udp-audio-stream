#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settingsdialog.h"

#include <QMainWindow>
#include <QUdpSocket>
#include <QAudio>//These five are QT's audio processing libraries
#include <QAudioDevice>
#include <QAudioSink>
#include <QAudioSource>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioOutput>
#include <QIODevice>
#include <QDebug>
#include <QAudioDecoder>
#include <QFile>
#include <QTcpSocket>
#include <QHostInfo>
#include <QNetworkInterface>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    struct audioSend{
        char audioDataSend[7680];
        int lensSend;
    };
    struct audioRecv{
        char audioDataRecv[7680];
        int lensRecv;
    };

    bool lastUpdatedFormatFileRead();
    void setAudioFormat();
    void makeUIElementsInvisible();
    void stopStream();
    bool fileOpen();
    QString getIPAddressFromUser();
    void setIPAdressAndPortNumber(QString giveThisTargetAddress, quint16 giveThisTargetPort);

public slots:
    void readyRead();

private slots:
    void onReadyReadFileStream();
    void onReadyReadLiveStream();
    void on_pushButton_start_clicked(bool checked);
    void on_pushButton_chooseFile_clicked();
    void on_pushButton_pause_clicked();
    void on_comboBox_senderReceiver_activated();
    void on_actionOptions_triggered();



private:
    Ui::MainWindow *ui;
    QStringList pieces;
    settingsDialog *mySettingsDialog;
    QAudioFormat *format;
    QFile *file;
    QFile *lastUpdatedFormatFile;
    QAudioDevice *infoInput;
    QAudioDevice *infoOutput;
    QAudioDecoder *decoder;
    QAudioSource *input;
    QAudioSink *output;
    QIODevice *IODevice;
    QUdpSocket  *socket;
    QUdpSocket *senderSocket;
    QHostAddress *targetAddress;
    quint16 *targetPort;
    QString LocalIP;
};
#endif // MAINWINDOW_H
