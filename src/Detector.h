#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

struct Detection {
    cv::Rect box;
    float confidence = 0.0f;
    int classId = -1;
    cv::Point center() const {
        return {box.x + box.width / 2, box.y + box.height / 2};
    }
};

// YOLOv8 ONNX vehicle detector (OpenCV DNN)
class Detector {
public:
    Detector(const std::string& modelPath,
             float confThreshold = 0.40f,
             float nmsThreshold = 0.45f,
             int inputSize = 640);

    std::vector<Detection> detect(const cv::Mat& frame);

private:
    cv::Mat preprocess(const cv::Mat& frame, float& scale, int& padX, int& padY) const;
    std::vector<Detection> postprocess(const cv::Mat& output,
                                       float scale,
                                       int padX,
                                       int padY,
                                       int frameW,
                                       int frameH) const;

    cv::dnn::Net net_;
    float confThreshold_;
    float nmsThreshold_;
    int inputSize_;
};
