#include "configdialog.h"
#include "ui_configdialog.h"
#include "QSettings"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    setOrder();
    setAttribute(Qt::WA_DeleteOnClose);
    QSettings *configIniRead = new QSettings("./cfg/streamer.ini", QSettings::IniFormat);
    configIniRead->setIniCodec("UTF-8");

    ui->name->setText(configIniRead->value("/video1/name").toString());
    ui->url->setText(configIniRead->value("/video1/url").toString());
    ui->user->setText(configIniRead->value("/video1/user").toString());
    ui->password->setText(configIniRead->value("/video1/password").toString());
    ui->codec->setCurrentIndex((configIniRead->value("/video1/codec_index").toString().toInt()));
    ui->algorithm->setCurrentIndex((configIniRead->value("/video1/algorithm_index")
                                    .toString().toInt()));

    ui->name_2->setText(configIniRead->value("/video2/name").toString());
    ui->url_2->setText(configIniRead->value("/video2/url").toString());
    ui->user_2->setText(configIniRead->value("/video2/user").toString());
    ui->password_2->setText(configIniRead->value("/video2/password").toString());
    ui->codec_2->setCurrentIndex((configIniRead->value("/video2/codec_index").toString().toInt()));
    ui->algorithm_2->setCurrentIndex((configIniRead->value("/video2/algorithm_index")
                                    .toString().toInt()));

    ui->name_3->setText(configIniRead->value("/video3/name").toString());
    ui->url_3->setText(configIniRead->value("/video3/url").toString());
    ui->user_3->setText(configIniRead->value("/video3/user").toString());
    ui->password_3->setText(configIniRead->value("/video3/password").toString());
    ui->codec_3->setCurrentIndex((configIniRead->value("/video3/codec_index").toString().toInt()));
    ui->algorithm_3->setCurrentIndex((configIniRead->value("/video3/algorithm_index")
                                    .toString().toInt()));

    ui->name_4->setText(configIniRead->value("/video4/name").toString());
    ui->url_4->setText(configIniRead->value("/video4/url").toString());
    ui->user_4->setText(configIniRead->value("/video4/user").toString());
    ui->password_4->setText(configIniRead->value("/video4/password").toString());
    ui->codec_4->setCurrentIndex((configIniRead->value("/video4/codec_index").toString().toInt()));
    ui->algorithm_4->setCurrentIndex((configIniRead->value("/video4/algorithm_index")
                                    .toString().toInt()));

    delete configIniRead;

}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::setOrder()
{
     QWidget::setTabOrder(ui->name, ui->url);
     QWidget::setTabOrder(ui->url, ui->user);
     QWidget::setTabOrder(ui->user, ui->password);
     QWidget::setTabOrder(ui->password, ui->codec);
     QWidget::setTabOrder(ui->codec, ui->algorithm);

     QWidget::setTabOrder(ui->algorithm, ui->name_2);

     QWidget::setTabOrder(ui->name_2, ui->url_2);
     QWidget::setTabOrder(ui->url_2, ui->user_2);
     QWidget::setTabOrder(ui->user_2, ui->password_2);
     QWidget::setTabOrder(ui->password_2, ui->codec_2);
     QWidget::setTabOrder(ui->codec_2, ui->algorithm_2);

      QWidget::setTabOrder(ui->algorithm_2, ui->name_3);

     QWidget::setTabOrder(ui->name_3, ui->url_3);
     QWidget::setTabOrder(ui->url_3, ui->user_3);
     QWidget::setTabOrder(ui->user_3, ui->password_3);
     QWidget::setTabOrder(ui->password_3, ui->codec_3);
     QWidget::setTabOrder(ui->codec_3, ui->algorithm_3);

     QWidget::setTabOrder(ui->algorithm_3, ui->name_4);

     QWidget::setTabOrder(ui->name_4, ui->url_4);
     QWidget::setTabOrder(ui->url_4, ui->user_4);
     QWidget::setTabOrder(ui->user_4, ui->password_4);
     QWidget::setTabOrder(ui->password_4, ui->codec_4);
     QWidget::setTabOrder(ui->codec_4, ui->algorithm_4);

     QWidget::setTabOrder(ui->algorithm_4, ui->confirm);


}

