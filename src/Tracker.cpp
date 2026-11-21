#include "Tracker.h"

#include <cmath>
#include <unordered_set>

Tracker::Tracker(float maxDistance, int maxMissed)
    : maxDistance_(maxDistance), maxMissed_(maxMissed) {}

std::vector<Track> Tracker::update(const std::vector<Detection>& detections) {
    std::unordered_set<int> matchedTrackIds;
    std::vector<char> detMatched(detections.size(), 0);

    // 1) Match each detection to nearest existing track
    for (size_t di = 0; di < detections.size(); ++di) {
        float bestDist = maxDistance_;
        int bestTrackId = -1;
        const cv::Point c = detections[di].center();

        for (const auto& [tid, track] : tracks_) {
            if (matchedTrackIds.count(tid)) {
                continue;
            }
            const float dx = static_cast<float>(c.x - track.center.x);
            const float dy = static_cast<float>(c.y - track.center.y);
            const float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < bestDist) {
                bestDist = dist;
                bestTrackId = tid;
            }
        }

        if (bestTrackId >= 0) {
            Track& t = tracks_[bestTrackId];
            t.box = detections[di].box;
            t.center = c;
            t.missedFrames = 0;
            matchedTrackIds.insert(bestTrackId);
            detMatched[di] = 1;
        }
    }

    // 2) Create new tracks for unmatched detections
    for (size_t di = 0; di < detections.size(); ++di) {
        if (detMatched[di]) {
            continue;
        }
        Track t;
        t.id = nextId_++;
        t.box = detections[di].box;
        t.center = detections[di].center();
        t.missedFrames = 0;
        tracks_[t.id] = t;
        matchedTrackIds.insert(t.id);
    }

    // 3) Age unmatched tracks; drop lost ones
    std::unordered_map<int, Track> kept;
    std::vector<Track> active;
    for (auto& [tid, track] : tracks_) {
        if (!matchedTrackIds.count(tid)) {
            track.missedFrames++;
        }
        if (track.missedFrames <= maxMissed_) {
            kept[tid] = track;
            active.push_back(track);
        }
    }

    tracks_.swap(kept);
    return active;
}
