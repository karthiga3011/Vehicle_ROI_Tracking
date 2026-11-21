#pragma once

#include <opencv2/opencv.hpp>

// Hardcoded Region of Interest (Option A)
class RoiManager {
public:
    // Change these 4 numbers to move/resize the green ROI box on your video
    // Format: x, y, width, height  (pixels from top-left of the frame)
    explicit RoiManager(cv::Rect roi = cv::Rect(60, 620, 1800, 380));

    const cv::Rect& getRoi() const;
    bool contains(const cv::Point& point) const;
    void draw(cv::Mat& frame) const;

private:
    cv::Rect roi_;
};
