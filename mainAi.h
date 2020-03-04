#include <cassert>
#include <chrono>
#include <cublas_v2.h>
#include <cudnn.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <time.h>
#include <unordered_map>
#include <vector>

#include "BatchStreamPPM.h"
#include "NvUffParser.h"
#include "logger.h"
#include "common.h"
#include "argsParser.h"
#include "NvInferPlugin.h"
#include "EntropyCalibrator.h"
#include <opencv2/opencv.hpp>
#include "shapeArray.h"

using namespace nvinfer1;
using namespace nvuffparser;
const char* INPUT_BLOB_NAME = "input/input_data";
const char* OUTPUT_BLOB_NAME0 = "conv_sbbox/BiasAdd";
const char* OUTPUT_BLOB_NAME1 = "conv_mbbox/BiasAdd";
const char* OUTPUT_BLOB_NAME2 = "conv_lbbox/BiasAdd";

//INT8 Calibration, currently set to calibrate over 100 images
static constexpr int CAL_BATCH_SIZE = 10;
static constexpr int FIRST_CAL_BATCH = 0, NB_CAL_BATCHES = 10;

std::string locateFile(const std::string& input)
{
    std::vector<std::string> dirs{"model/"};
    return locateFile(input, dirs);
}

std::vector<std::pair<int64_t, DataType>>
calculateBindingBufferSizes(const ICudaEngine& engine, int nbBindings, int batchSize)
{
    std::vector<std::pair<int64_t, DataType>> sizes;
    for (int i = 0; i < nbBindings; ++i)
    {
        Dims dims = engine.getBindingDimensions(i);
        DataType dtype = engine.getBindingDataType(i);

        int64_t eltCount = samplesCommon::volume(dims) * batchSize;
        sizes.push_back(std::make_pair(eltCount, dtype));
    }

    return sizes;
}

ICudaEngine* loadModelAndCreateEngine(const char* uffFile, int maxBatchSize,
                                      IUffParser* parser, IInt8Calibrator* calibrator, IHostMemory*& trtModelStream)
{
    // Create the builder
    IBuilder* builder = createInferBuilder(gLogger.getTRTLogger());
    assert(builder != nullptr);

    // Parse the UFF model to populate the network, then set the outputs.
    INetworkDefinition* network = builder->createNetwork();

    gLogInfo << "Begin parsing model..." << std::endl;
    if (!parser->parse(uffFile, *network, nvinfer1::DataType::kFLOAT))
    {
        gLogError << "Failure while parsing UFF file" << std::endl;
        return nullptr;
    }

    gLogInfo << "End parsing model..." << std::endl;

    // Build the engine.
    builder->setMaxBatchSize(maxBatchSize);
    // The _GB literal operator is defined in common/common.h
    builder->setMaxWorkspaceSize(1_GB); // We need about 1GB of scratch space for the plugin layer for batch size 5.
    /*if (gArgs.runInInt8)
    {
        builder->setInt8Mode(gArgs.runInInt8);
        builder->setInt8Calibrator(calibrator);
    }*/

    builder->setFp16Mode(true);
    //samplesCommon::enableDLA(builder, gArgs.useDLACore);

    gLogInfo << "Begin building engine..." << std::endl;
    ICudaEngine* engine = builder->buildCudaEngine(*network);
    if (!engine)
    {
        gLogError << "Unable to create engine" << std::endl;
        return nullptr;
    }
    gLogInfo << "End building engine..." << std::endl;

    // We don't need the network any more, and we can destroy the parser.
    network->destroy();
    parser->destroy();

    // Serialize the engine, then close everything down.
    trtModelStream = engine->serialize();

    builder->destroy();
    shutdownProtobufLibrary();
    return engine;
}

