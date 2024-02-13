#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QMessageBox>
#include <QDebug>

settingsDialog::settingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settingsDialog)
{
    ui->setupUi(this);
    fileFormatInfo = new QFile(this);
}

settingsDialog::~settingsDialog()
{
    delete ui;
}

//Get format preferences from user and save it to text file.
//Later, this text file will be read by MainWindow class to use as a format settings
void settingsDialog::on_pushButton_save_clicked()
{
    fileFormatInfo->setFileName(QCoreApplication::applicationDirPath() + "/lastUpdatedFormatSettings.txt");
    if (!fileFormatInfo->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Choose a file to start streaming");
        msgBox.exec();

        qDebug() << QString(QCoreApplication::applicationDirPath());
        return;
    }
    QTextStream out(fileFormatInfo);
    out << ui->comboBox_rate->currentText()+" ";
    out << ui->comboBox_channelCount->currentText()+" ";
    out << ui->comboBox_sampleSize->currentText()+" ";
    out << ui->comboBox_codec->currentText()+" ";
    out << ui->comboBox_sampleType->currentText()+" ";
    out << ui->comboBox_byteOrder->currentText();
    out.flush();
    fileFormatInfo->close();
    settingsDialog::close();
}
