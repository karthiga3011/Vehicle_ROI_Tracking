import onnxruntime as ort

model_path = "models/yolov8n.onnx"

session = ort.InferenceSession(
    model_path,
    providers=["CPUExecutionProvider"]
)

print("ONNX model loaded successfully!")
print("Input:", session.get_inputs()[0].shape)
print("Output:", session.get_outputs()[0].shape)
print("Providers:", session.get_providers())