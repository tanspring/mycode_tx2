/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_2;
    QFrame *frame;
    QPushButton *choose;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QPushButton *choose_2;
    QPushButton *choose_3;
    QPushButton *choose_4;
    QPushButton *display;
    QPushButton *rtmp;
    QPushButton *display_2;
    QPushButton *rtmp_2;
    QPushButton *display_3;
    QPushButton *rtmp_3;
    QPushButton *rtmp_4;
    QPushButton *display_4;
    QLCDNumber *lcdNumber;
    QLCDNumber *lcdNumber_2;
    QLabel *label_5;
    QLabel *label_6;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_8;
    QFrame *frame_3;
    QGridLayout *gridLayout_5;
    QWidget *video_3;
    QWidget *video;
    QWidget *video_2;
    QWidget *video_4;
    QListWidget *listWidget;
    QMenuBar *menuBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(980, 677);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        horizontalLayout_2 = new QHBoxLayout(centralWidget);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        frame = new QFrame(centralWidget);
        frame->setObjectName(QStringLiteral("frame"));
        sizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy);
        frame->setMaximumSize(QSize(200, 16777215));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        choose = new QPushButton(frame);
        choose->setObjectName(QStringLiteral("choose"));
        choose->setGeometry(QRect(110, 100, 71, 25));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(choose->sizePolicy().hasHeightForWidth());
        choose->setSizePolicy(sizePolicy1);
        label = new QLabel(frame);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 60, 171, 17));
        label_2 = new QLabel(frame);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(10, 160, 171, 17));
        label_3 = new QLabel(frame);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(10, 260, 171, 17));
        label_4 = new QLabel(frame);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(10, 350, 181, 17));
        choose_2 = new QPushButton(frame);
        choose_2->setObjectName(QStringLiteral("choose_2"));
        choose_2->setGeometry(QRect(110, 200, 71, 25));
        sizePolicy1.setHeightForWidth(choose_2->sizePolicy().hasHeightForWidth());
        choose_2->setSizePolicy(sizePolicy1);
        choose_3 = new QPushButton(frame);
        choose_3->setObjectName(QStringLiteral("choose_3"));
        choose_3->setGeometry(QRect(110, 300, 71, 25));
        sizePolicy1.setHeightForWidth(choose_3->sizePolicy().hasHeightForWidth());
        choose_3->setSizePolicy(sizePolicy1);
        choose_4 = new QPushButton(frame);
        choose_4->setObjectName(QStringLiteral("choose_4"));
        choose_4->setGeometry(QRect(110, 400, 71, 25));
        sizePolicy1.setHeightForWidth(choose_4->sizePolicy().hasHeightForWidth());
        choose_4->setSizePolicy(sizePolicy1);
        display = new QPushButton(frame);
        display->setObjectName(QStringLiteral("display"));
        display->setGeometry(QRect(11, 100, 41, 25));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(display->sizePolicy().hasHeightForWidth());
        display->setSizePolicy(sizePolicy2);
        rtmp = new QPushButton(frame);
        rtmp->setObjectName(QStringLiteral("rtmp"));
        rtmp->setGeometry(QRect(60, 100, 41, 25));
        sizePolicy2.setHeightForWidth(rtmp->sizePolicy().hasHeightForWidth());
        rtmp->setSizePolicy(sizePolicy2);
        display_2 = new QPushButton(frame);
        display_2->setObjectName(QStringLiteral("display_2"));
        display_2->setGeometry(QRect(11, 201, 41, 25));
        rtmp_2 = new QPushButton(frame);
        rtmp_2->setObjectName(QStringLiteral("rtmp_2"));
        rtmp_2->setGeometry(QRect(60, 200, 41, 25));
        display_3 = new QPushButton(frame);
        display_3->setObjectName(QStringLiteral("display_3"));
        display_3->setGeometry(QRect(11, 301, 41, 25));
        rtmp_3 = new QPushButton(frame);
        rtmp_3->setObjectName(QStringLiteral("rtmp_3"));
        rtmp_3->setGeometry(QRect(60, 300, 41, 25));
        rtmp_4 = new QPushButton(frame);
        rtmp_4->setObjectName(QStringLiteral("rtmp_4"));
        rtmp_4->setGeometry(QRect(11, 401, 41, 28));
        display_4 = new QPushButton(frame);
        display_4->setObjectName(QStringLiteral("display_4"));
        display_4->setGeometry(QRect(60, 400, 41, 28));
        lcdNumber = new QLCDNumber(frame);
        lcdNumber->setObjectName(QStringLiteral("lcdNumber"));
        lcdNumber->setGeometry(QRect(20, 480, 131, 41));
        lcdNumber_2 = new QLCDNumber(frame);
        lcdNumber_2->setObjectName(QStringLiteral("lcdNumber_2"));
        lcdNumber_2->setGeometry(QRect(20, 560, 131, 41));
        label_5 = new QLabel(frame);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(30, 450, 111, 17));
        label_6 = new QLabel(frame);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(20, 530, 131, 17));
        display->raise();
        rtmp->raise();
        display_2->raise();
        rtmp_2->raise();
        display_3->raise();
        rtmp_3->raise();
        rtmp_4->raise();
        display_4->raise();
        choose->raise();
        label->raise();
        label_2->raise();
        label_3->raise();
        label_4->raise();
        choose_2->raise();
        choose_3->raise();
        choose_4->raise();
        lcdNumber->raise();
        lcdNumber_2->raise();
        label_5->raise();
        label_6->raise();

        horizontalLayout_2->addWidget(frame);

        frame_2 = new QFrame(centralWidget);
        frame_2->setObjectName(QStringLiteral("frame_2"));
        sizePolicy.setHeightForWidth(frame_2->sizePolicy().hasHeightForWidth());
        frame_2->setSizePolicy(sizePolicy);
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        verticalLayout_8 = new QVBoxLayout(frame_2);
        verticalLayout_8->setSpacing(6);
        verticalLayout_8->setContentsMargins(11, 11, 11, 11);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        frame_3 = new QFrame(frame_2);
        frame_3->setObjectName(QStringLiteral("frame_3"));
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        frame_3->setLineWidth(0);
        gridLayout_5 = new QGridLayout(frame_3);
        gridLayout_5->setSpacing(0);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        gridLayout_5->setContentsMargins(0, 0, 0, 0);
        video_3 = new QWidget(frame_3);
        video_3->setObjectName(QStringLiteral("video_3"));
        sizePolicy.setHeightForWidth(video_3->sizePolicy().hasHeightForWidth());
        video_3->setSizePolicy(sizePolicy);

        gridLayout_5->addWidget(video_3, 0, 1, 1, 1);

        video = new QWidget(frame_3);
        video->setObjectName(QStringLiteral("video"));
        sizePolicy.setHeightForWidth(video->sizePolicy().hasHeightForWidth());
        video->setSizePolicy(sizePolicy);

        gridLayout_5->addWidget(video, 0, 0, 1, 1);

        video_2 = new QWidget(frame_3);
        video_2->setObjectName(QStringLiteral("video_2"));
        sizePolicy.setHeightForWidth(video_2->sizePolicy().hasHeightForWidth());
        video_2->setSizePolicy(sizePolicy);

        gridLayout_5->addWidget(video_2, 1, 0, 1, 1);

        video_4 = new QWidget(frame_3);
        video_4->setObjectName(QStringLiteral("video_4"));
        sizePolicy.setHeightForWidth(video_4->sizePolicy().hasHeightForWidth());
        video_4->setSizePolicy(sizePolicy);

        gridLayout_5->addWidget(video_4, 1, 1, 1, 1);


        verticalLayout_8->addWidget(frame_3);

        listWidget = new QListWidget(frame_2);
        listWidget->setObjectName(QStringLiteral("listWidget"));
        listWidget->setMaximumSize(QSize(16777215, 200));
        //listWidget->setIconSize(QSize(180,160));

        verticalLayout_8->addWidget(listWidget);


        horizontalLayout_2->addWidget(frame_2);

        MainWindow->setCentralWidget(centralWidget);
        frame_2->raise();
        frame->raise();
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 980, 22));
        MainWindow->setMenuBar(menuBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
        choose->setText(QApplication::translate("MainWindow", "\347\202\271\351\200\211\345\214\272\345\237\237", Q_NULLPTR));
        label->setText(QApplication::translate("MainWindow", "TextLabel", Q_NULLPTR));
        label_2->setText(QApplication::translate("MainWindow", "TextLabel", Q_NULLPTR));
        label_3->setText(QApplication::translate("MainWindow", "TextLabel", Q_NULLPTR));
        label_4->setText(QApplication::translate("MainWindow", "TextLabel", Q_NULLPTR));
        choose_2->setText(QApplication::translate("MainWindow", "\347\202\271\351\200\211\345\214\272\345\237\237", Q_NULLPTR));
        choose_3->setText(QApplication::translate("MainWindow", "\347\202\271\351\200\211\345\214\272\345\237\237", Q_NULLPTR));
        choose_4->setText(QApplication::translate("MainWindow", "\347\202\271\351\200\211\345\214\272\345\237\237", Q_NULLPTR));
        display->setText(QApplication::translate("MainWindow", "\346\230\276\347\244\272", Q_NULLPTR));
        rtmp->setText(QApplication::translate("MainWindow", "\346\216\250\346\265\201", Q_NULLPTR));
        display_2->setText(QApplication::translate("MainWindow", "\346\230\276\347\244\272", Q_NULLPTR));
        rtmp_2->setText(QApplication::translate("MainWindow", "\346\216\250\346\265\201", Q_NULLPTR));
        display_3->setText(QApplication::translate("MainWindow", "\346\230\276\347\244\272", Q_NULLPTR));
        rtmp_3->setText(QApplication::translate("MainWindow", "\346\216\250\346\265\201", Q_NULLPTR));
        rtmp_4->setText(QApplication::translate("MainWindow", "\346\230\276\347\244\272", Q_NULLPTR));
        display_4->setText(QApplication::translate("MainWindow", "\346\216\250\346\265\201", Q_NULLPTR));
        label_5->setText(QApplication::translate("MainWindow", "\346\234\252\345\270\246\345\256\211\345\205\250\345\270\275\347\273\237\350\256\241", Q_NULLPTR));
        label_6->setText(QApplication::translate("MainWindow", "\351\207\215\347\202\271\345\214\272\345\237\237\351\230\262\346\212\244\347\273\237\350\256\241", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
