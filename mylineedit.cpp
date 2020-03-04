#include "mylineedit.h"
#include "QTimer"

MyLineEdit::MyLineEdit(QWidget *parent):QLineEdit(parent)
{
}

MyLineEdit::~MyLineEdit()
{
}

void MyLineEdit::focusInEvent(QFocusEvent *e)
{
    // qDebug() << "*MyLineEdit::focusInEvent" << this->objectName();

    QPalette pGreen = QPalette();
    pGreen.setColor(QPalette::Base,Qt::gray);    //QPalette::Base 对可编辑输入框有效，还有其他类型，具体的查看文档
    setPalette(pGreen);
    QTimer::singleShot(0, this, &QLineEdit::selectAll);
    //ledSendStr->selectAll();        // 不起使用,只有在窗体获取到焦点时才有功能
    //ledSendStr->setFocus(Qt::OtherFocusReason);
}

void MyLineEdit::focusOutEvent(QFocusEvent *e)
{
    // qDebug() << "*MyLineEdit::focusOutEvent" << this->objectName();
    QLineEdit::focusOutEvent(e);
    QPalette pWhite = QPalette();
    pWhite.setColor(QPalette::Base,Qt::white);
    setPalette(pWhite);
    this->setCursorPosition(0);
    //QTimer::singleShot(0, this, &QLineEdit::clear);
}