void ConfigDialog::on_confirm_clicked()
{
    QSettings *configIniWrite = new QSettings("./cfg/streamer.ini", QSettings::IniFormat);
    configIniWrite->setIniCodec("UTF-8");
    bool cfgstate = true;
    if((ui->name->text().isEmpty()) || (ui->name->text() == "摄像头名称")) {
     configIniWrite->setValue("/video1/name", "摄像头1名称");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video1/name", ui->name->text());
    }
    if((ui->url->text().isEmpty()) || (ui->url->text() == "请输入url")) {
     configIniWrite->setValue("/video1/url", "请输入url");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video1/url", ui->url->text());
    }
    if((ui->user->text().isEmpty()) || (ui->user->text() == "请输入用户名")) {
     configIniWrite->setValue("/video1/user", "请输入用户名");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video1/user", ui->user->text());
    }
    if((ui->password->text().isEmpty()) || (ui->password->text() == "请输入密码")) {
     configIniWrite->setValue("/video1/password", "请输入密码");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video1/password", ui->password->text());
    }
    configIniWrite->setValue("/video1/codec_index", ui->codec->currentIndex());
    configIniWrite->setValue("/video1/codec", ui->codec->currentText());
    configIniWrite->setValue("/video1/algorithm_index", ui->algorithm->currentIndex());
    configIniWrite->setValue("/video1/algorithm", ui->algorithm->currentText());
    configIniWrite->setValue("/video1/cfgstate", cfgstate);

    cfgstate = true;
    if((ui->name_2->text().isEmpty()) || (ui->name_2->text() == "摄像头2名称")) {
     configIniWrite->setValue("/video2/name", "摄像头2名称");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video2/name", ui->name_2->text());
    }
    if((ui->url_2->text().isEmpty()) || (ui->url_2->text() == "请输入url")) {
     configIniWrite->setValue("/video2/url", "请输入url");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video2/url", ui->url_2->text());
    }
    if((ui->user_2->text().isEmpty()) || (ui->user_2->text() == "请输入用户名")) {
     configIniWrite->setValue("/video2/user", "请输入用户名");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video2/user", ui->user_2->text());
    }
    if((ui->password_2->text().isEmpty()) || (ui->password_2->text() == "请输入密码")) {
     configIniWrite->setValue("/video2/password", "请输入密码");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video2/password", ui->password_2->text());
    }
    configIniWrite->setValue("/video2/codec_index", ui->codec_2->currentIndex());
    configIniWrite->setValue("/video2/codec", ui->codec_2->currentText());
    configIniWrite->setValue("/video2/algorithm_index", ui->algorithm_2->currentIndex());
    configIniWrite->setValue("/video2/algorithm", ui->algorithm_2->currentText());
    configIniWrite->setValue("/video2/cfgstate", cfgstate);

    cfgstate = true;
    if((ui->name_3->text().isEmpty()) || (ui->name_3->text() == "摄像头3名称")) {
     configIniWrite->setValue("/video3/name", "摄像头3名称");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video3/name", ui->name_3->text());
    }
    if((ui->url_3->text().isEmpty()) || (ui->url_3->text() == "请输入url")) {
     configIniWrite->setValue("/video3/url", "请输入url");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video3/url", ui->url_3->text());
    }
    if((ui->user_3->text().isEmpty()) || (ui->user_3->text() == "请输入用户名")) {
     configIniWrite->setValue("/video3/user", "请输入用户名");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video3/user", ui->user_3->text());
    }
    if((ui->password_3->text().isEmpty()) || (ui->password_3->text() == "请输入密码")) {
     configIniWrite->setValue("/video3/password", "请输入密码");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video3/password", ui->password_3->text());
    }
    configIniWrite->setValue("/video3/codec_index", ui->codec_3->currentIndex());
    configIniWrite->setValue("/video3/codec", ui->codec_3->currentText());
    configIniWrite->setValue("/video3/algorithm_index", ui->algorithm_3->currentIndex());
    configIniWrite->setValue("/video3/algorithm", ui->algorithm_3->currentText());
    configIniWrite->setValue("/video3/cfgstate", cfgstate);

    cfgstate = true;
    if((ui->name_4->text().isEmpty()) || (ui->name_4->text() == "摄像头4名称")) {
     configIniWrite->setValue("/video4/name", "摄像头4名称");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video4/name", ui->name_4->text());
    }
    if((ui->url_4->text().isEmpty()) || (ui->url_4->text() == "请输入url")) {
     configIniWrite->setValue("/video4/url", "请输入url");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video4/url", ui->url_4->text());
    }
    if((ui->user_4->text().isEmpty()) || (ui->user_4->text() == "请输入用户名")) {
     configIniWrite->setValue("/video4/user", "请输入用户名");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video4/user", ui->user_4->text());
    }
    if((ui->password_4->text().isEmpty()) || (ui->password_4->text() == "请输入密码")) {
     configIniWrite->setValue("/video4/password", "请输入密码");
     cfgstate = false;
    } else {
     configIniWrite->setValue("/video4/password", ui->password_4->text());
    }
    configIniWrite->setValue("/video4/codec_index", ui->codec_4->currentIndex());
    configIniWrite->setValue("/video4/codec", ui->codec_4->currentText());
    configIniWrite->setValue("/video4/algorithm_index", ui->algorithm_4->currentIndex());
    configIniWrite->setValue("/video4/algorithm", ui->algorithm_4->currentText());
    configIniWrite->setValue("/video4/cfgstate", cfgstate);

    delete configIniWrite;
    this->accept();
}
