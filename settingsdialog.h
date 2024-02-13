#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFile>

namespace Ui {
class settingsDialog;
}

class settingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit settingsDialog(QWidget *parent = nullptr);
    ~settingsDialog();

private slots:
    void on_pushButton_save_clicked();

private:
    Ui::settingsDialog *ui;
    QFile *fileFormatInfo;
};

#endif // SETTINGSDIALOG_H
