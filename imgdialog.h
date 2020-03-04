#ifndef IMGDIALOG_H
#define IMGDIALOG_H

#include <QDialog>

namespace Ui {
class ImgDialog;
}

class ImgDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImgDialog(QWidget *parent = 0, int index=0);
    ~ImgDialog();

protected:
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent* event);
    void drawArea();
    void drawLastArea();
private:
    Ui::ImgDialog *ui;
    QPixmap pixmapSrc;
    QPixmap pixmapDraw;
    std::vector<std::vector<QPoint>> areas;
    int innerIndex;
    //void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
};

#endif // IMGDIALOG_H