void decode(ShapeArray<float>& conv_output, ShapeArray<float>& anchor, float stride, ShapeArray<float>& rltArray, int classNum)
{     
    /*
      utils.py
      def decode(conv_output, anchor, stride)
      conv_shape       = conv_output.shape
      batch_size       = conv_shape[0]
      output_size      = conv_shape[1]

      conv_output = np.reshape(conv_output, (batch_size, output_size, output_size, 3, 5 + len(classes)))

      conv_raw_dxdy = conv_output[:, :, :, :, 0:2]
      conv_raw_dwdh = conv_output[:, :, :, :, 2:4]
      conv_raw_conf = conv_output[:, :, :, :, 4:5]
      conv_raw_prob = conv_output[:, :, :, :, 5: ]

      col = np.tile(np.arange(0, output_size), output_size).reshape(-1, output_size)
      row = np.tile(np.arange(0, output_size).reshape(-1, 1), output_size)

      col = col.reshape(output_size, output_size, 1, 1).repeat(3, axis=-2)
      row = row.reshape(output_size, output_size, 1, 1).repeat(3, axis=-2)
      xy_grid = np.concatenate((col, row), axis=-1)

    */
    const std::vector<int> conv_shape = conv_output.getShape();
    int batch_size = conv_shape[0];
    int output_size = conv_shape[1];
  assert(conv_output.reshape({batch_size, output_size, output_size, 2,5+classNum}));
   //   assert(conv_output.reshape({batch_size, output_size, output_size, 3, 5+classNum}));

    ShapeArray<float> conv_raw_dxdy;
    ShapeArray<float> conv_raw_dwdh;
    ShapeArray<float> conv_raw_conf;
    ShapeArray<float> conv_raw_prob;
    conv_output.subArray(4, 0, 2, conv_raw_dxdy);
    conv_output.subArray(4, 2, 4, conv_raw_dwdh);
    conv_output.subArray(4, 4, 5, conv_raw_conf);
    conv_output.subArray(4, 5, 5+classNum, conv_raw_prob);

    ShapeArray<float> col;
    col.arange(0, output_size,1);
    col.tile({output_size});
    col.reshape({-1, output_size});

    ShapeArray<float> row;
    row.arange(0, output_size,1);
    row.reshape({-1, 1});
    row.tile({output_size});

    /*
      col = col.reshape(output_size, output_size, 1, 1).repeat(3, axis=-2)
      row = row.reshape(output_size, output_size, 1, 1).repeat(3, axis=-2)
      xy_grid = np.concatenate((col, row), axis=-1)

*/
    col.reshape({output_size, output_size, 1, 1});
    col.repeat({2},-2);
    
    row.reshape({output_size, output_size, 1, 1});
    row.repeat({2},-2);
    col.concatenate(row, -1);  //col = xy_grid

    /*
    pred_xy = (sigmoid(conv_raw_dxdy) + xy_grid) * stride
    pred_wh = (exp(conv_raw_dwdh) * anchor) * stride
    pred_xywh = np.concatenate([pred_xy, pred_wh], axis=-1)
*/

    ShapeArray<float> pred_xy;
    conv_raw_dxdy.sigmoid(pred_xy);
    pred_xy.add(col);
    pred_xy.multiply(stride);
    //pred_xy.print();
    ShapeArray<float> pred_wh;
    conv_raw_dwdh.exp(pred_wh);
    pred_wh.multiply(anchor);
    pred_wh.multiply(stride);
    pred_xy.concatenate(pred_wh, -1); //pred_xy = pred_xywh
    
    /*
    pred_conf = sigmoid(conv_raw_conf)
    pred_prob = sigmoid(conv_raw_prob)
*/
    ShapeArray<float> pred_conf;
    ShapeArray<float> pred_prob;
    conv_raw_conf.sigmoid(pred_conf);
    conv_raw_prob.sigmoid(pred_prob);

    /*
    return np.concatenate([pred_xywh, pred_conf, pred_prob], axis=-1)
*/
    pred_xy.concatenate(pred_conf, -1);
    pred_xy.concatenate(pred_prob, -1);
    rltArray.setData(pred_xy.getData(), pred_xy.getSize());
    rltArray.reshape(pred_xy.getShape());
}

