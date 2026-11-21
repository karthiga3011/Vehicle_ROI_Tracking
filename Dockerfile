# Task 2: Dockerize Vehicle ROI Tracking (C++ + OpenCV 4.9)
# Ubuntu apt OpenCV 4.5.4 cannot run YOLOv8 ONNX reliably — build OpenCV 4.9.0
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV HEADLESS=1

# Build tools + runtime deps for OpenCV video/DNN
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    pkg-config \
    wget \
    ca-certificates \
    ffmpeg \
    libavcodec-dev \
    libavformat-dev \
    libavutil-dev \
    libswscale-dev \
    libgtk-3-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    && rm -rf /var/lib/apt/lists/*

# Build OpenCV 4.9.0 (needed modules only — supports YOLOv8 ONNX)
WORKDIR /tmp/opencv_build
RUN git clone --depth 1 --branch 4.9.0 https://github.com/opencv/opencv.git && \
    mkdir -p opencv/build && cd opencv/build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          -DBUILD_LIST=core,imgproc,imgcodecs,videoio,dnn,highgui \
          -DBUILD_TESTS=OFF \
          -DBUILD_PERF_TESTS=OFF \
          -DBUILD_EXAMPLES=OFF \
          -DBUILD_opencv_apps=OFF \
          -DWITH_FFMPEG=ON \
          -DWITH_GTK=ON \
          -DOPENCV_GENERATE_PKGCONFIG=ON \
          .. && \
    cmake --build . -j$(nproc) && \
    cmake --install . && \
    ldconfig && \
    cd / && rm -rf /tmp/opencv_build

WORKDIR /app

COPY CMakeLists.txt ./
COPY src/ ./src/
COPY input/ ./input/
COPY models/yolov8n.onnx ./models/yolov8n.onnx

RUN mkdir -p output build

WORKDIR /app/build
RUN cmake .. && cmake --build . --config Release -j$(nproc)

WORKDIR /app
CMD ["./build/VehicleROI"]
