# Vehicle ROI Tracking

Computer Vision assignment: detect and track vehicles in a video ROI, show wait time (`MM:SS`), Dockerize the app, and (optional) GStreamer audio visualization.

**GitHub:** [karthiga3011/Vehicle_ROI_Tracking](https://github.com/karthiga3011/Vehicle_ROI_Tracking)

---

## Features

| Task | Description |
|------|-------------|
| **Task 1** | C++ + OpenCV app: YOLOv8 ONNX detection, ROI, tracking, wait timer, annotated MP4 |
| **Task 2** | Docker image to build and run the app headlessly |
| **Task 3** | GStreamer tutorials 1–3 + audio visualization bonus |

---

## Project structure

```
Vehicle_ROI_Tracking/
├── src/                      # C++ Task 1 sources (OOP)
├── CMakeLists.txt
├── Dockerfile                # Task 2
├── .dockerignore
├── input/traffic.mp4
├── models/yolov8n.onnx
├── output/result.mp4         # Sample annotated output
├── docs/Task1_Documentation.pdf
└── audio visualization/      # Task 3 (GStreamer)
    ├── tutorial1.c … tutorial3.c
    ├── Audio_Visualization.c
    └── audio.mpeg
```

### Task 1 classes

| Class | Role |
|-------|------|
| `Detector` | YOLOv8n ONNX via OpenCV DNN |
| `Tracker` | Centroid ID matching |
| `RoiManager` | Hardcoded ROI draw / point-in-ROI |
| `RoiTimer` | Per-vehicle wait time `MM:SS` |

---

## Task 1 — Build & run (Windows)

**Needs:** Visual Studio 2022 (C++), CMake, OpenCV

```bat
cd Vehicle_ROI_Tracking
mkdir build && cd build
cmake .. -DOpenCV_DIR=C:/opencv/opencv/build
cmake --build . --config Release

cd ..
set PATH=C:\opencv\opencv\build\x64\vc16\bin;%PATH%
build\Release\VehicleROI.exe
```

**PowerShell:**

```powershell
$env:PATH = "C:\opencv\opencv\build\x64\vc16\bin;" + $env:PATH
.\build\Release\VehicleROI.exe
```

**Output:** `output/result.mp4`

ROI (edit in `src/main.cpp`):

```cpp
RoiManager roiManager(cv::Rect(60, 620, 1800, 380));
```

---

## Task 2 — Docker

**Needs:** Docker Desktop (WSL2 on Windows)

```bat
docker build -t vehicle-roi .
docker run --rm -v "%cd%/output:/app/output" vehicle-roi
```

Writes `output/result.mp4` on the host. First build takes longer (compiles OpenCV 4.9 for YOLOv8 ONNX support).

---

## Task 3 — GStreamer (optional)

**Needs:** GStreamer MSVC + Visual Studio C++

```bat
cd "audio visualization"
set GSTREAMER_ROOT=C:\Users\Acer\AppData\Local\Programs\gstreamer\1.0\msvc_x86_64
set PATH=%GSTREAMER_ROOT%\bin;%PATH%

cl Audio_Visualization.c /I"%GSTREAMER_ROOT%\include\gstreamer-1.0" /I"%GSTREAMER_ROOT%\include\glib-2.0" /I"%GSTREAMER_ROOT%\lib\glib-2.0\include" /link /LIBPATH:"%GSTREAMER_ROOT%\lib" gstreamer-1.0.lib gobject-2.0.lib glib-2.0.lib /OUT:Audio_Visualization.exe
```

**Run (PowerShell):**

```powershell
cd "audio visualization"
$env:PATH = "C:\Users\Acer\AppData\Local\Programs\gstreamer\1.0\msvc_x86_64\bin;" + $env:PATH
.\Audio_Visualization.exe
```

Uses `audio.mpeg` in the same folder (visualization window via playbin).

Tutorials: `tutorial1.exe`, `tutorial2.exe`, `tutorial3.exe` (same GStreamer PATH).

---

## Documentation

- `docs/Task1_Documentation.pdf` — approach, OOP design, build steps, wait-time edge cases

---

## Notes

- Vehicle classes: COCO car, motorcycle, bus, truck  
- Detection every 2nd frame for CPU throughput  
- `HEADLESS=1` disables GUI (used in Docker)  
- Large file: `output/result.mp4` (~70 MB); Git may warn above 50 MB  

---

## License / tools

Open-source stack: OpenCV, Ultralytics YOLOv8 (ONNX), GStreamer.
