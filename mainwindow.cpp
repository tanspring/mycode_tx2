#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDir"
#include "QSettings"
#include "imgdialog.h"
#include "qpoint.h"
#include <gst/gst.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/tracking.hpp>
#include <pthread.h>
#include <stdio.h>
#include <gst/video/video.h>
#include <cairo.h>
#include <cairo-gobject.h>
#include <glib.h>
#include "mainwindow.h"
#include <QApplication>
#include "mainAi.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include "inters.h"
#include <QMessageBox>

#include <vlc/vlc.h>


#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

#define USE_WEBCFG
using namespace nvinfer1;
using namespace nvuffparser;

const int streamCount = 4;//支持4个视频
std::string rtspUrls[streamCount] = {"", "", "", ""};
int rtsCodes[streamCount] = {264, 264, 264, 264};
const std::vector<std::string> arithmetics = {"安全帽检测", "重点区域防护"};

cv::Mat img_safehat_pre[streamCount];
cv::Mat img_protectArea_pre[streamCount];
double  safehat_mean = 0.0;
float protectArea_mean=0.0;

/*float safetyHatAnchor[] = {6,8,10,13,15,20,23,29,32,39,42,52,58,73,87,119,134,188};
const vector<int> safetyHatShapes[3] = {{1, 76, 76,21}, {1, 38, 38,21},{1, 19, 19,21}};
float safetyHatStride[3] = {8,16,32};
int safetyHatClassNum = 2;*/

/*安全帽检测算法*/
float safetyHatAnchor[] = {0.875,1.125,1.75,2.25,1.625,2.000,3.125,3.8125};
const vector<int> safetyHatShapes[2] = {{1, 52, 52,14}, {1, 26, 26,14}};
float safetyHatStride[2] = {8,16};
int safetyHatClassNum = 2; 
const char*  safetyHatEnginePath = "safetyHat.engine";

/*重点区域防护算法*/
//float protectAreaAnchor[] = {8,16,15,27,18,46,27,52,34,83,45,122,73,142,91,214,175,268};
float protectAreaAnchor[] = {0.875,2.125,1.375,3.25,1.1875,2.9375,1.9375,4.9375,1.5625,3.969,2.8125,6.21875};
const vector<int> protectAreaShapes[3] = {{1, 52, 52, 12}, {1, 26, 26, 12},{1, 13, 13, 12}};
float protectAreaStride[3] = {8,16,32};
int protectAreaClassNum = 1;
const char* protectAreaEnginePath = "protectArea1.engine";

const int protectAreaImgSize = 416;
const int safetyHatImgSize = 416;

//const int safetyHatImgSize = 608;
int arithmeticFlag[streamCount] = {-1, -1, -1, -1};	//算法flag，arithmetics的index
std::string imgPaths[streamCount] = {"", "", "", ""};//点选区域时用的img
QString cameras[streamCount] = {"", "", "", ""};//摄像头名
QString cameraIds[streamCount] = {"", "", "", ""};//摄像头id
QString camera_arithmeticId[streamCount] = {"", "", "", ""};//摄像头对应的算法id
GstBuffer* predictBuffers[] = {NULL, NULL, NULL, NULL};//预测结果对应的帧数据
GstBuffer* imgBuffers[] = {NULL, NULL, NULL, NULL};//appsink收到的当前帧数据
std::vector<std::pair<int, int>> pts[4]; //点选区域坐标 

pthread_mutex_t bufMutexs[streamCount];//保护imgBuffers
pthread_mutex_t predictBufMutexs[streamCount];//保护predictBuffers
pthread_mutex_t predictMutexs[streamCount];//保护predictRectss
pthread_mutex_t cvMutexs[streamCount];//保护cvRectss
pthread_mutex_t chooseImgMutexs[streamCount];//保护点选图片
pthread_mutex_t ptsMutexs[streamCount];//保护点选区域坐标
pthread_mutex_t uiMutex;//保护ui list

std::vector<cv::Rect2d> predictRectss[streamCount];//预测框坐标
std::vector<int> predictClss[streamCount];//预测类型集
std::vector<cv::Rect2d> cvRectss[streamCount];//cv框坐标
std::vector<int> cvClss[streamCount];//cv类型集

std::vector<int> predictClss_pre[streamCount];//预测类型集

GstElement* pipelines[streamCount];//pipeline
Ui::MainWindow *gUI = NULL;//主界面
const int listViewNums = 4;//ui list显示条目
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");//sqlite实例


int  protectAreaImag_count = 0;
int  safetyHatImag_count = 0;

//播放声音
 libvlc_instance_t *inst;
 libvlc_media_player_t *safehat_mp;
libvlc_media_t *safehat_m;
libvlc_media_player_t *protectarea_mp;
libvlc_media_t *protectarea_m;

int     safehat_flag=0;
int     protectarea_flag=0;
int     safehat_cnt=0;
int     protectarea_cnt=0;


typedef struct _cameraInfo
{
    QString cameraId;
    QString arithmeticId;
    QString cameraName;
    QString arithmeticName;
    QString IOPath;
    QString encodingType;
    QString cameraUser;
    QString cameraPassword;
    QString cameraType;
    QString     originalPath;
    QString     imgPath;
    int     cameraNumber;
    int     state;
    int     useIs;
}cameraInfo;
QVector<cameraInfo> infoVect;

/*
	加载网络配置，读取摄像头信息
*/
int loadWebCfg() {
    db.setDatabaseName("/home/yskj/data/www/sqlite/tileDB/zhgd-ai.db");

    if (!db.open())
    {
        g_print("can not open db file");
        return -1;
    }
    QSqlQuery query;
    query.prepare("SELECT camera.name as cameraName, camera.originalPath as originalPath, camera.imgPath as imgPath, \
                  camera.encodingType as encodingType, camera.cameraUser as cameraUser, camera.cameraPassword as cameraPassword, \
                  camera.id as cameraId, arithmetic.id as arithmeticId, arithmetic.name as arithmeticName \
                  from camera join arithmetic,  camera_arithmetic where camera.id=camera_arithmetic.cameraId and arithmetic.id=camera_arithmetic.arithmeticId");
            query.exec();
            int cameraIndex = 0;
    while (query.next())
    {
        bool bsave = false;
        cameraInfo tmp;
        tmp.arithmeticName = query.value("arithmeticName").toString();
        for(int i = 0; i < arithmetics.size(); ++i){
            if(0 == arithmetics[i].compare((const char*)tmp.arithmeticName.toLocal8Bit())) {
                bsave = true;
                break;
            }
        }
        if(bsave)
        {
            tmp.cameraId = query.value("cameraId").toString();
            tmp.arithmeticId = query.value("arithmeticId").toString();
            tmp.cameraName = query.value("cameraName").toString();
            tmp.originalPath = query.value("originalPath").toString();
            tmp.imgPath = query.value("imgPath").toString();
            tmp.encodingType = query.value("encodingType").toString();
            tmp.cameraUser = query.value("cameraUser").toString();
            tmp.cameraPassword = query.value("cameraPassword").toString();
            infoVect.push_back(tmp);
            ++cameraIndex;
        }
        if(cameraIndex == streamCount) {
            break;
        }
    }

    for (int i=0; i<infoVect.size(); i++)
    {
        printf("-------------%s\n", (const char *)infoVect[i].cameraName.toLocal8Bit());
        cameras[i] =  infoVect[i].cameraName;
        cameraIds[i] = infoVect[i].cameraId;
        camera_arithmeticId[i] = infoVect[i].arithmeticId;
        imgPaths[i] =  std::string((const char *)infoVect[i].imgPath.toLocal8Bit())+std::string("/")+std::string(infoVect[i].cameraName.toLocal8Bit())+std::string("/")+std::string(infoVect[i].arithmeticName.toLocal8Bit());
        rtspUrls[i] = std::string("rtsp://") + std::string((const char *)infoVect[i].cameraUser.toLocal8Bit())
                + std::string(":") + std::string((const char *)infoVect[i].cameraPassword.toLocal8Bit())
                + std::string("@") + std::string((const char *)infoVect[i].originalPath.toLocal8Bit());
        rtsCodes[i] = infoVect[i].encodingType.toInt();
        for(int j = 0; j < arithmetics.size(); ++j){
            if(0 == arithmetics[j].compare((const char*)infoVect[i].arithmeticName.toLocal8Bit())) {
                arithmeticFlag[i] = j;
                break;
            }
        }
    }
    return 0;
}

