from ultralytics import YOLO

model = YOLO("models/yolov8n.pt")

model.export(
    format="onnx",
    imgsz=640,
    simplify=True,
    opset=12
)

print("ONNX conversion completed!")