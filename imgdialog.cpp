#include "imgdialog.h"
#include "ui_imgdialog.h"
#include "QMessageBox"
#include "qpainter.h"
#include <QKeyEvent>
extern std::vector<std::pair<int, int>> pts[4];
extern const int streamCount = 4;
extern pthread_mutex_t chooseImgMutexs[streamCount];

ImgDialog::ImgDialog(QWidget *parent, int index) :
    QDialog(parent),
    ui(new Ui::ImgDialog)
{
    ui->setupUi(this);
    innerIndex = index - 1;
    char imgpath[256] = {0};
    pthread_mutex_lock(&chooseImgMutexs[index-1]);
    sprintf(imgpath, "%d.jpg", index);
    QImage *image = new QImage(imgpath);
    this->setFixedSize(image->width(), image->height());
    ui->img->setGeometry(0,0, image->width(), image->height());
    pixmapSrc = QPixmap::fromImage(*image);
    pixmapDraw = QPixmap::fromImage(*image);
    ui->img->setPixmap(pixmapSrc);
    pthread_mutex_unlock(&chooseImgMutexs[index-1]);
    this->setMouseTracking(true);
    std::vector<QPoint> first;
    areas.push_back(first);
    //ui->img->installEventFilter(this);
}

void ImgDialog::mousePressEvent(QMouseEvent *event) {
    int x = this->mapFromGlobal( this->cursor().pos()).x();
    int y = this->mapFromGlobal(this->cursor().pos()).y();
    char log[256] = {0};
    QPainter painter(&pixmapDraw);
    QPen mypen;
    mypen.setWidth(3);
    mypen.setColor(Qt::green);
    painter.setPen(mypen);
    painter.drawPoint(x,y);
    ui->img->setPixmap(pixmapDraw);
    areas[areas.size()-1].push_back({x, y});
}

void ImgDialog::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        areas.clear();
        std::vector<QPoint> newArea;
        areas.push_back(newArea);
        pixmapDraw = pixmapSrc;
        drawArea();
    }
    else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        drawArea();
        std::vector<QPoint> newArea;
        areas.push_back(newArea);
    }
}

ImgDialog::~ImgDialog()
{    
    pts[innerIndex].clear();
    FILE *fp;
    if(innerIndex==0)
     fp = fopen("test1.txt","w");
    if(innerIndex==1)
     fp = fopen("test2.txt","w");
    if(innerIndex==2)
     fp = fopen("test3.txt","w");
    if(innerIndex==3)
     fp = fopen("test4.txt","w");

    for (int i=0; i < areas.size()-1; ++i) {
        printf("----------~ImgDialog(), do pts copy-----\n");
        pts[innerIndex].push_back(std::make_pair(-1, -1));
       // fprintf(fp,"%d %d\n",-1,-1);
        for(int j = 0; j < areas[i].size(); ++j) {
            pts[innerIndex].push_back(std::make_pair(areas[i][j].x(), areas[i][j].y()));
            fprintf(fp,"%d %d\n",areas[i][j].x(),areas[i][j].y());
        }
    }
    fclose(fp);
    delete ui;
}

void ImgDialog::drawArea()
{
    QPainter painter(&pixmapDraw);
    QPen mypen;
    mypen.setWidth(3);
    mypen.setColor(Qt::green);
    painter.setPen(mypen);
    QPoint pts[100] = {{0,0}};
    for (int i=0; i < areas.size(); ++i) {
        memset(pts, 0, sizeof(QPoint)*100);
        for(int j = 0; j < areas[i].size(); ++j) {
            pts[j] = areas[i][j];
        }
        painter.drawPolygon(pts, areas[i].size());
    }
    ui->img->setPixmap(pixmapDraw);
}

void ImgDialog::drawLastArea() {
    QPainter painter(&pixmapDraw);
    QPen mypen;
    mypen.setWidth(3);
    mypen.setColor(Qt::green);
    painter.setPen(mypen);
    QPoint pts[100] = {{0,0}};
    for(int j = 0; j < areas[areas.size() - 1].size(); ++j) {
        pts[j] = areas[areas.size() - 1][j];
    }
    painter.drawPolygon(pts, areas[areas.size() - 1].size());
    ui->img->setPixmap(pixmapDraw);
}

