#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <unordered_map>

// Counts how long each track stays inside the ROI
class RoiTimer {
public:
    // graceFrames: keep timer alive if detection flickers for a few frames
    explicit RoiTimer(int graceFrames = 10);

    // Call once per frame for each active track
    void update(int trackId, bool insideRoi, double fps);

    // Returns MM:SS if this track currently has an active wait time
    bool getWaitTime(int trackId, std::string& mmss) const;

    void draw(cv::Mat& frame, int trackId, const cv::Rect& box) const;

private:
    struct State {
        bool inside = false;
        double elapsedSec = 0.0;
        int missInside = 0;
    };

    static std::string formatMmSs(double seconds);

    int graceFrames_;
    std::unordered_map<int, State> states_;
};
