#include "Detector.h"
#include "RoiManager.h"
#include "RoiTimer.h"
#include "Tracker.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

int main() {
    // ---------- paths (relative to where you run the exe) ----------
    const std::string videoPath = "input/traffic.mp4";
    const std::string modelPath = "models/yolov8n.onnx";
    const std::string outputPath = "output/result.mp4";

    // ---------- Option A: HARDCODED ROI on the ROAD (1920x1080) ----------
    // cv::Rect(x, y, width, height) — lower band covering the roadway
    RoiManager roiManager(cv::Rect(60, 620, 1800, 380));

    // Preview off in Docker (HEADLESS=1) — no GUI display in container
    const char* headlessEnv = std::getenv("HEADLESS");
    const bool showPreview = !(headlessEnv && std::string(headlessEnv) == "1");
    // Run YOLO every N frames (2 = ~2x faster, still tracks OK)
    const int detectEveryN = 2;

    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "ERROR: cannot open video: " << videoPath << std::endl;
        std::cerr << "Run the exe from the project root folder." << std::endl;
        return 1;
    }

    const int frameW = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    const int frameH = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps < 1.0) {
        fps = 25.0;
    }

    std::cout << "[Main] Video " << frameW << "x" << frameH << " @ " << fps << " FPS" << std::endl;
    std::cout << "[Main] ROI = " << roiManager.getRoi() << std::endl;

    // Try a common OpenCV MP4 fourcc; fallback to AVI if needed
    cv::VideoWriter writer;
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    if (!writer.open(outputPath, fourcc, fps, cv::Size(frameW, frameH))) {
        fourcc = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');
        const std::string aviPath = "output/result.avi";
        if (!writer.open(aviPath, fourcc, fps, cv::Size(frameW, frameH))) {
            std::cerr << "ERROR: cannot open video writer" << std::endl;
            return 1;
        }
        std::cout << "[Main] Writing: " << aviPath << std::endl;
    } else {
        std::cout << "[Main] Writing: " << outputPath << std::endl;
    }

    // Higher conf = fewer false boxes (billboard/person), a bit faster NMS
    Detector detector(modelPath, 0.50f, 0.45f, 640);
    Tracker tracker;
    RoiTimer roiTimer;

    cv::Mat frame;
    std::vector<Detection> detections;
    std::vector<Track> tracks;
    int frameIndex = 0;
    const auto t0 = std::chrono::steady_clock::now();

    if (showPreview) {
        cv::namedWindow("Vehicle ROI Tracking", cv::WINDOW_NORMAL);
        cv::resizeWindow("Vehicle ROI Tracking", 960, 540);  // smaller window = less lag
    }

    while (cap.read(frame)) {
        if (frame.empty()) {
            break;
        }

        // 1) Detect vehicles (skip some frames to reduce lag)
        if (frameIndex % detectEveryN == 0) {
            detections = detector.detect(frame);
            tracks = tracker.update(detections);
        }
        // else: reuse last tracks (boxes stay, timer still updates below)

        // 3) Draw ROI
        roiManager.draw(frame);

        // 4) For each track: box + timer if inside ROI
        for (const Track& track : tracks) {
            const bool inside = roiManager.contains(track.center);
            roiTimer.update(track.id, inside, fps);

            const cv::Scalar color = inside ? cv::Scalar(0, 255, 255) : cv::Scalar(255, 128, 0);
            cv::rectangle(frame, track.box, color, 2);

            if (inside) {
                // Combined "ID + Wait MM:SS" label with background (no overlap clutter)
                roiTimer.draw(frame, track.id, track.box);
            } else {
                // Outside ROI: only small ID above box
                const std::string idText = "ID " + std::to_string(track.id);
                cv::putText(frame, idText, cv::Point(track.box.x, std::max(20, track.box.y - 8)),
                            cv::FONT_HERSHEY_SIMPLEX, 0.55, color, 1, cv::LINE_AA);
            }
        }

        // HUD
        cv::putText(frame, "Frame " + std::to_string(frameIndex), cv::Point(20, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

        writer.write(frame);

        if (showPreview) {
            cv::imshow("Vehicle ROI Tracking", frame);
            const int key = cv::waitKey(1);
            if (key == 'q' || key == 'Q') {
                break;
            }
        }

        ++frameIndex;
        if (frameIndex % 50 == 0) {
            std::cout << "[Main] processed " << frameIndex << " frames, tracks=" << tracks.size()
                      << std::endl;
        }
    }

    const auto t1 = std::chrono::steady_clock::now();
    const double sec = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "[Main] Done. Frames=" << frameIndex << " Time=" << sec
              << "s Approx FPS=" << (frameIndex / std::max(0.001, sec)) << std::endl;

    cap.release();
    writer.release();
    cv::destroyAllWindows();
    return 0;
}