void decode_2(ShapeArray<float>& conv_output, ShapeArray<float>& anchor, float stride, ShapeArray<float>& rltArray, int classNum)
{     
    /*
      utils.py
      def decode(conv_output, anchor, stride)
      conv_shape       = conv_output.shape
      batch_size       = conv_shape[0]
      output_size      = conv_shape[1]

      conv_output = np.reshape(conv_output, (batch_size, output_size, output_size, 3, 5 + len(classes)))

      conv_raw_dxdy = conv_output[:, :, :, :, 0:2]
      conv_raw_dwdh = conv_output[:, :, :, :, 2:4]
      conv_raw_conf = conv_output[:, :, :, :, 4:5]
      conv_raw_prob = conv_output[:, :, :, :, 5: ]

      col = np.tile(np.arange(0, output_size), output_size).reshape(-1, output_size)
      row = np.tile(np.arange(0, output_size).reshape(-1, 1), output_size)

      col = col.reshape(output_size, output_size, 1, 1).repeat(3, axis=-2)
      row = row.reshape(output_size, output_size, 1, 1).repeat(3, axis=-2)
      xy_grid = np.concatenate((col, row), axis=-1)

    */
    const std::vector<int> conv_shape = conv_output.getShape();
    int batch_size = conv_shape[0];
    int output_size = conv_shape[1];
    assert(conv_output.reshape({batch_size, output_size, output_size, 2, 5+classNum}));

    ShapeArray<float> conv_raw_dxdy;
    ShapeArray<float> conv_raw_dwdh;
    ShapeArray<float> conv_raw_conf;
    ShapeArray<float> conv_raw_prob;
    conv_output.subArray(4, 0, 2, conv_raw_dxdy);
    conv_output.subArray(4, 2, 4, conv_raw_dwdh);
    conv_output.subArray(4, 4, 5, conv_raw_conf);
    conv_output.subArray(4, 5, 5+classNum, conv_raw_prob);

    ShapeArray<float> col;
    col.arange(0, output_size,1);
    col.tile({output_size});
    col.reshape({-1, output_size});

    ShapeArray<float> row;
    row.arange(0, output_size,1);
    row.reshape({-1, 1});
    row.tile({output_size});

    /*
      col = col.reshape(output_size, output_size, 1, 1).repeat(3, axis=-2)
      row = row.reshape(output_size, output_size, 1, 1).repeat(3, axis=-2)
      xy_grid = np.concatenate((col, row), axis=-1)

*/
    col.reshape({output_size, output_size, 1, 1});
    col.repeat({2},-2);
    
    row.reshape({output_size, output_size, 1, 1});
    row.repeat({2},-2);
    col.concatenate(row, -1);  //col = xy_grid

    /*
    pred_xy = (sigmoid(conv_raw_dxdy) + xy_grid) * stride
    pred_wh = (exp(conv_raw_dwdh) * anchor) * stride
    pred_xywh = np.concatenate([pred_xy, pred_wh], axis=-1)
*/

    ShapeArray<float> pred_xy;
    conv_raw_dxdy.sigmoid(pred_xy);
    pred_xy.add(col);
    pred_xy.multiply(stride);
    //pred_xy.print();
    ShapeArray<float> pred_wh;
    conv_raw_dwdh.exp(pred_wh);
    pred_wh.multiply(anchor);
    pred_wh.multiply(stride);
    pred_xy.concatenate(pred_wh, -1); //pred_xy = pred_xywh
    
    /*
    pred_conf = sigmoid(conv_raw_conf)
    pred_prob = sigmoid(conv_raw_prob)
*/
    ShapeArray<float> pred_conf;
    ShapeArray<float> pred_prob;
    conv_raw_conf.sigmoid(pred_conf);
    conv_raw_prob.sigmoid(pred_prob);

    /*
    return np.concatenate([pred_xywh, pred_conf, pred_prob], axis=-1)
*/
    pred_xy.concatenate(pred_conf, -1);
    pred_xy.concatenate(pred_prob, -1);
    rltArray.setData(pred_xy.getData(), pred_xy.getSize());
    rltArray.reshape(pred_xy.getShape());
}

