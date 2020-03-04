/********************************************************************************
** Form generated from reading UI file 'imgdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMGDIALOG_H
#define UI_IMGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_ImgDialog
{
public:
    QLabel *img;

    void setupUi(QDialog *ImgDialog)
    {
        if (ImgDialog->objectName().isEmpty())
            ImgDialog->setObjectName(QStringLiteral("ImgDialog"));
        ImgDialog->resize(400, 300);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ImgDialog->sizePolicy().hasHeightForWidth());
        ImgDialog->setSizePolicy(sizePolicy);
        img = new QLabel(ImgDialog);
        img->setObjectName(QStringLiteral("img"));
        img->setGeometry(QRect(80, 70, 211, 131));

        retranslateUi(ImgDialog);

        QMetaObject::connectSlotsByName(ImgDialog);
    } // setupUi

    void retranslateUi(QDialog *ImgDialog)
    {
        ImgDialog->setWindowTitle(QApplication::translate("ImgDialog", "Dialog", Q_NULLPTR));
        img->setText(QApplication::translate("ImgDialog", "TextLabel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ImgDialog: public Ui_ImgDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMGDIALOG_H