/*
	rtsp 连接回调
*/
static void cb_new_rtspsrc_pad(GstElement *element, GstPad*pad, gpointer data)
{
    GstPad *sink_pad = gst_element_get_static_pad(GST_ELEMENT(data), "sink");
    if(gst_pad_is_linked(sink_pad) != TRUE)
    {

        gchar *name;
        name = gst_pad_get_name(pad);
        g_print("A new pad %s was created\n", name);
        if (gst_element_link_pads(element, name, GST_ELEMENT(data), "sink") != TRUE)
        {
            printf("Failed to link elements 3\n");
        } else {
            printf("Success to link elements 3\n");
        }
        g_free(name);
    } else {
        printf("have linked\n");
    }
    gst_object_unref(sink_pad);
}

/*
  	appsink回调，获取当前sample，保存到当前imgbuffer中
*/
static GstFlowReturn on_new_sample_from_sink(GstElement *sink_ele, gpointer data)
{
    long index = (long)data;
    GstSample *sample = NULL;
    GstBuffer *buffer;
    GstFlowReturn ret;
    GstMapInfo map;

    g_signal_emit_by_name(sink_ele, "pull-sample", &sample, &ret);
    int width = 0, height = 0;
    if(sample)
    {
        GstCaps *caps;
        GstStructure *s;
        caps = gst_sample_get_caps(sample);
        g_assert(caps);
        s = gst_caps_get_structure(caps, 0);
        gst_structure_get_int(s, "width", &width);
        gst_structure_get_int(s, "height", &height);
        buffer = gst_sample_get_buffer(sample);
        //g_print("on_new_sample_from_sink call!, width = %d, width = %d, size=%ld, timestamp=%ld\n", width, height, gst_buffer_get_size(buffer), GST_BUFFER_TIMESTAMP(buffer));
    }
    else
    {
        g_print("sample is NULL\n");
        return ret;
    }

    if(gst_buffer_map(buffer, &map, GST_MAP_READ))
    {
        pthread_mutex_lock(&bufMutexs[index]);
        //pthread_cond_wait(&bufCond, &bufMutex);
        if(imgBuffers[index] != NULL)
        {
            gst_buffer_unref(imgBuffers[index]);
            imgBuffers[index] = NULL;
        }
        //printf("--------------gst_buffer_copy_region----------------3\n");
        imgBuffers[index] = gst_buffer_copy_region(buffer, (GstBufferCopyFlags)(GST_BUFFER_COPY_ALL | GST_BUFFER_COPY_DEEP), 0, gst_buffer_get_size(buffer));
        //printf("--------------gst_buffer_copy_region----------------4\n");
        g_assert(imgBuffers[index]);
        pthread_mutex_unlock(&bufMutexs[index]);
        

    }
    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);
    return ret;
}

/*
	opencv跟帧算法线程
*/
void* trackFunc(void* param)
{
    long index = (long)param;
    printf("~~~~~~~~trackFunc index = %ld~~~~~~~~~\n",index);
    while(true) {
        cv::Size size(768, 432);
        if(imgBuffers[index] != NULL) {
            pthread_mutex_lock(&bufMutexs[index]);
            GstBuffer *saveBuf = gst_buffer_copy_region(imgBuffers[index], (GstBufferCopyFlags)(GST_BUFFER_COPY_ALL | GST_BUFFER_COPY_DEEP), 0, gst_buffer_get_size(imgBuffers[index]));
            pthread_mutex_unlock(&bufMutexs[index]);
            GstMapInfo saveInfo;
            gst_buffer_map(saveBuf, &saveInfo, GST_MAP_READ);
            char imgpath[256] = {0}; // save for choose area
            sprintf(imgpath, "%ld.jpg", index+1);
            cv::Mat image2save(size, CV_8UC4, (void *) saveInfo.data);
            std::vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
            compression_params.push_back(100);
            pthread_mutex_lock(&chooseImgMutexs[index]);
            imwrite(imgpath,image2save,compression_params);
            pthread_mutex_unlock(&chooseImgMutexs[index]);
            gst_buffer_unmap(saveBuf, &saveInfo);
            gst_buffer_unref(saveBuf);
        }

        pthread_mutex_lock(&predictMutexs[index]);
        std::vector<cv::Rect2d> tempRects;
        std::vector<int> tempCls;
        tempRects.assign(predictRectss[index].begin(), predictRectss[index].end());
        tempCls.assign(predictClss[index].begin(), predictClss[index].end());
        predictRectss[index].clear();
        predictClss[index].clear();
        pthread_mutex_unlock(&predictMutexs[index]);
        //printf("---------------predictRectss[%d]-----------size=%d-------\n ", index, tempRects.size());
        if(tempCls.size() > 0) {
            if(predictBuffers[index] == NULL) {
                continue;
            }

            cv::MultiTracker trackers;
            //printf("--------------gst_buffer_copy_region----------------5\n");
            pthread_mutex_lock(&predictBufMutexs[index]);
            GstBuffer *tempBuf = gst_buffer_copy_region(predictBuffers[index], (GstBufferCopyFlags)(GST_BUFFER_COPY_ALL | GST_BUFFER_COPY_DEEP), 0, gst_buffer_get_size(predictBuffers[index]));
            pthread_mutex_unlock(&predictBufMutexs[index]);
            //printf("--------------gst_buffer_copy_region----------------6\n");
            GstMapInfo info;
            gst_buffer_map(tempBuf, &info, GST_MAP_READ);
            //cv::Size size(800, 600);
            cv::Mat image(size, CV_8UC4, (void *) info.data);
            
            if(tempRects.size() > 0) {
                for (int i = 0; i < tempRects.size(); ++i) {
                    trackers.add(cv::TrackerMedianFlow::create(), image, tempRects[i]);
                }
            }
            int tracktimes = 30;
            while((--tracktimes > 0) &&(predictClss[index].size() == 0)) {
                g_usleep(40000);

                //auto t_start = std::chrono::high_resolution_clock::now();
                GstMapInfo info2;
                pthread_mutex_lock(&bufMutexs[index]);
                //printf("--------------gst_buffer_copy_region----------------7\n");
                GstBuffer *tempBuf2 = gst_buffer_copy_region(imgBuffers[index], (GstBufferCopyFlags)(GST_BUFFER_COPY_ALL | GST_BUFFER_COPY_DEEP), 0, gst_buffer_get_size(imgBuffers[index]));
                //printf("--------------gst_buffer_copy_region----------------8\n");
                pthread_mutex_unlock(&bufMutexs[index]);
                gst_buffer_map(tempBuf2, &info2, GST_MAP_READ);
                cv::Mat image2(size, CV_8UC4, (void *) info2.data);
                trackers.update(image2);
                gst_buffer_unmap(tempBuf2, &info2);
                gst_buffer_unref(tempBuf2);
                pthread_mutex_lock(&cvMutexs[index]);
                cvRectss[index].clear();
                cvClss[index].clear();
                cvClss[index].assign(tempCls.begin(), tempCls.end());
                //printf("--------trackers.rlt-----------trackers.getObjects().size=%d\n", trackers.getObjects().size());
                //printf("--------trackers.rlt-----------cvCls=%d\n", cvClss[index].size());
                for(int i=0; i<trackers.getObjects().size(); i++) {
                    //printf("object-%d: %f %f %f %f \n", i, trackers.getObjects()[i].x,trackers.getObjects()[i].y,trackers.getObjects()[i].width,trackers.getObjects()[i].height);
                    cvRectss[index].push_back(trackers.getObjects()[i]);
                }
                pthread_mutex_unlock(&cvMutexs[index]);
                //auto t_end = std::chrono::high_resolution_clock::now();
                //float total = std::chrono::duration<float, std::milli>(t_end - t_start).count();
                //gLogInfo << "Time taken for TrackerMedianFlow is " << total << " ms." << std::endl;

            }
            gst_buffer_unmap(tempBuf, &info);
            gst_buffer_unref(tempBuf);
        } else {
            pthread_mutex_lock(&cvMutexs[index]);
            //printf("---------------cvMutexs[%d]-----------sleep500000-------\n ", index);
            cvRectss[index].clear();
            cvClss[index].clear();
            pthread_mutex_unlock(&cvMutexs[index]);
            g_usleep(500000);
        }
    }
}

std::time_t getTimeStamp()
{
    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    std::time_t timestamp =  tp.time_since_epoch().count();
    return timestamp;
}


