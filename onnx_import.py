import onnx
from onnx import helper

# 加载原始 ONNX 模型
model_path = "/home/nuc/zz/gesture_detect/model/best.onnx"
model = onnx.load(model_path)

# 将 opset 版本降级为 10
onnx.save(onnx.helper.make_model(model.graph, producer_name='onnx-graphopt', opset_imports=[helper.make_opsetid('', 10)]), '/home/nuc/zz/gesture_detect/model/best_opset10.onnx')

print("模型已降级并保存为 best_opset10.onnx")
