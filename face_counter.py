import cv2
import numpy as np
import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision

# 1. Inisialisasi Detektor Wajah
base_options = python.BaseOptions(model_asset_path='blaze_face_full_range.tflite')
options = vision.FaceDetectorOptions(base_options=base_options)
detector = vision.FaceDetector.create_from_options(options)

# 2. Inisialisasi Kamera
video = cv2.VideoCapture(0)

print("Membuka Kamera... Tekan 'q' pada jendela video untuk keluar.")

while True:
    success, frame = video.read()
    if not success:
        print("Gagal membaca frame dari kamera.")
        break

    # Efek cermin (opsional)
    frame = cv2.flip(frame, 1)

    # MediaPipe Tasks memerlukan objek mp.Image berformat RGB
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb_frame)

    # Deteksi Wajah
    detection_result = detector.detect(mp_image)
    
    # Hitung jumlah wajah
    jumlah_wajah = len(detection_result.detections) if detection_result.detections else 0

    # Visualisasi kotak wajah
    if detection_result.detections:
        for detection in detection_result.detections:
            bbox = detection.bounding_box
            # Proteksi koordinat agar tidak minus/keluar layar
            x = max(0, int(bbox.origin_x))
            y = max(0, int(bbox.origin_y))
            w = int(bbox.width)
            h = int(bbox.height)
            
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

    # Tampilkan teks jumlah wajah di layar monitor
    cv2.putText(frame, f"Jumlah Wajah: {jumlah_wajah}", (30, 40),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 255), 2)

    cv2.imshow("Real-time Face Counter", frame)

    # Keluar dari looping jika menekan tombol 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Bersihkan resources
detector.close()
video.release()
cv2.destroyAllWindows()