void doInference(IExecutionContext& context, float* inputData, int batchSize, ShapeArray<float>* decodeRlt, float* anchorlist, float* stride, const vector<int>* shapes, int input_h, int input_w, int classNum)
{

    const ICudaEngine& engine = context.getEngine();

    int nbBindings = engine.getNbBindings();
    std::vector<void*> buffers(nbBindings);
    std::vector<std::pair<int64_t, DataType>> buffersSizes = calculateBindingBufferSizes(engine, nbBindings, batchSize);
    for (int i = 0; i < nbBindings; ++i)
    {
        auto bufferSizesOutput = buffersSizes[i];
        buffers[i] = samplesCommon::safeCudaMalloc(bufferSizesOutput.first * samplesCommon::getElementSize(bufferSizesOutput.second));
    }

    int inputIndex = engine.getBindingIndex(INPUT_BLOB_NAME);
    int outputIndex[3];
    outputIndex[0] = engine.getBindingIndex(OUTPUT_BLOB_NAME0),
            outputIndex[1] = outputIndex[0] + 1,
            outputIndex[2] = outputIndex[0] + 2;

    cudaStream_t stream;
    CHECK(cudaStreamCreate(&stream));
    CHECK(cudaMemcpyAsync(buffers[inputIndex], inputData, batchSize * INPUT_C * input_h * input_w * sizeof(float), cudaMemcpyHostToDevice, stream));
    auto t_start = std::chrono::high_resolution_clock::now();
    context.execute(batchSize, &buffers[0]);
    auto t_end = std::chrono::high_resolution_clock::now();
    float total = std::chrono::duration<float, std::milli>(t_end - t_start).count();
    //gLogInfo << "Time taken for inference is " << total << " ms." << std::endl;
    ShapeArray<float> outputArray[3];
    ShapeArray<float> anchors[3];
    anchors[0].setData(anchorlist, 4);
    anchors[0].reshape({2,2});
    anchors[1].setData(anchorlist+4, 4);
    anchors[1].reshape({2,2});
    anchors[2].setData(anchorlist+8, 4);
    anchors[2].reshape({2,2});
    
    for(int i = 0; i < 3; ++i)
    {
        float* output = new float[buffersSizes[outputIndex[i]].first];
        size_t memSiz = buffersSizes[outputIndex[i]].first * samplesCommon::getElementSize(buffersSizes[outputIndex[i]].second);
        CHECK(cudaMemcpyAsync(output, buffers[outputIndex[i]], memSiz, cudaMemcpyDeviceToHost));
        outputArray[i].setData(output, buffersSizes[outputIndex[i]].first);
        delete[] output;
        //outputArray[i].print();
        assert(outputArray[i].reshape(shapes[i]));
        decode(outputArray[i], anchors[i], stride[i], decodeRlt[i], classNum);
    }
    //Release the stream and the buffers
    cudaStreamDestroy(stream);
    CHECK(cudaFree(buffers[inputIndex]));
    CHECK(cudaFree(buffers[outputIndex[0]]));
    CHECK(cudaFree(buffers[outputIndex[1]]));
    CHECK(cudaFree(buffers[outputIndex[2]]));
}

void doInference_2(IExecutionContext& context, float* inputData, int batchSize, ShapeArray<float>* decodeRlt, float* anchorlist, float* stride, const vector<int>* shapes, int input_h, int input_w, int classNum)
{

    const ICudaEngine& engine = context.getEngine();

    int nbBindings = engine.getNbBindings();
    std::vector<void*> buffers(nbBindings);
    std::vector<std::pair<int64_t, DataType>> buffersSizes = calculateBindingBufferSizes(engine, nbBindings, batchSize);
    for (int i = 0; i < nbBindings; ++i)
    {
        auto bufferSizesOutput = buffersSizes[i];
        buffers[i] = samplesCommon::safeCudaMalloc(bufferSizesOutput.first * samplesCommon::getElementSize(bufferSizesOutput.second));
    }

    int inputIndex = engine.getBindingIndex(INPUT_BLOB_NAME);
    int outputIndex[2];
    outputIndex[0] = engine.getBindingIndex(OUTPUT_BLOB_NAME0),
            outputIndex[1] = outputIndex[0] + 1;

    cudaStream_t stream;
    CHECK(cudaStreamCreate(&stream));
    CHECK(cudaMemcpyAsync(buffers[inputIndex], inputData, batchSize * INPUT_C * input_h * input_w * sizeof(float), cudaMemcpyHostToDevice, stream));
    auto t_start = std::chrono::high_resolution_clock::now();
    context.execute(batchSize, &buffers[0]);
    auto t_end = std::chrono::high_resolution_clock::now();
    float total = std::chrono::duration<float, std::milli>(t_end - t_start).count();
    //gLogInfo << "Time taken for inference is " << total << " ms." << std::endl;
    ShapeArray<float> outputArray[2];
    ShapeArray<float> anchors[2];
    anchors[0].setData(anchorlist, 4);
    anchors[0].reshape({2,2});
    anchors[1].setData(anchorlist+4, 4);
    anchors[1].reshape({2,2});
    for(int i = 0; i < 2; ++i)
    {
        float* output = new float[buffersSizes[outputIndex[i]].first];
        size_t memSiz = buffersSizes[outputIndex[i]].first * samplesCommon::getElementSize(buffersSizes[outputIndex[i]].second);
        CHECK(cudaMemcpyAsync(output, buffers[outputIndex[i]], memSiz, cudaMemcpyDeviceToHost));
        outputArray[i].setData(output, buffersSizes[outputIndex[i]].first);
        delete[] output;
        assert(outputArray[i].reshape(shapes[i]));
        decode_2(outputArray[i], anchors[i], stride[i], decodeRlt[i], classNum);
    }
    //Release the stream and the buffers
    cudaStreamDestroy(stream);
    CHECK(cudaFree(buffers[inputIndex]));
    CHECK(cudaFree(buffers[outputIndex[0]]));
    CHECK(cudaFree(buffers[outputIndex[1]]));
}


