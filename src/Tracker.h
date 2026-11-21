#pragma once

#include "Detector.h"

#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <vector>

struct Track {
    int id = -1;
    cv::Rect box;
    cv::Point center;
    int missedFrames = 0;
};

// Simple centroid tracker (nearest-neighbor matching)
class Tracker {
public:
    explicit Tracker(float maxDistance = 80.0f, int maxMissed = 20);

    std::vector<Track> update(const std::vector<Detection>& detections);

private:
    float maxDistance_;
    int maxMissed_;
    int nextId_ = 1;
    std::unordered_map<int, Track> tracks_;
};
