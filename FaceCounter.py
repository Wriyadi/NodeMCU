import cv2
import numpy as np
import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision

# STEP 2: Create a FaceDetector object.
base_options = python.BaseOptions(model_asset_path='detector.tflite')
options = vision.FaceDetectorOptions(base_options=base_options)
detector = vision.FaceDetector.create_from_options(options)

# STEP 3: Load the input image.
IMAGE_FILE = 'input_image.jpg'  # Ganti dengan nama file gambar Anda
image = mp.Image.create_from_file(IMAGE_FILE)

# STEP 4: Detect faces in the input image.
detection_result = detector.detect(image)

# STEP 5: Process the detection result and count faces
image_copy = np.copy(image.numpy_view())

# --- LOGIKA MENGHITUNG WAJAH ---
jumlah_wajah = 0
if detection_result.detections:
    jumlah_wajah = len(detection_result.detections)

print(f"Jumlah wajah yang terdeteksi: {jumlah_wajah}")
# --------------------------------

# Menggambar kotak di wajah secara manual jika fungsi 'visualize' bawaan tidak ada
annotated_image = cv2.cvtColor(image_copy, cv2.COLOR_RGB2BGR)
if detection_result.detections:
    for detection in detection_result.detections:
        bbox = detection.bounding_box
        start_point = (int(bbox.origin_x), int(bbox.origin_y))
        end_point = (int(bbox.origin_x + bbox.width), int(bbox.origin_y + bbox.height))
        # Gambar kotak hijau di setiap wajah
        cv2.rectangle(annotated_image, start_point, end_point, (0, 255, 0), 3)

# Tulis jumlah wajah ke dalam gambar
cv2.putText(annotated_image, f"Total Wajah: {jumlah_wajah}", (30, 50),
            cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)

# Tampilkan Gambar
# JIKA DI PC/LAPTOP LOKAL:
cv2.imshow("Hasil Deteksi", annotated_image)
cv2.waitKey(0)
cv2.destroyAllWindows()

# JIKA DI GOOGLE COLAB (Hapus tanda komentar di bawah jika pakai Colab):
# from google.colab.patches import cv2_imshow
# cv2_imshow(annotated_image)