void postprocess_boxes(ShapeArray<float>& pred_bbox, float img_old_width, float img_old_height, int input_size, float score_threshold, ShapeArray<float>& out_boxes, int classNum)
{
    float valid_scale[2]={0, 9999999.0};

    ShapeArray<float>pred_xywh;
    pred_bbox.subArray(1, 0, 4, pred_xywh);
    ShapeArray<float>pred_conf;
    pred_bbox.subArray(1, 4, 5, pred_conf);
    ShapeArray<float>pred_prob;
    pred_bbox.subArray(1, 5, 5+classNum, pred_prob);

    ShapeArray<float>pred_coor;
    pred_xywh.subArray(1,0,2, pred_coor);
    ShapeArray<float>tmp1;
    pred_xywh.subArray(1,2,4, tmp1);
    tmp1.multiply(0.5);
    pred_coor.subtract(tmp1);

    ShapeArray<float>tmp2;
    pred_xywh.subArray(1,0,2, tmp2);
    tmp2.add(tmp1);
    pred_coor.concatenate(tmp2, -1);
    
    float resize_ratio = min(input_size / img_old_width, input_size / img_old_height);

    float dw = (input_size - resize_ratio * img_old_width) / 2;
    float dh = (input_size - resize_ratio * img_old_height) / 2;
    
    pred_coor.subArray(1, 0, 3, 2, tmp1);
    tmp1.subtract(dw);
    tmp1.division(resize_ratio);
    tmp1.multiply(1.0);

    pred_coor.subArray(1, 1, 4, 2, tmp2);
    tmp2.subtract(dh);
    tmp2.division(resize_ratio);
    tmp2.multiply(1.0);
    pred_coor.setSub(1, 0, 3, 2, tmp1);
    pred_coor.setSub(1, 1, 4, 2, tmp2);

    ShapeArray<float> tmp3;
    ShapeArray<float> tmp4;
    pred_coor.subArray(1, 0, 2, tmp1);
    ShapeArray<float> cmpArray;
    float cmp1[] = {0, 0};
    float cmp2[] = {img_old_width - 1, img_old_height -1};

    cmpArray.setData(cmp1, 2);
    cmpArray.broadcast(tmp1.getShape());
    tmp1.maximum(cmpArray, tmp3);
    
    pred_coor.subArray(1, 2, 4, tmp1);
    cmpArray.clear();
    cmpArray.setData(cmp2, 2);
    cmpArray.broadcast(tmp1.getShape());
    tmp1.minimum(cmpArray, tmp4);

    pred_coor.clear();
    pred_coor.setData(tmp3.getData(), tmp3.getSize());
    pred_coor.reshape(tmp3.getShape());
    pred_coor.concatenate(tmp4,-1);

    ShapeArray<float> rltArray;
    ShapeArray<float> rltArray1;
    ShapeArray<float> rltArray2;
    pred_coor.subArray(1, 0, 1, tmp1);
    pred_coor.subArray(1, 2, 3, tmp2);
    tmp1.less(tmp2, rltArray1);
    pred_coor.subArray(1, 1, 2, tmp1);
    pred_coor.subArray(1, 3, 4, tmp2);
    tmp1.less(tmp2, rltArray2);
    rltArray1.logical_and(rltArray2, rltArray);
    rltArray.broadcast(pred_coor.getShape());
    pred_coor.multiply(rltArray);
    
    pred_coor.subArray(1, 2, 4, tmp1);
    pred_coor.subArray(1, 0, 2, tmp2);
    tmp1.subtract(tmp2);
    tmp1.multiply_reduce(-1, tmp2);
    tmp2.sqrt(tmp1);// tmp1 = bboxes_scale
    
    tmp1.greater(valid_scale[0], tmp2);
    tmp1.less(valid_scale[1], tmp3);
    tmp2.logical_and(tmp3, tmp1);// tmp1 = scale_mask

    // bboxes_scale = np.sqrt(np.multiply.reduce(pred_coor[:, 2:4] - pred_coor[:, 0:2], axis=-1))
    //scale_mask = np.logical_and((valid_scale[0] < bboxes_scale), (bboxes_scale < valid_scale[1]))
    ShapeArray<float> classes;
    pred_prob.argmax(-1, classes);
    long pred_coor_len = pred_coor.getSize()/4;
    const float* pred_prob_data = pred_prob.getData();
    float *tmpData = new float[pred_coor_len];
    for(int i=0; i<pred_coor_len; ++i) {
        int index = i*classNum + classes.getData()[i];
        tmpData[i] = pred_prob_data[index];
    }
    tmp2.clear();
    tmp2.setData(tmpData, pred_coor_len);
    delete[] tmpData;
    pred_conf.reshape({-1});
    pred_conf.multiply(tmp2);//pred_conf = scores
    
    pred_conf.greater(score_threshold, tmp2); //tmp2 = score_mask
    tmp1.logical_and(tmp2, tmp3);// tmp3 = mask
    pred_coor.subArrayLastlayer(tmp3, rltArray); //coors
    pred_conf.subArrayLastlayer(tmp3, rltArray1);//scores
    classes.subArrayLastlayer(tmp3, rltArray2);//classes
    /*rltArray1.reshape({-1,1});
    rltArray.concatenate(rltArray1, -1);
    classes.subArrayLastlayer(tmp3, rltArray2);
    rltArray2.reshape({-1,1});
    rltArray.concatenate(rltArray2, -1);
    rltArray.print();*/
    //nms
    const float finfo = 1.19209290e-07;
    float tmpdata[6] = {0,0,0,0,0,0};
    ShapeArray<float> best_boxes;
    best_boxes.setData(tmpdata, 6);
    for(int i=0; i < classNum; ++i) {
        float cls = i;
        rltArray2.equal(cls, tmp1);
        rltArray.subArrayLastlayer(tmp1, tmp2);
        ShapeArray<float> subscores;
        rltArray1.subArrayLastlayer(tmp1, subscores);
        ShapeArray<float> subclasses;
        rltArray2.subArrayLastlayer(tmp1, subclasses);
        while(tmp2.getSize() > 0) {
            subscores.argmax(-1, tmp3);
            int bestIndex = tmp3.getData()[0];
            int len = tmp2.getSize()/4;
            ShapeArray<float> cls_bboxes;
            ShapeArray<float> best_box;
            ShapeArray<float> best_score;
            ShapeArray<float> best_class;
            ShapeArray<float> left;
            ShapeArray<float> right;
            ShapeArray<float> right2;
            ShapeArray<float> top;
            ShapeArray<float> bottom;
            ShapeArray<float> cls_scores;
            ShapeArray<float> clses;
            tmp2.subArray(0, 0, bestIndex, cls_bboxes);
            tmp2.subArray(0, bestIndex+1, len, right);
            cls_bboxes.concatenate(right);
            tmp2.subArray(0, bestIndex, bestIndex+1, best_box);
            subscores.subArray(0, bestIndex, bestIndex+1, best_score);
            subclasses.subArray(0, bestIndex, bestIndex+1, best_class);
            //best_class.setData(&cls, 1);

            subscores.subArray(0, 0, bestIndex, cls_scores);
            subscores.subArray(0, bestIndex+1, len, right);
            cls_scores.concatenate(right);

            subclasses.subArray(0, 0, bestIndex, clses);
            subclasses.subArray(0, bestIndex+1, len, right);
            clses.concatenate(right);

            best_box.subArray(1, 2, 3, right);
            best_box.subArray(1, 0, 1, left);
            best_box.subArray(1, 3, 4, bottom);
            best_box.subArray(1, 1, 2, top);
            right.subtract(left);
            bottom.subtract(top);
            right.multiply(bottom);

            cls_bboxes.subArray(1, 2, 3, right2);
            cls_bboxes.subArray(1, 0, 1, left);
            cls_bboxes.subArray(1, 3, 4, bottom);
            cls_bboxes.subArray(1, 1, 2, top);
            right2.subtract(left);
            bottom.subtract(top);
            right2.multiply(bottom);
            ShapeArray<float> left_up_1;
            ShapeArray<float> left_up_2;
            ShapeArray<float> left_up;
            best_box.subArray(1, 0, 2, left_up_1);
            cls_bboxes.subArray(1, 0, 2, left_up_2);
            left_up_1.broadcast(left_up_2.getShape());
            left_up_1.maximum(left_up_2, left_up);
            ShapeArray<float> right_down_1;
            ShapeArray<float> right_down_2;
            ShapeArray<float> right_down;
            best_box.subArray(1, 2, 4, right_down_1);
            cls_bboxes.subArray(1, 2, 4, right_down_2);
            right_down_1.broadcast(right_down_2.getShape());
            right_down_1.minimum(right_down_2, right_down);
            ShapeArray<float> inter;
            right_down.subtract(left_up);
            right_down.maximum(0.0, inter);
            ShapeArray<float> inter_area;
            ShapeArray<float> inter_h;
            inter.subArray(1, 0, 1, inter_area);
            inter.subArray(1, 1, 2, inter_h);
            inter_area.multiply(inter_h);

            right.add(right2);
            right.subtract(inter_area);
            inter_area.division(right);
            ShapeArray<float> ious;
            inter_area.maximum(finfo, ious);
            float * weight = new float[ious.getSize()];
            for(int j = 0; j < ious.getSize(); ++j) {
                weight[j] = 1.0;
            }
            ShapeArray<float> weightArray(weight, ious.getSize());
            delete[] weight;
            ShapeArray<float> ious_mask;
            float iou_threshold = 0.5;
            ious.greater(iou_threshold, ious_mask);
            ious_mask.reshape({-1});
            weightArray.reshape({-1});
            weightArray.multiply(ious_mask);
            weightArray.subtract(1.0);
            weightArray.multiply(-1.0);
            cls_scores.multiply(weightArray);
            cls_scores.greater(0.0, ious_mask);
            cls_bboxes.subArrayLastlayer(ious_mask, tmp2);
            cls_scores.subArrayLastlayer(ious_mask, subscores);
            clses.subArrayLastlayer(ious_mask, subclasses);
            best_box.reshape({-1});
            best_boxes.concatenate(best_box);
            best_boxes.concatenate(best_score);
            best_boxes.concatenate(best_class);
        }
    }
    
    best_boxes.reshape({-1,6});
    //best_boxes.print();
    best_boxes.subArray(0, 1, best_boxes.getSize()/6, out_boxes);
    //out_boxes.print();
    //classes = np.argmax(pred_prob, axis=-1)
    //scores = pred_conf * pred_prob[np.arange(len(pred_coor)), classes]
    //score_mask = scores > score_threshold
    //mask = np.logical_and(scale_mask, score_mask)
    //coors, scores, classes = pred_coor[mask], scores[mask], classes[mask]

}

