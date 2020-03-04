/********************************************************************************
** Form generated from reading UI file 'configdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIGDIALOG_H
#define UI_CONFIGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include "mylineedit.h"

QT_BEGIN_NAMESPACE

class Ui_ConfigDialog
{
public:
    QFrame *frame1;
    QLineEdit *user;
    QLineEdit *password;
    QLineEdit *url;
    MyLineEdit *name;
    QComboBox *codec;
    QComboBox *algorithm;
    QLabel *label;
    QFrame *frame2;
    QLineEdit *user_2;
    QLineEdit *password_2;
    QLineEdit *url_2;
    QLineEdit *name_2;
    QComboBox *codec_2;
    QComboBox *algorithm_2;
    QLabel *label_2;
    QFrame *frame3;
    QLineEdit *user_3;
    QLineEdit *password_3;
    QLineEdit *url_3;
    QLineEdit *name_3;
    QComboBox *codec_3;
    QComboBox *algorithm_3;
    QLabel *label_3;
    QFrame *frame4;
    QLineEdit *user_4;
    QLineEdit *password_4;
    QLineEdit *url_4;
    QLineEdit *name_4;
    QComboBox *codec_4;
    QComboBox *algorithm_4;
    QLabel *label_4;
    QPushButton *confirm;

    void setupUi(QDialog *ConfigDialog)
    {
        if (ConfigDialog->objectName().isEmpty())
            ConfigDialog->setObjectName(QStringLiteral("ConfigDialog"));
        ConfigDialog->resize(998, 492);
        frame1 = new QFrame(ConfigDialog);
        frame1->setObjectName(QStringLiteral("frame1"));
        frame1->setGeometry(QRect(30, 10, 931, 80));
        frame1->setFrameShape(QFrame::StyledPanel);
        frame1->setFrameShadow(QFrame::Raised);
        user = new QLineEdit(frame1);
        user->setObjectName(QStringLiteral("user"));
        user->setGeometry(QRect(440, 40, 111, 25));
        password = new QLineEdit(frame1);
        password->setObjectName(QStringLiteral("password"));
        password->setGeometry(QRect(580, 40, 111, 25));
        url = new QLineEdit(frame1);
        url->setObjectName(QStringLiteral("url"));
        url->setGeometry(QRect(160, 40, 251, 25));
        name = new MyLineEdit(frame1);
        name->setObjectName(QStringLiteral("name"));
        name->setGeometry(QRect(20, 40, 101, 25));
        codec = new QComboBox(frame1);
        codec->setObjectName(QStringLiteral("codec"));
        codec->setGeometry(QRect(730, 40, 61, 25));
        algorithm = new QComboBox(frame1);
        algorithm->setObjectName(QStringLiteral("algorithm"));
        algorithm->setGeometry(QRect(820, 40, 71, 25));
        label = new QLabel(frame1);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 10, 67, 17));
        frame2 = new QFrame(ConfigDialog);
        frame2->setObjectName(QStringLiteral("frame2"));
        frame2->setGeometry(QRect(30, 120, 931, 80));
        frame2->setFrameShape(QFrame::StyledPanel);
        frame2->setFrameShadow(QFrame::Raised);
        user_2 = new QLineEdit(frame2);
        user_2->setObjectName(QStringLiteral("user_2"));
        user_2->setGeometry(QRect(440, 40, 113, 25));
        password_2 = new QLineEdit(frame2);
        password_2->setObjectName(QStringLiteral("password_2"));
        password_2->setGeometry(QRect(580, 40, 113, 25));
        url_2 = new QLineEdit(frame2);
        url_2->setObjectName(QStringLiteral("url_2"));
        url_2->setGeometry(QRect(160, 40, 251, 25));
        name_2 = new QLineEdit(frame2);
        name_2->setObjectName(QStringLiteral("name_2"));
        name_2->setGeometry(QRect(20, 40, 101, 25));
        codec_2 = new QComboBox(frame2);
        codec_2->setObjectName(QStringLiteral("codec_2"));
        codec_2->setGeometry(QRect(730, 40, 61, 25));
        algorithm_2 = new QComboBox(frame2);
        algorithm_2->setObjectName(QStringLiteral("algorithm_2"));
        algorithm_2->setGeometry(QRect(820, 40, 71, 25));
        label_2 = new QLabel(frame2);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(20, 10, 67, 17));
        frame3 = new QFrame(ConfigDialog);
        frame3->setObjectName(QStringLiteral("frame3"));
        frame3->setGeometry(QRect(30, 230, 931, 80));
        frame3->setFrameShape(QFrame::StyledPanel);
        frame3->setFrameShadow(QFrame::Raised);
        user_3 = new QLineEdit(frame3);
        user_3->setObjectName(QStringLiteral("user_3"));
        user_3->setGeometry(QRect(440, 40, 113, 25));
        password_3 = new QLineEdit(frame3);
        password_3->setObjectName(QStringLiteral("password_3"));
        password_3->setGeometry(QRect(580, 40, 113, 25));
        url_3 = new QLineEdit(frame3);
        url_3->setObjectName(QStringLiteral("url_3"));
        url_3->setGeometry(QRect(160, 40, 251, 25));
        name_3 = new QLineEdit(frame3);
        name_3->setObjectName(QStringLiteral("name_3"));
        name_3->setGeometry(QRect(20, 40, 101, 25));
        codec_3 = new QComboBox(frame3);
        codec_3->setObjectName(QStringLiteral("codec_3"));
        codec_3->setGeometry(QRect(730, 40, 61, 25));
        algorithm_3 = new QComboBox(frame3);
        algorithm_3->setObjectName(QStringLiteral("algorithm_3"));
        algorithm_3->setGeometry(QRect(820, 40, 71, 25));
        label_3 = new QLabel(frame3);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(20, 10, 67, 17));
        frame4 = new QFrame(ConfigDialog);
        frame4->setObjectName(QStringLiteral("frame4"));
        frame4->setGeometry(QRect(30, 340, 931, 80));
        frame4->setFrameShape(QFrame::StyledPanel);
        frame4->setFrameShadow(QFrame::Raised);
        user_4 = new QLineEdit(frame4);
        user_4->setObjectName(QStringLiteral("user_4"));
        user_4->setGeometry(QRect(440, 40, 113, 25));
        password_4 = new QLineEdit(frame4);
        password_4->setObjectName(QStringLiteral("password_4"));
        password_4->setGeometry(QRect(580, 40, 113, 25));
        url_4 = new QLineEdit(frame4);
        url_4->setObjectName(QStringLiteral("url_4"));
        url_4->setGeometry(QRect(160, 40, 251, 25));
        name_4 = new QLineEdit(frame4);
        name_4->setObjectName(QStringLiteral("name_4"));
        name_4->setGeometry(QRect(20, 40, 101, 25));
        codec_4 = new QComboBox(frame4);
        codec_4->setObjectName(QStringLiteral("codec_4"));
        codec_4->setGeometry(QRect(730, 40, 61, 25));
        algorithm_4 = new QComboBox(frame4);
        algorithm_4->setObjectName(QStringLiteral("algorithm_4"));
        algorithm_4->setGeometry(QRect(820, 40, 71, 25));
        label_4 = new QLabel(frame4);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(20, 10, 67, 17));
        confirm = new QPushButton(ConfigDialog);
        confirm->setObjectName(QStringLiteral("confirm"));
        confirm->setGeometry(QRect(870, 450, 89, 25));

        retranslateUi(ConfigDialog);

        QMetaObject::connectSlotsByName(ConfigDialog);
    } // setupUi

    void retranslateUi(QDialog *ConfigDialog)
    {
        ConfigDialog->setWindowTitle(QApplication::translate("ConfigDialog", "Dialog", Q_NULLPTR));
        user->setText(QString());
        password->setText(QString());
        url->setText(QString());
        name->setProperty("text", QVariant(QApplication::translate("ConfigDialog", "\346\221\204\345\203\217\345\244\2641\345\220\215\347\247\260", Q_NULLPTR)));
        codec->clear();
        codec->insertItems(0, QStringList()
         << QApplication::translate("ConfigDialog", "264", Q_NULLPTR)
         << QApplication::translate("ConfigDialog", "265", Q_NULLPTR)
        );
        algorithm->clear();
        algorithm->insertItems(0, QStringList()
         << QApplication::translate("ConfigDialog", "\345\256\211\345\205\250\345\270\275", Q_NULLPTR)
         << QApplication::translate("ConfigDialog", "\351\207\215\347\202\271\345\214\272\345\237\237", Q_NULLPTR)
        );
        label->setText(QApplication::translate("ConfigDialog", "\347\224\273\351\235\2421", Q_NULLPTR));
        user_2->setText(QString());
        password_2->setText(QString());
        url_2->setText(QString());
        name_2->setText(QApplication::translate("ConfigDialog", "\346\221\204\345\203\217\345\244\2642\345\220\215\347\247\260", Q_NULLPTR));
        codec_2->clear();
        codec_2->insertItems(0, QStringList()
         << QApplication::translate("ConfigDialog", "264", Q_NULLPTR)
         << QApplication::translate("ConfigDialog", "265", Q_NULLPTR)
        );
        algorithm_2->clear();
        algorithm_2->insertItems(0, QStringList()
         << QApplication::translate("ConfigDialog", "\345\256\211\345\205\250\345\270\275", Q_NULLPTR)
         << QApplication::translate("ConfigDialog", "\351\207\215\347\202\271\345\214\272\345\237\237", Q_NULLPTR)
        );
        label_2->setText(QApplication::translate("ConfigDialog", "\347\224\273\351\235\2422", Q_NULLPTR));
        user_3->setText(QString());
        password_3->setText(QString());
        url_3->setText(QString());
        name_3->setText(QApplication::translate("ConfigDialog", "\346\221\204\345\203\217\345\244\2643\345\220\215\347\247\260", Q_NULLPTR));
        codec_3->clear();
        codec_3->insertItems(0, QStringList()
         << QApplication::translate("ConfigDialog", "264", Q_NULLPTR)
         << QApplication::translate("ConfigDialog", "265", Q_NULLPTR)
        );
        algorithm_3->clear();
        algorithm_3->insertItems(0, QStringList()
         << QApplication::translate("ConfigDialog", "\345\256\211\345\205\250\345\270\275", Q_NULLPTR)
         << QApplication::translate("ConfigDialog", "\351\207\215\347\202\271\345\214\272\345\237\237", Q_NULLPTR)
        );
        label_3->setText(QApplication::translate("ConfigDialog", "\347\224\273\351\235\2423", Q_NULLPTR));
        user_4->setText(QString());
        password_4->setText(QString());
        url_4->setText(QString());
        name_4->setText(QApplication::translate("ConfigDialog", "\346\221\204\345\203\217\345\244\2644\345\220\215\347\247\260", Q_NULLPTR));
        codec_4->clear();
        codec_4->insertItems(0, QStringList()
         << QApplication::translate("ConfigDialog", "264", Q_NULLPTR)
         << QApplication::translate("ConfigDialog", "265", Q_NULLPTR)
        );
        algorithm_4->clear();
        algorithm_4->insertItems(0, QStringList()
         << QApplication::translate("ConfigDialog", "\345\256\211\345\205\250\345\270\275", Q_NULLPTR)
         << QApplication::translate("ConfigDialog", "\351\207\215\347\202\271\345\214\272\345\237\237", Q_NULLPTR)
        );
        label_4->setText(QApplication::translate("ConfigDialog", "\347\224\273\351\235\2424", Q_NULLPTR));
        confirm->setText(QApplication::translate("ConfigDialog", "\347\241\256\345\256\232", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ConfigDialog: public Ui_ConfigDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIGDIALOG_H
