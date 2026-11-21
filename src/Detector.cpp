#include "Detector.h"

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

namespace {
// COCO class IDs: car, motorcycle, bus, truck
const std::unordered_set<int> kVehicleClasses = {2, 3, 5, 7};
}  // namespace

Detector::Detector(const std::string& modelPath,
                   float confThreshold,
                   float nmsThreshold,
                   int inputSize)
    : confThreshold_(confThreshold),
      nmsThreshold_(nmsThreshold),
      inputSize_(inputSize) {
    net_ = cv::dnn::readNetFromONNX(modelPath);
    net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    std::cout << "[Detector] Loaded model: " << modelPath << std::endl;
}

cv::Mat Detector::preprocess(const cv::Mat& frame, float& scale, int& padX, int& padY) const {
    const int w = frame.cols;
    const int h = frame.rows;
    scale = static_cast<float>(inputSize_) / static_cast<float>(std::max(w, h));
    const int newW = static_cast<int>(std::round(w * scale));
    const int newH = static_cast<int>(std::round(h * scale));
    padX = (inputSize_ - newW) / 2;
    padY = (inputSize_ - newH) / 2;

    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(newW, newH));

    cv::Mat letterbox(inputSize_, inputSize_, CV_8UC3, cv::Scalar(114, 114, 114));
    resized.copyTo(letterbox(cv::Rect(padX, padY, newW, newH)));

    cv::Mat blob;
    cv::dnn::blobFromImage(letterbox, blob, 1.0 / 255.0, cv::Size(inputSize_, inputSize_),
                           cv::Scalar(), true, false);
    return blob;
}

std::vector<Detection> Detector::postprocess(const cv::Mat& output,
                                             float scale,
                                             int padX,
                                             int padY,
                                             int frameW,
                                             int frameH) const {
    // YOLOv8 ONNX output is usually [1, 84, 8400] (or [1, 8400, 84])
    // Avoid OpenCV reshape() here — it crashes on some OpenCV 4.5.x builds.
    cv::Mat pred;  // final shape: [num_boxes, 84]

    if (output.dims == 3) {
        const int d1 = output.size[1];
        const int d2 = output.size[2];
        cv::Mat view(d1, d2, CV_32F, const_cast<float*>(output.ptr<float>()));
        if (d1 <= 85 && d1 < d2) {
            // [84, 8400] -> [8400, 84]
            cv::transpose(view, pred);
        } else {
            pred = view.clone();
        }
    } else if (output.dims == 2) {
        if (output.rows <= 85 && output.rows < output.cols) {
            cv::transpose(output, pred);
        } else {
            pred = output;
        }
    } else if (output.dims == 4) {
        // Rare: [1, 1, 84, 8400] style — flatten first two dims
        const int d1 = output.size[2];
        const int d2 = output.size[3];
        cv::Mat view(d1, d2, CV_32F, const_cast<float*>(output.ptr<float>()));
        if (d1 <= 85 && d1 < d2) {
            cv::transpose(view, pred);
        } else {
            pred = view.clone();
        }
    } else {
        std::cerr << "[Detector] Unexpected output dims=" << output.dims << std::endl;
        return {};
    }

    const int numAttrs = pred.cols;  // expect 84 = 4 + 80
    if (numAttrs < 5) {
        return {};
    }
    const int numClasses = numAttrs - 4;

    std::vector<cv::Rect> boxes;
    std::vector<float> scores;
    std::vector<int> classIds;

    for (int i = 0; i < pred.rows; ++i) {
        const float* row = pred.ptr<float>(i);
        const float cx = row[0];
        const float cy = row[1];
        const float bw = row[2];
        const float bh = row[3];

        float bestScore = 0.0f;
        int bestClass = -1;
        for (int c = 0; c < numClasses; ++c) {
            const float score = row[4 + c];
            if (score > bestScore) {
                bestScore = score;
                bestClass = c;
            }
        }

        if (bestScore < confThreshold_ || !kVehicleClasses.count(bestClass)) {
            continue;
        }

        // Undo letterbox
        float x = (cx - static_cast<float>(padX)) / scale;
        float y = (cy - static_cast<float>(padY)) / scale;
        float w = bw / scale;
        float h = bh / scale;

        int left = static_cast<int>(std::round(x - w * 0.5f));
        int top = static_cast<int>(std::round(y - h * 0.5f));
        int width = static_cast<int>(std::round(w));
        int height = static_cast<int>(std::round(h));

        left = std::clamp(left, 0, frameW - 1);
        top = std::clamp(top, 0, frameH - 1);
        width = std::clamp(width, 1, frameW - left);
        height = std::clamp(height, 1, frameH - top);

        boxes.emplace_back(left, top, width, height);
        scores.push_back(bestScore);
        classIds.push_back(bestClass);
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, scores, confThreshold_, nmsThreshold_, indices);

    std::vector<Detection> detections;
    detections.reserve(indices.size());
    for (int idx : indices) {
        detections.push_back({boxes[idx], scores[idx], classIds[idx]});
    }
    return detections;
}

std::vector<Detection> Detector::detect(const cv::Mat& frame) {
    float scale = 1.0f;
    int padX = 0;
    int padY = 0;
    cv::Mat blob = preprocess(frame, scale, padX, padY);

    net_.setInput(blob);

    // Prefer named outputs when available; fall back to default forward
    std::vector<cv::Mat> outputs;
    const auto outNames = net_.getUnconnectedOutLayersNames();
    if (!outNames.empty()) {
        net_.forward(outputs, outNames);
    } else {
        cv::Mat out = net_.forward();
        outputs.push_back(out);
    }

    if (outputs.empty() || outputs[0].empty()) {
        return {};
    }
    return postprocess(outputs[0], scale, padX, padY, frame.cols, frame.rows);
}