/*int main(int argc, char* argv[])
{
    IRuntime* runtime = createInferRuntime(gLogger.getTRTLogger());
    assert(runtime != nullptr);
    FILE *fp = fopen("test.engine", "rb");
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

    cv::Mat img = cv::imread("/home/yskj/bin/000015.jpg", CV_LOAD_IMAGE_COLOR);
    cv::imshow("show img",img);
    cv::waitKey(0);
    cv::Mat img2;
    cv::cvtColor(img, img2, cv::COLOR_BGR2RGB);
    int ih, iw;
    ih = 608;
    iw = 608;
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
    cv::imshow("show img",imgResized);
    cv::waitKey(0);
    vector<float> data_img(ih * iw * 3);
    //float *image_paded = new float[ih * iw * 3];
    for(int i=0; i < ih*iw*3; ++i)
    {
      data_img[i] = 128.0;
    }
    int dw, dh;
    dw = (iw - nw) / 2;
    dh = (ih-nh) / 2;
    printf("scale is %f, image width = %f, height=%f, nw = %d, nh = %d\n", scale, w, h, nw, nh);
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
    const int num_class = 2;
    printf("before do this\n");
    ShapeArray<float> pred_bboxes_pre[3];
    ShapeArray<float> pred_bboxes;
    doInference(*context, &data_img[0], N, pred_bboxes_pre);
    pred_bboxes.setData(pred_bboxes_pre[0].getData(), pred_bboxes_pre[0].getSize());
    pred_bboxes.reshape({-1, 5+num_class});
    for(int i=1; i<3; ++i)
    {
      pred_bboxes_pre[i].reshape({-1, 5+num_class});
      pred_bboxes.concatenate(pred_bboxes_pre[i], 0);
    }
    for(int i=0; i<3; ++i)
    {
      pred_bboxes_pre[i].clear();
    }
    ShapeArray<float> out_boxes;
    postprocess_boxes(pred_bboxes, w, h, 608, 0.4, out_boxes);
    
    for(int i = 0; i < out_boxes.getSize()/6; ++i) {
        int cls = (int)(out_boxes.getData()[5 + i*6]);
        float score = out_boxes.getData()[4 + i*6];
        char text[10] = {0};
        sprintf(text, "%f", score);
        cv::Point pt1((int)(out_boxes.getData()[0 + i*6]), (int)(out_boxes.getData()[1 + i*6]));
        cv::Point pt2((int)(out_boxes.getData()[2 + i*6]), (int)(out_boxes.getData()[3 + i*6]));
        if(cls == 0) {
          cv::rectangle(img, pt1, pt2, cv::Scalar(255, 0, 0), 2, 8, 0);
        }
        else {
          cv::rectangle(img, pt1, pt2, cv::Scalar(0, 255, 255), 2, 8, 0);
        }
        cv::putText(img, text, pt1, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,255),1,8);
    }
    cv::imshow("show img",img);
    cv::waitKey(0);

    printf("do this2\n");

    /*char *image_paded2 = new char[ih * iw * 3];
    for(int i=0; i < ih*iw*3; ++i)
    {
      image_paded2[i] = 128;
    }
    for(int i = dh; i < nh+dh; ++i)
    {
      for(int j = dw; j < nw+dw; ++j)
      {
        image_paded2[i * iw*3 + j*3] = imgResized.data[(i-dh)*nw*3 + (j-dw)*3];
        image_paded2[i * iw*3 + j*3 + 1] = imgResized.data[(i-dh)*nw*3 + (j-dw)*3 + 1];
        image_paded2[i * iw*3 + j*3 + 2] = imgResized.data[(i-dh)*nw*3 + (j-dw)*3 + 2];
      }
    }
    cv::Size size2(608, 608);
    cv::Mat image3(size2, CV_8UC3, (void *) image_paded2);
    cv::imshow("显示灰度图",image3);
    cv::waitKey(0);*/


