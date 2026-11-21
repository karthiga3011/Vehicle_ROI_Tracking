#include "RoiTimer.h"

#include <algorithm>
#include <cstdio>

RoiTimer::RoiTimer(int graceFrames) : graceFrames_(graceFrames) {}

std::string RoiTimer::formatMmSs(double seconds) {
    if (seconds < 0.0) {
        seconds = 0.0;
    }
    const int total = static_cast<int>(seconds);
    const int mm = total / 60;
    const int ss = total % 60;
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", mm, ss);
    return std::string(buf);
}

void RoiTimer::update(int trackId, bool insideRoi, double fps) {
    if (fps <= 1e-6) {
        fps = 25.0;
    }
    State& s = states_[trackId];

    if (insideRoi) {
        if (!s.inside) {
            // Entering ROI — start / resume timer
            s.inside = true;
            // Keep previous elapsedSec if you want resume; for assignment, restart:
            // s.elapsedSec = 0.0;  // uncomment to reset every re-entry
        }
        s.missInside = 0;
        s.elapsedSec += 1.0 / fps;
        return;
    }

    // Outside ROI (or missed detection)
    if (s.inside) {
        s.missInside++;
        if (s.missInside <= graceFrames_) {
            // Grace period: still count time (handles flicker)
            s.elapsedSec += 1.0 / fps;
            return;
        }
        // Left ROI for real — freeze elapsed, mark outside
        s.inside = false;
        s.missInside = 0;
        // Keep elapsedSec so last time can still be shown if needed
    }
}

bool RoiTimer::getWaitTime(int trackId, std::string& mmss) const {
    const auto it = states_.find(trackId);
    if (it == states_.end()) {
        return false;
    }
    if (!it->second.inside && it->second.elapsedSec <= 0.0) {
        return false;
    }
    // Show timer only while considered inside (including grace)
    if (!it->second.inside) {
        return false;
    }
    mmss = formatMmSs(it->second.elapsedSec);
    return true;
}

void RoiTimer::draw(cv::Mat& frame, int trackId, const cv::Rect& box) const {
    std::string mmss;
    if (!getWaitTime(trackId, mmss)) {
        return;
    }

    // One clear label inside the top of the box (avoids overlapping nearby cars)
    const std::string text = "ID " + std::to_string(trackId) + "  Wait " + mmss;
    const double fontScale = 0.55;
    const int thickness = 1;
    int baseline = 0;
    const cv::Size size =
        cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);

    const int pad = 4;
    int x = box.x + 4;
    int y = box.y + size.height + pad + 2;
    if (y > frame.rows - 2) {
        y = frame.rows - 2;
    }
    if (x + size.width + pad * 2 > frame.cols) {
        x = std::max(0, frame.cols - size.width - pad * 2);
    }

    const cv::Rect bg(x - pad, y - size.height - pad,
                      size.width + pad * 2, size.height + baseline + pad * 2);
    cv::rectangle(frame, bg, cv::Scalar(0, 0, 0), cv::FILLED);
    cv::putText(frame, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, fontScale,
                cv::Scalar(0, 255, 255), thickness, cv::LINE_AA);
}
