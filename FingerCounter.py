import cv2
import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision

# ==========================================
# 1. Konfigurasi MediaPipe Tasks API
# ==========================================
base_options = python.BaseOptions(model_asset_path='hand_landmarker.task')
options = vision.HandLandmarkerOptions(
    base_options=base_options, 
    num_hands=2,
    min_hand_detection_confidence=0.5,
    min_hand_presence_confidence=0.5,
    min_tracking_confidence=0.5
)
detector = vision.HandLandmarker.create_from_options(options)

# ==========================================
# 2. Fungsi Manual untuk Menghitung Jari
# ==========================================
def count_fingers(hand_landmarks):
    # Urutan titik (Landmarks):
    # Jempol (Thumb): Tip = 4, IP = 3
    # Telunjuk (Index): Tip = 8, PIP = 6
    # Tengah (Middle): Tip = 12, PIP = 10
    # Manis (Ring): Tip = 16, PIP = 14
    # Kelingking (Pinky): Tip = 20, PIP = 18
    
    fingers_count = 0
    
    # Cek 4 Jari (Telunjuk, Tengah, Manis, Kelingking)
    # Jika Y Tip (ujung) lebih kecil dari Y PIP (bawahnya), berarti jari terbuka/berdiri
    tips_ids = [8, 12, 16, 20]
    pips_ids = [6, 10, 14, 18]
    
    for tip, pip in zip(tips_ids, pips_ids):
        if hand_landmarks[tip].y < hand_landmarks[pip].y:
            fingers_count += 1
            
    # Cek Jempol (Jempol bergerak ke sumbu X, bukan Y)
    # Ini logika sederhana: bandingkan X ujung jempol dengan sendi jempol
    if hand_landmarks[4].x < hand_landmarks[3].x:
        fingers_count += 1
        
    return fingers_count

# ==========================================
# 3. Looping Kamera OpenCV
# ==========================================
video = cv2.VideoCapture(0)

while True:
    success, img = video.read()
    if not success:
        break
        
    img = cv2.flip(img, 1)  # Efek cermin
    h, w, _ = img.shape

    # Konversi BGR (format OpenCV) ke RGB (format MediaPipe)
    rgb_image = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    
    # Buat objek mp.Image
    mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb_image)

    # Lakukan Deteksi
    detection_result = detector.detect(mp_image)
    
    total_fingers = 0

    # Jika ada tangan yang terdeteksi
    if detection_result.hand_landmarks:
        for hand_landmarks in detection_result.hand_landmarks:
            # Hitung jumlah jari untuk tangan ini
            total_fingers += count_fingers(hand_landmarks)
            
            # (Opsional) Gambar titik-titik sendi di tangan
            for landmark in hand_landmarks:
                cx, cy = int(landmark.x * w), int(landmark.y * h)
                cv2.circle(img, (cx, cy), 5, (255, 0, 255), cv2.FILLED)

    # Tampilkan teks jumlah jari
    cv2.putText(img, f'Total Fingers: {total_fingers}', (50, 50),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    # Tampilkan video
    cv2.imshow("Video", img)

    # Tekan 'q' untuk keluar
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    
detector.close()
video.release()
cv2.destroyAllWindows()