/*auto fileName = locateFile("yolov3.uff");
    printf("file path = %s\n", fileName.c_str());
    const int N = 1;
    auto parser = createUffParser();

    BatchStream calibrationStream(CAL_BATCH_SIZE, NB_CAL_BATCHES);

    parser->registerInput("input/input_data", DimsCHW(608, 608, 3), UffInputOrder::kNHWC);
    // MarkOutput_0 is a node created by the UFF converter when we specify an ouput with -O.
    parser->registerOutput("conv_sbbox/BiasAdd");
    parser->registerOutput("conv_mbbox/BiasAdd");
    parser->registerOutput("conv_lbbox/BiasAdd");

    IHostMemory* trtModelStream{nullptr};

    std::unique_ptr<IInt8Calibrator> calibrator;
    calibrator.reset(new Int8EntropyCalibrator2(calibrationStream, FIRST_CAL_BATCH, "UffSSD", INPUT_BLOB_NAME));

    ICudaEngine* tmpEngine = loadModelAndCreateEngine(fileName.c_str(), N, parser, calibrator.get(), trtModelStream);
    FILE *fp = fopen("test.engine", "wb");
    fwrite(trtModelStream->data(), trtModelStream->size(), 1, fp);
    fclose(fp);
    assert(tmpEngine != NULL);*/
//return 0;
//}
