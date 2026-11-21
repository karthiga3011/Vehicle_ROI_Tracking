#include "RoiManager.h"

RoiManager::RoiManager(cv::Rect roi) : roi_(std::move(roi)) {}

const cv::Rect& RoiManager::getRoi() const {
    return roi_;
}

bool RoiManager::contains(const cv::Point& point) const {
    return roi_.contains(point);
}

void RoiManager::draw(cv::Mat& frame) const {
    cv::rectangle(frame, roi_, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, "ROI", cv::Point(roi_.x, std::max(20, roi_.y - 8)),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
}