//
// 去重算法，opencv 实现 SSIM 算法
//
double getMSSIM(cv::Mat  inputimage1, cv::Mat inputimage2)
{
    cv::Mat i1 = inputimage1;
    cv::Mat i2 = inputimage2;
    const double C1 = 6.5025, C2 = 58.5225;
    int d = CV_32F;
    cv::Mat I1, I2;
    i1.convertTo(I1, d);
    i2.convertTo(I2, d);
    cv::Mat I2_2 = I2.mul(I2);
    cv::Mat I1_2 = I1.mul(I1);
    cv::Mat I1_I2 = I1.mul(I2);
    cv::Mat mu1, mu2;
    cv::GaussianBlur(I1, mu1, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(I2, mu2, cv::Size(11, 11), 1.5);
    cv::Mat mu1_2 = mu1.mul(mu1);
    cv::Mat mu2_2 = mu2.mul(mu2);
    cv::Mat mu1_mu2 = mu1.mul(mu2);
    cv::Mat sigma1_2, sigma2_2, sigma12;
    cv::GaussianBlur(I1_2, sigma1_2, cv::Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;
    cv::GaussianBlur(I2_2, sigma2_2, cv::Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;
    cv::GaussianBlur(I1_I2, sigma12, cv::Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;
    cv::Mat t1, t2, t3;
    t1 = 2 * mu1_mu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);
    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);
    cv::Mat ssim_map;
    cv::divide(t3, t1, ssim_map);
    cv::Scalar mssim = cv::mean(ssim_map);
    return ((mssim.val[0]+mssim.val[1]+mssim.val[2])/3);
}


/*
	重点区域防护算法线程
*/
void *protectFunc(void*)
{
    printf("its protect Func thread start!\n");
    IRuntime* runtime = createInferRuntime(gLogger.getTRTLogger());
    assert(runtime != nullptr);
    FILE *fp = fopen(protectAreaEnginePath, "rb");
    assert(fp != nullptr);
    fseek(fp, 0L, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    char* data = new char[length];
    printf("INPUT_H= %d\n", INPUT_H);
    printf("data length is = %ld\n", length);
    fread(data, length, 1, fp);
    fclose(fp);
    printf("do this\n");
    ICudaEngine* engine = runtime->deserializeCudaEngine(data, length, nullptr);
    assert(engine != NULL);
    IExecutionContext* context = engine->createExecutionContext();
    assert(context != nullptr);

    //gUI->listWidget->setIconSize(QSize(280,200));

    GstFlowReturn ret;
    static int count=0;
    bool init = false;
    int index = 0;
    while(true) {
        g_usleep(500000/4);
             if(protectarea_flag >0){
            protectarea_cnt ++;
            if(protectarea_cnt > 40){
                        libvlc_media_player_stop(protectarea_mp); 
                        protectarea_flag =0;
                        protectarea_cnt =0;
            }
        }
        index = (streamCount + index) % streamCount;
        if(arithmetics[arithmeticFlag[index]].compare("重点区域防护") != 0){
            index++;
            continue;
        }
        std::vector<std::pair<int, int>> area;

        pthread_mutex_lock(&ptsMutexs[index]);
        area.assign(pts[index].begin(), pts[index].end());
        pthread_mutex_unlock(&ptsMutexs[index]);
        if(area.size() == 0) {
            index++;
            //printf("------------have no area to protect ^o^--------\n");
            continue;
        }
        if(imgBuffers[index] != NULL)
        {
            //printf("--------------gst_buffer_copy_region----index=%d------------1\n", index);
            pthread_mutex_lock(&predictBufMutexs[index]);
            if(predictBuffers[index] != NULL) {
                gst_buffer_unref(predictBuffers[index]);
            }
            pthread_mutex_lock(&bufMutexs[index]);
            predictBuffers[index] = gst_buffer_copy_region(imgBuffers[index], (GstBufferCopyFlags)(GST_BUFFER_COPY_ALL | GST_BUFFER_COPY_DEEP), 0, gst_buffer_get_size(imgBuffers[index]));

            pthread_mutex_unlock(&bufMutexs[index]);
            pthread_mutex_unlock(&predictBufMutexs[index]);
            //printf("--------------gst_buffer_copy_region----------------2\n");
            GstMapInfo info;
            if(gst_buffer_map(predictBuffers[index], &info, GST_MAP_READ))
            {
                //open cv
                cv::Size size_old(768, 432);
                cv::Mat img(size_old, CV_8UC4, (void *) info.data);
                cv::Mat img2;
                cv::cvtColor(img, img2, cv::COLOR_BGR2RGB);
                int ih, iw;
                ih = protectAreaImgSize;
                iw = protectAreaImgSize;
                float h,w;
                h = img2.rows;
                w = img2.cols;
                float scale = min(iw/w, ih/h);
                int nw,nh;
                nw = (int)(scale * w);
                nh = (int)(scale * h);
                cv::Size size(nw, nh);
                cv::Mat imgResized;
                cv::resize(img2, imgResized, size);
                vector<float> data_img(ih * iw * 3);
                for(int i=0; i < ih*iw*3; ++i)
                {
                    data_img[i] = 128.0;
                }
                int dw, dh;
                dw = (iw - nw) / 2;
                dh = (ih-nh) / 2;
                //printf("scale is %f, image width = %f, height=%f, nw = %d, nh = %d\n", scale, w, h, nw, nh);
                for(int i = dh; i < nh+dh; ++i)
                {
                    for(int j = dw; j < nw+dw; ++j)
                    {
                        data_img[i * iw*3 + j*3] = imgResized.data[(i-dh)*nw*3 + (j-dw)*3];
                        data_img[i * iw*3 + j*3 + 1] = imgResized.data[(i-dh)*nw*3 + (j-dw)*3 + 1];
                        data_img[i * iw*3 + j*3 + 2] = imgResized.data[(i-dh)*nw*3 + (j-dw)*3 + 2];
                    }
                }
                for(int i=0; i < ih*iw*3; ++i)
                {
                    data_img[i] = data_img[i]/255;
                }
                const int N = 1;
                //printf("before do this\n");
                ShapeArray<float> pred_bboxes_pre[3];
                ShapeArray<float> pred_bboxes;
                doInference(*context, &data_img[0], N, pred_bboxes_pre, protectAreaAnchor, protectAreaStride, protectAreaShapes, protectAreaImgSize, protectAreaImgSize, protectAreaClassNum);

                pred_bboxes.setData(pred_bboxes_pre[0].getData(), pred_bboxes_pre[0].getSize());
                pred_bboxes.reshape({-1, 5+protectAreaClassNum});
                std::cout <<"****** pred_bboxes.size = "<< pred_bboxes.getSize()<<std::endl;
                for(int i=1; i<3; ++i)
                {
                    pred_bboxes_pre[i].reshape({-1, 5+protectAreaClassNum});
                    pred_bboxes.concatenate(pred_bboxes_pre[i], 0);
                }
                for(int i=0; i<3; ++i)
                {
                    pred_bboxes_pre[i].clear();
                }
                ShapeArray<float> out_boxes;
                 //std::cout <<"****** pred_bboxes.size = "<< pred_bboxes.getSize()<<std::endl;
                postprocess_boxes(pred_bboxes, w, h, protectAreaImgSize, 0.5, out_boxes, protectAreaClassNum);
                pthread_mutex_lock(&predictMutexs[index]);
                predictRectss[index].clear();
                predictClss[index].clear();
                std::cout <<"out_boxes.size = "<< out_boxes .getSize()<<std::endl;
                bool needSave = false;
                for(int i = 0; i < out_boxes.getSize()/6; ++i) {
                    int cls = (int)(out_boxes.getData()[5 + i*6]);
                    int x = (int)out_boxes.getData()[0 + i*6];
                    int y = (int)out_boxes.getData()[1 + i*6];
                    int width = (int)(out_boxes.getData()[2 + i*6] - out_boxes.getData()[0 + i*6]);
                    int height = (int)(out_boxes.getData()[3 + i*6] - out_boxes.getData()[1 + i*6]);
                    std::vector<cv::Point> p1_pgn;
                    p1_pgn.push_back({x,y+height*0.6});
                    p1_pgn.push_back({x+width,y+height*0.6});
                    p1_pgn.push_back({x+width,y + height});
                    p1_pgn.push_back({x,y+height});
                    bool bsave = false;
                    std::vector<cv::Point> p2_pgn;
                    double p1_area = cv::contourArea(p1_pgn);
                    for(int j =0; j < area.size(); ++j) {
                        if(area[j].first != -1) {
                            p2_pgn.push_back({area[j].first, area[j].second});
                            if(j == (area.size() - 1)) {
                                std::vector<cv::Point> interPoly;
                                getintersection(p2_pgn, p1_pgn, interPoly);
                                if(interPoly.size() > 0) {
                                    double interarea = cv::contourArea(interPoly);
                                    double iou = interarea/p1_area;
                                    if(iou > 0.15) {
                                        bsave = true;
                                        break;
                                    }
                                }
                            }
                        } else {
                            if(j != 0) {
                                std::vector<cv::Point> interPoly;
                                getintersection(p2_pgn, p1_pgn, interPoly);
                                if(interPoly.size() > 0) {
                                    double interarea = cv::contourArea(interPoly);
                                    double iou = interarea/p1_area;
                                    if(iou > 0.15) {
                                        bsave = true;
                                        break;
                                    }
                                }
                            }
                            p2_pgn.clear();
                        }
                    }
                    if(bsave) {
                        predictRectss[index].push_back({x,y, width , height });
                        predictClss[index].push_back(cls);
                    }
                }
                if(!(img_protectArea_pre[index].empty()))
                    protectArea_mean =getMSSIM(img2,img_protectArea_pre[index]);
                img_protectArea_pre[index]=img2.clone();
                std::cout<< "proetect_mean =" << protectArea_mean<<"q\n"<<std::endl;
                    //if(predictClss[index].size() > 0&&(predictClss[index].size()!=predictClss_pre[index].size())) {
                    if (predictClss[index].size() > 0 && (((protectArea_mean < 0.96) && (predictClss[index].size() != predictClss_pre[index].size())) || (protectArea_mean < 0.6)))
                {
                    //save imageß
                    for(int i=0; i < predictClss[index].size(); ++i) {
                        if(predictClss[index][i] == 0) {
                            needSave = true;
                            cv::rectangle(img, predictRectss[index][i], cv::Scalar(0, 0, 255), 1, 8);
                            //protectAreaImag_count ++;
                         // gUI->lcdNumber_2->display(protectAreaImag_count);
                        }
                    }
                  //  if(predictClss[index].size()==predictClss_pre[index].size())
                    //    needSave = false;
                    //predictClss_pre[index]=predictClss[index];
                    if(needSave){
                        bool start = false;
                        std::pair<int,int> sp, curp;
                        for(int i =0; i < pts[index].size(); ++i) {
                            if(pts[index][i].first != -1) {
                                if(!start) {
                                    start = true;
                                    sp = pts[index][i];
                                    curp = sp;
                                } else if(i == (pts[index].size() - 1)){
                                    start = false;
                                    cv::line(img, cv::Point(curp.first, curp.second), cv::Point(pts[index][i].first, pts[index][i].second), cv::Scalar(0, 255, 0), 1, 8);
                                    curp = pts[index][i];
                                    cv::line(img, cv::Point(curp.first, curp.second), cv::Point(sp.first, sp.second), cv::Scalar(0, 255, 0), 1, 8);
                                } else {
                                    cv::line(img, cv::Point(curp.first, curp.second), cv::Point(pts[index][i].first, pts[index][i].second), cv::Scalar(0, 255, 0), 1, 8);
                                    curp = pts[index][i];
                                }
                            } else {
                                if(start) {
                                    start = false;
                                    cv::line(img, cv::Point(curp.first, curp.second), cv::Point(sp.first, sp.second), cv::Scalar(0, 255, 0), 1, 8);
                                } else {}
                            }
                        }
                        time_t t = time(0);
                        char day[64] = {0};
                        strftime(day, sizeof(day), "%Y-%m-%d", localtime(&t));
                        time_t timestamp =  getTimeStamp();
                        char savepath[256] = {0};
                        sprintf(savepath, "%s/%s/", imgPaths[index].c_str(), day);
                        if(pathCheck(savepath)) {
                            sprintf(savepath, "%s/%s/%ld.jpg", imgPaths[index].c_str(), day, timestamp);
                            std::vector<int> compression_params;
                            compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
                            compression_params.push_back(100);
                            imwrite(savepath,img,compression_params);
                            pthread_mutex_lock(&uiMutex);
                            if (gUI->listWidget->count() == listViewNums) {
                                QListWidgetItem *item = gUI->listWidget->takeItem(0);
                                delete item;
                            }
                            
                            gUI->listWidget->addItem(new QListWidgetItem(QIcon(savepath),cameras[index]));
                            if (true)
                            {
                                QSqlQuery query;
                                query.prepare("insert into file_info (id, fileName, path, createTime, cameraId, arithmeticId, catchType, catchNumber) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
                                char tmp[256] = {0};
                                sprintf(tmp, "%ld", timestamp);
                                query.addBindValue(tmp);//id
                                memset(tmp, 0, 256);
                                sprintf(tmp, "%ld.jpg", timestamp);//fileName
                                query.addBindValue(tmp);
                                query.addBindValue(savepath);
                                query.addBindValue(day);
                                query.addBindValue(cameraIds[index]);
                                query.addBindValue(camera_arithmeticId[index]);
                                query.addBindValue("protect Area");
                                int nums = predictClss[index].size();
                                protectAreaImag_count = protectAreaImag_count+nums;
                                gUI->lcdNumber_2->display(protectAreaImag_count);
                                query.addBindValue(nums);
                                //system("play  危险区域，请勿靠近.mp3");
                                if(protectarea_flag==0) {
                                    libvlc_media_player_play(protectarea_mp);
                                    protectarea_flag=1;
                                }
                                 
                                query.exec();
                            }else {
                                g_print("can not open db file");
                            }
                            pthread_mutex_unlock(&uiMutex);
                        }
                    }
                }
                predictClss_pre[index]=predictClss[index];
                pthread_mutex_lock(&cvMutexs[index]);
                cvRectss[index].assign(predictRectss[index].begin(), predictRectss[index].end());
                cvClss[index].assign(predictClss[index].begin(), predictClss[index].end());
                pthread_mutex_unlock(&cvMutexs[index]);

                pthread_mutex_unlock(&predictMutexs[index]);
            }
            gst_buffer_unmap(predictBuffers[index], &info);
            index++;
        } else {
            index++;
        }
    }
}

/*
安全帽检测算法线程
*/
void *predictFunc(void*)
{
    printf("its predict Img thread start!\n");
    IRuntime* runtime = createInferRuntime(gLogger.getTRTLogger());
    assert(runtime != nullptr);
    FILE *fp = fopen(safetyHatEnginePath, "rb");
    assert(fp != nullptr);
    fseek(fp, 0L, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    char* data = new char[length];
    printf("data length is = %ld\n", length);
    fread(data, length, 1, fp);
    fclose(fp);
    printf("do this\n");
    ICudaEngine* engine = runtime->deserializeCudaEngine(data, length, nullptr);
    assert(engine != NULL);
    IExecutionContext* context = engine->createExecutionContext();
    assert(context != nullptr);

    //gUI->listWidget->setIconSize(QSize(280,200));

    GstFlowReturn ret;
    static int count=0;
    bool init = false;
    int index = 0;
    while(true)
    {
        g_usleep(500000/4);
        if(safehat_flag >0){
            safehat_cnt ++;
            if(safehat_cnt > 40){
                 libvlc_media_player_stop(safehat_mp);
                safehat_flag =0;
                 safehat_cnt =0;
            }
        }
        index = (streamCount + index) % streamCount;
        if(arithmetics[arithmeticFlag[index]].compare("安全帽检测") != 0){
            index++;
            continue;
        }
        if(imgBuffers[index] != NULL)
        {
            //printf("--------------gst_buffer_copy_region----index=%d------------1\n", index);
            pthread_mutex_lock(&predictBufMutexs[index]);
            if(predictBuffers[index] != NULL) {
                gst_buffer_unref(predictBuffers[index]);
            }
            pthread_mutex_lock(&bufMutexs[index]);
            predictBuffers[index] = gst_buffer_copy_region(imgBuffers[index], (GstBufferCopyFlags)(GST_BUFFER_COPY_ALL | GST_BUFFER_COPY_DEEP), 0, gst_buffer_get_size(imgBuffers[index]));

            pthread_mutex_unlock(&bufMutexs[index]);
            pthread_mutex_unlock(&predictBufMutexs[index]);
            //printf("--------------gst_buffer_copy_region----------------2\n");
            GstMapInfo info;
            if(gst_buffer_map(predictBuffers[index], &info, GST_MAP_READ))
            {
                //open cv
                cv::Size size_old(768, 432);
                cv::Mat img(size_old, CV_8UC4, (void *) info.data);
                cv::Mat img2;
                cv::cvtColor(img, img2, cv::COLOR_BGR2RGB);
                int ih, iw;
                ih = safetyHatImgSize;
                iw = safetyHatImgSize;
                float h,w;
                h = img2.rows;
                w = img2.cols;
                float scale = min(iw/w, ih/h);
                int nw,nh;
                nw = (int)(scale * w);
                nh = (int)(scale * h);
                cv::Size size(nw, nh);
                cv::Mat imgResized;
                cv::resize(img2, imgResized, size);  
                imgResized.convertTo(imgResized,CV_32F);
                //  opencv    图形增强
                cv::Mat  resultimag = cv::Mat::zeros(imgResized.size(),imgResized.type());

                //laplas 
              //  cv::Mat kernel = (cv::Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);
             //   cv::filter2D(imgResized, resultimag, CV_8UC3, kernel);

                //  opencv  log  s=c*log(1+v*r)    r =(0~1),    v =1;    c =1;
                imgResized = imgResized /255;
                for(int i=0;i<imgResized.rows;i++)
                {
                    for(int j=0;j<imgResized.cols;j++)
                    {
                        resultimag.at<Vec3f>(i,j)[0] = cv::log(1+imgResized.at<Vec3f>(i,j)[0] );
                        resultimag.at<Vec3f>(i,j)[1] = cv::log(1+imgResized.at<Vec3f>(i,j)[1] );
                        resultimag.at<Vec3f>(i,j)[2] = cv::log(1+imgResized.at<Vec3f>(i,j)[2] );
                    }
                }
                cv::normalize(resultimag,resultimag,0,255,CV_MINMAX);
                cv::convertScaleAbs(resultimag,resultimag);
              // cv::imshow("resultimag ",resultimag);
             //  cv::waitKey(0);
                vector<float> data_img(ih * iw * 3);
                for(int i=0; i < ih*iw*3; ++i)
                {
                    data_img[i] = 128.0;
                }
                int dw, dh;
                dw = (iw - nw) / 2;
                dh = (ih-nh) / 2;
                //printf("scale is %f, image width = %f, height=%f, nw = %d, nh = %d\n", scale, w, h, nw, nh);
                for(int i = dh; i < nh+dh; ++i)
                {
                    for(int j = dw; j < nw+dw; ++j)
                    {
                        data_img[i * iw*3 + j*3] = resultimag.data[(i-dh)*nw*3 + (j-dw)*3];
                        data_img[i * iw*3 + j*3 + 1] = resultimag.data[(i-dh)*nw*3 + (j-dw)*3 + 1];
                        data_img[i * iw*3 + j*3 + 2] = resultimag.data[(i-dh)*nw*3 + (j-dw)*3 + 2];
                       /* data_img[i * iw*3 + j*3] = imgResized.data[(i-dh)*nw*3 + (j-dw)*3];
                        data_img[i * iw*3 + j*3 + 1] = imgResized.data[(i-dh)*nw*3 + (j-dw)*3 + 1];
                        data_img[i * iw*3 + j*3 + 2] = imgResized.data[(i-dh)*nw*3 + (j-dw)*3 + 2];*/
                    }
                }
                
                for(int i=0; i < ih*iw*3; ++i)
                {
                    data_img[i] = data_img[i]/255;
                }
                const int N = 1;
                //printf("before do this\n");
                ShapeArray<float> pred_bboxes_pre[2];
                ShapeArray<float> pred_bboxes;
                doInference_2(*context, &data_img[0], N, pred_bboxes_pre, safetyHatAnchor, safetyHatStride, safetyHatShapes, safetyHatImgSize, safetyHatImgSize, safetyHatClassNum);
                pred_bboxes.setData(pred_bboxes_pre[0].getData(), pred_bboxes_pre[0].getSize());
                pred_bboxes.reshape({-1, 5+safetyHatClassNum});
                for(int i=1; i<2; ++i)
                {
                    pred_bboxes_pre[i].reshape({-1, 5+safetyHatClassNum});
                    pred_bboxes.concatenate(pred_bboxes_pre[i], 0);
                }
                for(int i=0; i<2; ++i)
                {
                    pred_bboxes_pre[i].clear();
                }
                ShapeArray<float> out_boxes;
                postprocess_boxes(pred_bboxes, w, h, safetyHatImgSize, 0.5, out_boxes, safetyHatClassNum);

                pthread_mutex_lock(&predictMutexs[index]);
                predictRectss[index].clear();
                predictClss[index].clear();
                bool needSave = false;
                for(int i = 0; i < out_boxes.getSize()/6; ++i) {
                    int cls = (int)(out_boxes.getData()[5 + i*6]);
                    int x = (int)out_boxes.getData()[0 + i*6];
                    int y = (int)out_boxes.getData()[1 + i*6];
                    int width = (int)(out_boxes.getData()[2 + i*6] - out_boxes.getData()[0 + i*6]);
                    int height = (int)(out_boxes.getData()[3 + i*6] - out_boxes.getData()[1 + i*6]);
                    predictRectss[index].push_back({x,y, width , height });
                    predictClss[index].push_back(cls);
                }
                if(!(img_safehat_pre[index].empty()))
                    safehat_mean =getMSSIM(img2,img_safehat_pre[index]);  
                img_safehat_pre[index]=img2.clone();    

                //if(predictClss[index].size() > 0&&(predictClss[index].size()!=predictClss_pre[index].size())) {
                if (predictClss[index].size() > 0 && (((safehat_mean < 0.96) && (predictClss[index].size() != predictClss_pre[index].size()))||(safehat_mean < 0.6)))
                {
                    //save image
                    int nums = 0;
                    for(int i=0; i < predictClss[index].
                    size(); ++i) {
                        if(predictClss[index][i] == 0) {
                            // 未带安全帽
                            needSave = true;
                            cv::rectangle(img, predictRectss[index][i], cv::Scalar(0, 0, 255), 1, 8);
                            nums ++;
                            

                        }
                    }
                    //safetyHatImag_count = safetyHatImag_count+nums;
                    //gUI->lcdNumber->display(safetyHatImag_count);
                   // if(predictClss[index].size()==predictClss_pre[index].size())
                   //     needSave = false;
                    //predictClss_pre[index]=predictClss[index];
                    if(needSave){
                        time_t t = time(0);
                        char day[64] = {0};
                        strftime(day, sizeof(day), "%Y-%m-%d", localtime(&t));
                        time_t timestamp =  getTimeStamp();
                        char savepath[256] = {0};
                        sprintf(savepath, "%s/%s/", imgPaths[index].c_str(), day);
                        if(pathCheck(savepath)) {
                            sprintf(savepath, "%s/%s/%ld.jpg", imgPaths[index].c_str(), day, timestamp);
                            std::vector<int> compression_params;
                            compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
                            compression_params.push_back(100);
                            imwrite(savepath,img,compression_params);
                            pthread_mutex_lock(&uiMutex);
                            if (gUI->listWidget->count() == listViewNums) {
                                QListWidgetItem *item = gUI->listWidget->takeItem(0);
                                delete item;
                            }
                            //gUI->listWidget->setIconSize(QSize(280,240));
                            gUI->listWidget->addItem(new QListWidgetItem(QIcon(savepath),cameras[index]));
                            if (true)
                            {
                                QSqlQuery query;
                                query.prepare("insert into file_info (id, fileName, path, createTime, cameraId, arithmeticId, catchType, catchNumber) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
                                char tmp[256] = {0};
                                sprintf(tmp, "%ld", timestamp);
                                query.addBindValue(tmp);//id
                                memset(tmp, 0, 256);
                                sprintf(tmp, "%ld.jpg", timestamp);//fileName
                                query.addBindValue(tmp);
                                query.addBindValue(savepath);
                                query.addBindValue(day);
                                query.addBindValue(cameraIds[index]);
                                query.addBindValue(camera_arithmeticId[index]);
                                query.addBindValue("Satety Hat");
                                query.addBindValue(nums);
                                query.exec();
                                safetyHatImag_count =safetyHatImag_count+ nums ;
                                gUI->lcdNumber->display(safetyHatImag_count);
                                //system("play  请正确佩戴安全帽.mp3");
                                if(safehat_flag ==0){
                                       libvlc_media_player_play(safehat_mp); 
                                       safehat_flag =1;
                                }
                                 
                                //g_print("insert sql over \n");    
                            }else {
                                g_print("can not open db file");
                            }
                            pthread_mutex_unlock(&uiMutex);
                        }
                    }
                }
                predictClss_pre[index]=predictClss[index];
                pthread_mutex_lock(&cvMutexs[index]);
                cvRectss[index].assign(predictRectss[index].begin(), predictRectss[index].end());
                cvClss[index].assign(predictClss[index].begin(), predictClss[index].end());
                pthread_mutex_unlock(&cvMutexs[index]);

                pthread_mutex_unlock(&predictMutexs[index]);
            }
            gst_buffer_unmap(predictBuffers[index], &info);
            index++;
        } else {
            index++;
        }

    }
}

typedef struct
{
    gboolean valid;
    GstVideoInfo vinfo;
} CairoOverlayState;


static void
prepare_overlay (GstElement * overlay, GstCaps * caps, gpointer user_data)
{
    CairoOverlayState *state = (CairoOverlayState *) user_data;

    state->valid = !state->valid;
}

/*
  绘制标记框
*/
static void
draw_overlay (GstElement * overlay, cairo_t * cr, guint64 timestamp,
              guint64 duration, gpointer user_data)
{
    long index = (long)user_data;

    std::vector<cv::Rect2d> tempRects;
    std::vector<int> tempCls;
    pthread_mutex_lock(&cvMutexs[index]);
    tempRects.assign(cvRectss[index].begin(), cvRectss[index].end());
    tempCls.assign(cvClss[index].begin(), cvClss[index].end());
    pthread_mutex_unlock(&cvMutexs[index]);

    if(tempRects.size() != tempCls.size()) {
        //printf("-----cvRects.size() != cvCls.size()-----");
        return;
    }
    for(int i =0; i < tempCls.size(); ++i) {
        cairo_move_to(cr,tempRects[i].x,tempRects[i].y);
        cairo_line_to(cr,tempRects[i].x+tempRects[i].width,tempRects[i].y);
        cairo_line_to(cr,tempRects[i].x+tempRects[i].width,tempRects[i].y + tempRects[i].height);
        cairo_line_to(cr,tempRects[i].x,tempRects[i].y + tempRects[i].height);
        cairo_line_to(cr,tempRects[i].x,tempRects[i].y);
        if(tempCls[i] == 0) {
            cairo_set_source_rgb(cr,1,0,0);
        } else {
            cairo_set_source_rgb(cr,0,0,1);
        }
        cairo_stroke(cr);
    }

    bool start = false;
    std::pair<int,int> sp;
    for(int i =0; i < pts[index].size(); ++i) {
        //printf("pts[index][i].first = %d, pts[index][i].second = %d\n", pts[index][i].first, pts[index][i].second);
        if(pts[index][i].first != -1) {
            if(!start) {
                start = true;
                cairo_set_source_rgb(cr,0,1,0);
                sp = pts[index][i];
                cairo_move_to(cr, pts[index][i].first, pts[index][i].second);
            } else if(i == (pts[index].size() - 1)){
                start = false;
                cairo_line_to(cr,pts[index][i].first, pts[index][i].second);
                cairo_line_to(cr,sp.first, sp.second);
                cairo_stroke(cr);
            } else {
                cairo_line_to(cr,pts[index][i].first, pts[index][i].second);
            }
        } else {
            if(start) {
                start = false;
                cairo_line_to(cr,sp.first, sp.second);
                cairo_stroke(cr);
            } else {}
        }
    }
}

/*
	初始化pipeline，type：0=显示，else=推流
*/
int initPipeLine(Ui::MainWindow *ui, int index, int type = 0) {
    char name[128] = {0};
    GstElement
            *rtspsrc,
            *rtph264depay,
            *h264parse,
            *omxh264dec,
            *nvvidconv,
            *xvimagesink,
            *appsink,
            *tee,
            *showqueue,
            *savequeue;
    GstElement *cairo_overlay;
    CairoOverlayState *overlay_state;
    GstElement *omxh264enc, *flvmux, *rtmpsink;
    //GMainLoop *loop;
    //GstBus *bus;
    GstStateChangeReturn ret;

    GstPad *tee_show_pad, *tee_save_pad;
    GstPad *queue_show_pad, *queue_save_pad;
    overlay_state = g_new0 (CairoOverlayState, 1);
    memset(name, 128, 0);
    sprintf(name, "TEST%ld", getTimeStamp());
    pipelines[index] = gst_pipeline_new(name);
    g_assert(pipelines[index]);

    //bus = gst_pipeline_get_bus(GST_PIPELINE(pipelines[index]));
    //g_assert(bus);

    memset(name, 128, 0);
    sprintf(name, "source%ld", getTimeStamp());
    rtspsrc = gst_element_factory_make("rtspsrc", name);
    g_assert(rtspsrc);
    g_object_set(rtspsrc, "location", rtspUrls[index].c_str(), NULL);
    g_object_set(rtspsrc, "latency", 300, NULL);
    g_object_set(rtspsrc, "protocols", "tcp", NULL);

    memset(name, 128, 0);
    if(rtsCodes[index] == 264) {
        sprintf(name, "rtph264depay%ld", getTimeStamp());
        rtph264depay = gst_element_factory_make("rtph264depay", name);
    } else {
        sprintf(name, "rtph265depay%ld", getTimeStamp());
        rtph264depay = gst_element_factory_make("rtph265depay", name);
    }
    g_assert(rtph264depay);

    memset(name, 128, 0);
    if(rtsCodes[index] == 264) {
        sprintf(name, "h264parse%ld", getTimeStamp());
        h264parse = gst_element_factory_make("h264parse", name);
    } else {
        sprintf(name, "h264parse%ld", getTimeStamp());
        h264parse = gst_element_factory_make("h265parse", name);
    }
    g_assert(h264parse);

    memset(name, 128, 0);
    if(rtsCodes[index] == 264) {
        sprintf(name, "omxh264dec%ld", getTimeStamp());
        omxh264dec = gst_element_factory_make("omxh264dec", name);
    } else {
        sprintf(name, "omxh265dec%ld", getTimeStamp());
        omxh264dec = gst_element_factory_make("omxh265dec", name);
    }
    g_assert(omxh264dec);

    memset(name, 128, 0);
    sprintf(name, "nvvidconv%ld", getTimeStamp());
    nvvidconv = gst_element_factory_make("nvvidconv", name);
    g_assert(nvvidconv);



    memset(name, 128, 0);
    sprintf(name, "caps%ld", getTimeStamp());
    GstElement *capsfilter = gst_element_factory_make("capsfilter", name);
    g_object_set(capsfilter, "caps", gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGRx", "width", G_TYPE_INT, 768, "height", G_TYPE_INT, 432, NULL), NULL);

    memset(name, 128, 0);
    sprintf(name, "videoconvert%ld", getTimeStamp());
    GstElement *videoconvert = gst_element_factory_make("videoconvert", name);
    g_assert(videoconvert);

    memset(name, 128, 0);
    sprintf(name, "cairooverlay%ld", getTimeStamp());
    cairo_overlay = gst_element_factory_make ("cairooverlay", name);
    g_assert(cairo_overlay);
    g_signal_connect (cairo_overlay, "draw", G_CALLBACK (draw_overlay), (gpointer)index);
    g_signal_connect (cairo_overlay, "caps-changed", G_CALLBACK (prepare_overlay), overlay_state);

    memset(name, 128, 0);
    sprintf(name, "adaptor%ld", getTimeStamp());
    GstElement *adaptor = gst_element_factory_make ("videoconvert", name);

    memset(name, 128, 0);
    sprintf(name, "xvimagesink%ld", getTimeStamp());
    xvimagesink = gst_element_factory_make("xvimagesink", name);
    g_assert(xvimagesink);

    memset(name, 128, 0);
    sprintf(name, "omxh264enc%ld", getTimeStamp());
    omxh264enc = gst_element_factory_make("omxh264enc", name);
    g_assert(omxh264enc);

    memset(name, 128, 0);
    sprintf(name, "flvmux%ld", getTimeStamp());
    flvmux = gst_element_factory_make("flvmux", name);
    g_assert(flvmux);
    g_object_set(flvmux, "streamable", true, NULL);

    memset(name, 128, 0);
    sprintf(name, "rtmpsink%ld", getTimeStamp());
    rtmpsink = gst_element_factory_make("rtmpsink", name);
    g_assert(rtmpsink);
    char locationUrl[256]={0};
    sprintf(locationUrl, "rtmp://localhost:1935/mylive/test%d", index+1);
    printf("locationUrl=%s", locationUrl);
    g_object_set(rtmpsink, "location", locationUrl, NULL);

    memset(name, 128, 0);
    sprintf(name, "tee%ld", getTimeStamp());
    tee = gst_element_factory_make("tee", name);
    g_assert(tee);

    memset(name, 128, 0);
    sprintf(name, "show%ld", getTimeStamp());
    showqueue = gst_element_factory_make("queue", name);
    g_assert(showqueue);

    memset(name, 128, 0);
    sprintf(name, "save%ld", getTimeStamp());
    savequeue = gst_element_factory_make("queue", name);
    g_assert(savequeue);

    memset(name, 128, 0);
    sprintf(name, "appsink%ld", getTimeStamp());
    appsink = gst_element_factory_make("appsink", name);
    g_assert(appsink);
    g_object_set(appsink, "emit-signals", TRUE, "sync", FALSE, NULL);
    g_signal_connect(appsink, "new-sample", G_CALLBACK(on_new_sample_from_sink), (gpointer)index);

    gst_bin_add_many(GST_BIN(pipelines[index]), rtspsrc, rtph264depay, NULL);
    g_signal_connect(rtspsrc, "pad-added", G_CALLBACK(cb_new_rtspsrc_pad), rtph264depay);

    gst_bin_add_many(GST_BIN(pipelines[index]), h264parse, NULL);
    if (gst_element_link(rtph264depay, h264parse) != TRUE) {
        g_printerr ("Elements could not be linked-----1.\n");
        return -1;
    }
    if(type == 0) {
        gst_bin_add_many(GST_BIN(pipelines[index]), omxh264dec, nvvidconv, capsfilter, videoconvert, cairo_overlay, adaptor, tee, showqueue, savequeue, xvimagesink, appsink, NULL);
    } else {
        gst_bin_add_many(GST_BIN(pipelines[index]), omxh264dec, nvvidconv, capsfilter, videoconvert, cairo_overlay, adaptor, tee, showqueue, savequeue, omxh264enc, flvmux, rtmpsink, appsink, NULL);
    }
    if (gst_element_link_many(h264parse, omxh264dec, nvvidconv, capsfilter, tee, NULL) != TRUE) {
        g_printerr ("Elements could not be linked-----1.\n");
        return -1;
    }

    if(type == 0) {
        if (gst_element_link_many(showqueue, videoconvert, cairo_overlay, adaptor, xvimagesink,  NULL) != TRUE) {
            g_printerr ("Elements could not be linked-----2.\n");
            return -1;
        }
        if (index == 0) {
            gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (xvimagesink), ui->video->winId());
        } else if(index == 1) {
            gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (xvimagesink), ui->video_2->winId());
        } else if(index == 2) {
            gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (xvimagesink), ui->video_3->winId());
        } else if(index == 3) {
            gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (xvimagesink), ui->video_4->winId());
        }
    }else {
        if (gst_element_link_many(showqueue, videoconvert, cairo_overlay, adaptor, omxh264enc,  flvmux, rtmpsink, NULL) != TRUE) {
            g_printerr ("Elements could not be linked-----2.\n");
            return -1;
        }
    }

    if (gst_element_link_many(savequeue, appsink, NULL) != TRUE) {
        g_printerr ("Elements could not be linked-----5.\n");
        return -1;
    }

    tee_show_pad = gst_element_get_request_pad(tee, "src_%u");
    g_assert(tee_show_pad);
    g_print("show ========%s", gst_pad_get_name(tee_show_pad));
    queue_show_pad = gst_element_get_static_pad(showqueue, "sink");
    if (gst_pad_link(tee_show_pad, queue_show_pad) != GST_PAD_LINK_OK) {
        g_printerr ("Elements could not be linked-----3.\n");
        return -1;
    }

    tee_save_pad = gst_element_get_request_pad(tee, "src_%u");
    g_assert(tee_save_pad);
    g_print("show ========%s", gst_pad_get_name(tee_save_pad));
    queue_save_pad = gst_element_get_static_pad(savequeue, "sink");
    if (gst_pad_link(tee_save_pad, queue_save_pad) != GST_PAD_LINK_OK) {
        g_printerr ("Elements could not be linked-----3.\n");
        return -1;
    }
    ret = gst_element_set_state(pipelines[index], GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (pipelines[index]);
        return -1;
    } else if (ret == GST_STATE_CHANGE_NO_PREROLL) {
        g_printerr ("its playing\n");
    }
}

void* pipeLineFunc(void* param)
{
    long index = (long) param;
    while(true) {
        g_usleep(1000000);
        if(pipelines[index] != NULL) {
            GstState state, pending;
            gst_element_get_state(pipelines[index], &state, &pending, GST_CLOCK_TIME_NONE);
            printf("----pipeline[%d]------state=%d, pending=%d\n", index, state, pending);
            if(state != GST_STATE_PLAYING) {
                printf("----pipeline[%d]--is not playing,url is=%s\n", index, rtspUrls[index].c_str());
            }
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setIconSize(QSize(288,210));
    ui->listWidget->setSpacing(16);
    ui->listWidget->setResizeMode(QListWidget::Adjust);
    ui->listWidget->setFlow(QListView::TopToBottom);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    connect(ui->video,SIGNAL(mouseDoubleCllckEvent()),ui->video,SLOT(mousedoubleclicked));

    loadCfg();
    for (int i = 0; i < streamCount; ++i) {
        initPipeLine(ui, i);
    }
    ui->display->setStyleSheet("background:rgb(0,255,0)");
    ui->display_2->setStyleSheet("background:rgb(0,255,0)");
    ui->display_3->setStyleSheet("background:rgb(0,255,0)");
    ui->rtmp_4->setStyleSheet("background:rgb(0,255,0)");

    for (int i = 0; i < streamCount; ++i) {
        pthread_mutex_init(&cvMutexs[i], NULL);
        pthread_mutex_init(&predictMutexs[i], NULL);
        pthread_mutex_init(&bufMutexs[i], NULL);
        pthread_mutex_init(&predictBufMutexs[i], NULL);
        pthread_mutex_init(&chooseImgMutexs[i], NULL);
        pthread_mutex_init(&ptsMutexs[i], NULL);
    }
    pthread_mutex_init(&uiMutex, NULL);

    gUI = ui;
    pthread_t predictThreadId;
    pthread_create(&predictThreadId, NULL, (void* (*)(void*))predictFunc, NULL);

    pthread_t protectThreadId;
    pthread_create(&protectThreadId, NULL, (void* (*)(void*))protectFunc, NULL);
    
    pthread_t pipeStateCheckThreadId[streamCount];
    for (long i = 0; i < streamCount; ++i) {
        pthread_create(&pipeStateCheckThreadId[i], NULL, (void* (*)(void*))pipeLineFunc, (void*)i);
    }

    pthread_t trackThreadId[streamCount];
    for (long i = 0; i < streamCount; ++i) {
        pthread_create(&trackThreadId[i], NULL, (void* (*)(void*))trackFunc, (void*)i);
    }

}

MainWindow::~MainWindow()
{
    // stop playing
   libvlc_media_player_stop(safehat_mp);

    // free the media_player
    libvlc_media_player_release(safehat_mp);

   libvlc_media_player_stop(protectarea_mp);

    // free the media_player
    libvlc_media_player_release(protectarea_mp);

    libvlc_release(inst);


    delete ui;
}

void MainWindow::reLoadCfg() {

}

void MainWindow::loadCfg() {
#ifdef USE_WEBCFG
    loadWebCfg();
    pair<int,int> intp;
    FILE *fp;
    if(!rtspUrls[0].empty()) {
        ui->label->setText(QString("画面1:    ") + cameras[0]);
        ui->display->setDisabled(false);
        ui->rtmp->setDisabled(false);
        if(arithmetics[arithmeticFlag[0]].compare("重点区域防护") == 0) {
            ui->choose->setDisabled(false);
            fp=fopen("test1.txt","r");//你自己的文件路径
             if(fp == NULL)
                 fclose(fp);
             else{
             while(1){
                 fscanf(fp,"%d %d",&intp.first,&intp.second);
                 pts[0].push_back(intp);
             //    cout <<intp.first<<" "<<intp.second<<endl;
                 if (feof(fp))break;
             }
             fclose(fp);
             }
        } else {
            ui->choose->setDisabled(true);
        }
    } else {
        ui->label->setText(QString("画面1:    未配置"));
        ui->display->setDisabled(true);
        ui->rtmp->setDisabled(true);
        ui->choose->setDisabled(true);
    }

    if(!rtspUrls[1].empty()) {
        ui->label_2->setText(QString("画面2:    ") + cameras[1]);
        ui->display_2->setDisabled(false);
        ui->rtmp_2->setDisabled(false);
        if(arithmetics[arithmeticFlag[1]].compare("重点区域防护") == 0) {
            ui->choose_2->setDisabled(false);
            fp=fopen("test2.txt","r");//你自己的文件路径
            if(fp == NULL)
                fclose(fp);
            else{
            while(1){
                fscanf(fp,"%d %d",&intp.first,&intp.second);
                pts[1].push_back(intp);
             //   cout <<intp.first<<" "<<intp.second<<endl;
                if (feof(fp))break;
            }
            fclose(fp);
            }

        } else {
            ui->choose_2->setDisabled(true);
        }
    } else {
        ui->label_2->setText(QString("画面2:    未配置"));
        ui->display_2->setDisabled(true);
        ui->rtmp_2->setDisabled(true);
        ui->choose_2->setDisabled(true);
    }


    if(!rtspUrls[2].empty()) {
        ui->label_3->setText(QString("画面3:    ") + cameras[2]);
        ui->display_3->setDisabled(false);
        ui->rtmp_3->setDisabled(false);
        if(arithmetics[arithmeticFlag[2]].compare("重点区域防护") == 0) {
            ui->choose_3->setDisabled(false);
            fp=fopen("test3.txt","r");//你自己的文件路径
            if(fp == NULL)
                fclose(fp);
            else{
            while(1){
                fscanf(fp,"%d %d",&intp.first,&intp.second);
                pts[2].push_back(intp);
             //   cout <<intp.first<<" "<<intp.second<<endl;
                if (feof(fp))break;
            }
            fclose(fp);
            }

        } else {
            ui->choose_3->setDisabled(true);
        }
    } else {
        ui->label_3->setText(QString("画面3:    未配置"));
        ui->display_3->setDisabled(true);
        ui->rtmp_3->setDisabled(true);
        ui->choose_3->setDisabled(true);
    }


    if(!rtspUrls[3].empty()) {
        ui->label_4->setText(QString("画面4:    ") + cameras[3]);
        ui->display_4->setDisabled(false);
        ui->rtmp_4->setDisabled(false);
        if(arithmetics[arithmeticFlag[3]].compare("重点区域防护") == 0) {
            ui->choose_4->setDisabled(false);
            fp=fopen("test4.txt","r");//你自己的文件路径
            if(fp == NULL)
                fclose(fp);
            else{
            while(1){
                fscanf(fp,"%d %d",&intp.first,&intp.second);
                pts[3].push_back(intp);
            //    cout <<intp.first<<" "<<intp.second<<endl;
                if (feof(fp))break;
            }
            fclose(fp);
            }
        } else {
            ui->choose_4->setDisabled(true);
        }
    } else {
        ui->label_4->setText(QString("画面4:    未配置"));
        ui->display_4->setDisabled(true);
        ui->rtmp_4->setDisabled(true);
        ui->choose_4->setDisabled(true);
    }

    // load the vlc engine
    inst = libvlc_new(0, NULL);

    // create a new item
    safehat_m = libvlc_media_new_path(inst, "safehat.mp3");

    // create a media play playing environment
    safehat_mp = libvlc_media_player_new_from_media(safehat_m);

    // no need to keep the media now
    libvlc_media_release(safehat_m);

   // create a new item
    protectarea_m = libvlc_media_new_path(inst, "protectarea.mp3");

    // create a media play playing environment
    protectarea_mp = libvlc_media_player_new_from_media(protectarea_m);

    // no need to keep the media now
    libvlc_media_release(protectarea_m);

    //gUI->listWidget->setIconSize(QSize(180,200));







#else
    QSettings *configIniRead = new QSettings("./cfg/streamer.ini", QSettings::IniFormat);
    configIniRead->setIniCodec("UTF-8");
    bool cfgstate = configIniRead->value("/video1/cfgstate").toBool();
    if(!cfgstate) {
        ui->label->setText(QString("画面1:    未配置"));
        ui->display->setDisabled(true);
        ui->rtmp->setDisabled(true);
    } else {
        ui->label->setText(QString("画面1:    ") + configIniRead->value("/video1/name").toString());
        ui->display->setDisabled(false);
        ui->rtmp->setDisabled(false);
    }

    cfgstate = configIniRead->value("/video2/cfgstate").toBool();
    if(!cfgstate) {
        ui->label_2->setText(QString("画面2:    未配置"));
        ui->display_2->setDisabled(true);
        ui->rtmp_2->setDisabled(true);
    } else {
        ui->label_2->setText(QString("画面2:    ") + configIniRead->value("/video2/name").toString());
        ui->display_2->setDisabled(false);
        ui->rtmp_2->setDisabled(false);
    }

    cfgstate = configIniRead->value("/video3/cfgstate").toBool();
    if(!cfgstate) {
        ui->label_3->setText(QString("画面3:    未配置"));
        ui->display_3->setDisabled(true);
        ui->rtmp_3->setDisabled(true);
    } else {
        ui->label_3->setText(QString("画面3:    ") + configIniRead->value("/video3/name").toString());
        ui->display_3->setDisabled(false);
        ui->rtmp_3->setDisabled(false);
    }

    cfgstate = configIniRead->value("/video4/cfgstate").toBool();
    if(!cfgstate) {
        ui->label_4->setText(QString("画面4:    未配置"));
        ui->display_4->setDisabled(true);
        ui->rtmp_4->setDisabled(true);
    } else {
        ui->label_4->setText(QString("画面4:    ") + configIniRead->value("/video4/name").toString());
        ui->display_4->setDisabled(false);
        ui->rtmp_4->setDisabled(false);
    }
    delete configIniRead;
#endif
}

void MainWindow::on_choose_clicked()
{
    cfgDialog = new ImgDialog(this, 1);
    cfgDialog->setModal(true);
    cfgDialog->setWindowTitle(QStringLiteral("重点区域点选"));
    int ret = cfgDialog->exec();
    delete cfgDialog;
    
}

void MainWindow::on_choose_2_clicked()
{
    cfgDialog = new ImgDialog(this, 2);
    cfgDialog->setModal(true);
    cfgDialog->setWindowTitle(QStringLiteral("重点区域点选"));
    int ret = cfgDialog->exec();
    delete cfgDialog;
}

void MainWindow::on_choose_3_clicked()
{
    cfgDialog = new ImgDialog(this, 3);
    cfgDialog->setModal(true);
    cfgDialog->setWindowTitle(QStringLiteral("重点区域点选"));
    int ret = cfgDialog->exec();
    delete cfgDialog;
}

void MainWindow::on_choose_4_clicked()
{
    cfgDialog = new ImgDialog(this, 4);
    cfgDialog->setModal(true);
    cfgDialog->setWindowTitle(QStringLiteral("重点区域点选"));
    int ret = cfgDialog->exec();
    delete cfgDialog;
}
void MainWindow::on_display_clicked()
{
    gst_element_set_state (pipelines[0], GST_STATE_NULL);
    gst_object_unref (pipelines[0]);
    initPipeLine(ui, 0);
    ui->display->setStyleSheet("background:rgb(0,255,0)");
    ui->rtmp->setStyleSheet("background:rgb(240,240,240)");
}
void MainWindow::on_rtmp_clicked()
{
    gst_element_set_state (pipelines[0], GST_STATE_NULL);
    gst_object_unref (pipelines[0]);
    int ret = initPipeLine(ui, 0, 1);
    ui->rtmp->setStyleSheet("background:rgb(0,255,0)");
    ui->display->setStyleSheet("background:rgb(240,240,240)");
}

void MainWindow::on_display_2_clicked()
{
    gst_element_set_state (pipelines[1], GST_STATE_NULL);
    gst_object_unref (pipelines[1]);
    initPipeLine(ui, 1);
    ui->display_2->setStyleSheet("background:rgb(0,255,0)");
    ui->rtmp_2->setStyleSheet("background:rgb(240,240,240)");
}

void MainWindow::on_rtmp_2_clicked()
{
    gst_element_set_state (pipelines[1], GST_STATE_NULL);
    gst_object_unref (pipelines[1]);
    initPipeLine(ui, 1, 1);
    ui->rtmp_2->setStyleSheet("background:rgb(0,255,0)");
    ui->display_2->setStyleSheet("background:rgb(240,240,240)");
}

void MainWindow::on_display_3_clicked()
{
    gst_element_set_state (pipelines[2], GST_STATE_NULL);
    gst_object_unref (pipelines[2]);
    initPipeLine(ui, 2);
    ui->display_3->setStyleSheet("background:rgb(0,255,0)");
    ui->rtmp_3->setStyleSheet("background:rgb(240,240,240)");
}

void MainWindow::on_rtmp_3_clicked()
{
    gst_element_set_state (pipelines[2], GST_STATE_NULL);
    gst_object_unref (pipelines[2]);
    initPipeLine(ui, 2, 1);
    ui->rtmp_3->setStyleSheet("background:rgb(0,255,0)");
    ui->display_3->setStyleSheet("background:rgb(240,240,240)");
}

void MainWindow::on_rtmp_4_clicked()
{
    gst_element_set_state (pipelines[3], GST_STATE_NULL);
    gst_object_unref (pipelines[3]);
    initPipeLine(ui, 3);
    ui->rtmp_4->setStyleSheet("background:rgb(0,255,0)");
    ui->display_4->setStyleSheet("background:rgb(240,240,240)");
}

void MainWindow::on_display_4_clicked()
{
    gst_element_set_state (pipelines[3], GST_STATE_NULL);
    gst_object_unref (pipelines[3]);
    initPipeLine(ui, 3, 1);
    ui->display_4->setStyleSheet("background:rgb(0,255,0)");
    ui->rtmp_4->setStyleSheet("background:rgb(240,240,240)");
}

void MainWindow::mousedoubleclicked()
{
     ui->display_4->setStyleSheet("background:rgb(240,0,0)");
}
