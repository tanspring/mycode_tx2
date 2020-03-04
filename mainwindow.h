#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "configdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
        
private slots:
    void loadCfg();
    void reLoadCfg();
    void on_choose_clicked();

    void on_choose_2_clicked();

    void on_choose_3_clicked();

    void on_choose_4_clicked();

    void on_display_clicked();

    void on_rtmp_clicked();

    void on_display_2_clicked();

    void on_rtmp_2_clicked();

    void on_display_3_clicked();

    void on_rtmp_3_clicked();

    void on_rtmp_4_clicked();

    void on_display_4_clicked();
 void   mousedoubleclicked();
private:
    Ui::MainWindow *ui;
    QDialog *cfgDialog;
    QDialog *imgDialog;
    //pthread_t pipeStateCheckThreadId[4];
};

#endif // MAINWINDOW_H
