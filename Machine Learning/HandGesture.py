import cv2
import numpy as np
import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision
import serial
import time

# ==========================================
# 1. Konfigurasi PySerial (Komunikasi Arduino)
# ==========================================
SERIAL_PORT = 'COM4'
BAUD_RATE = 115200

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
    time.sleep(2.0)  # Menunggu Arduino reset setelah koneksi terbuka
    print(f"Koneksi Serial berhasil dibuka di port {SERIAL_PORT}")
except Exception as e:
    print(f"Gagal membuka port {SERIAL_PORT}: {e}")
    ser = None

# ==========================================
# 2. Konfigurasi MediaPipe Tasks API
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
# 3. Fungsi Deteksi Orientasi Telapak Tangan
# ==========================================
def get_palm_direction(hand_landmarks):
    """
    Menentukan apakah telapak tangan menghadap kamera (FRONT)
    atau membelakangi kamera (BACK) menggunakan cross product vektor.
    """
    wrist      = hand_landmarks[0]   # Pergelangan tangan
    index_mcp  = hand_landmarks[5]   # Pangkal jari telunjuk
    pinky_mcp  = hand_landmarks[17]  # Pangkal jari kelingking

    # Vektor dari pergelangan ke pangkal telunjuk
    v1 = np.array([
        index_mcp.x - wrist.x,
        index_mcp.y - wrist.y,
        index_mcp.z - wrist.z
    ])
    # Vektor dari pergelangan ke pangkal kelingking
    v2 = np.array([
        pinky_mcp.x - wrist.x,
        pinky_mcp.y - wrist.y,
        pinky_mcp.z - wrist.z
    ])

    # Cross product -> vektor normal permukaan telapak
    normal = np.cross(v1, v2)

    # Jika komponen Z positif -> telapak menghadap kamera (FRONT)
    return 'FRONT' if normal[2] > 0 else 'BACK'

# ==========================================
# 4. Fungsi Deteksi Jari Naik ke Atas
# ==========================================
def is_finger_up(tip, pip, dip, wrist, threshold=0.02):
    """
    Mendeteksi apakah jari berdiri tegak ke atas.
    """
    tip_above_pip   = tip.y < pip.y - threshold
    tip_above_wrist = tip.y < wrist.y
    return tip_above_pip and tip_above_wrist

def is_thumb_up(hand_landmarks, handedness, palm_direction):
    """
    Deteksi jempol naik ke atas dengan mempertimbangkan arah X mirror dan orientasi.
    """
    tip = hand_landmarks[4]   # Ujung jempol
    ip  = hand_landmarks[3]   # Sendi IP jempol

    # Kriteria 1: Jempol berdiri ke atas (sumbu Y)
    thumb_pointing_up = tip.y < ip.y - 0.02

    # Kriteria 2: Jempol menjulur ke samping saat telapak datar
    if handedness == 'Right':
        if palm_direction == 'FRONT':
            thumb_pointing_side = tip.x > ip.x + 0.02   # Menjulur ke kanan
        else:
            thumb_pointing_side = tip.x < ip.x - 0.02   # Menjulur ke kiri
    else:
        if palm_direction == 'FRONT':
            thumb_pointing_side = tip.x < ip.x - 0.02   # Menjulur ke kiri
        else:
            thumb_pointing_side = tip.x > ip.x + 0.02   # Menjulur ke kanan

    return thumb_pointing_up or thumb_pointing_side

# ==========================================
# 5. Fungsi Utama Menghitung Jari
# ==========================================
def count_fingers(hand_landmarks, handedness):
    fingers_count = 0
    palm_direction = get_palm_direction(hand_landmarks)
    wrist          = hand_landmarks[0]

    # Pasangan [tip_id, pip_id, dip_id] untuk 4 jari
    FINGER_JOINTS = [
        (8,  6,  7),  # Telunjuk
        (12, 10, 11), # Tengah
        (16, 14, 15), # Manis
        (20, 18, 19), # Kelingking
    ]

    for tip_id, pip_id, dip_id in FINGER_JOINTS:
        if is_finger_up(hand_landmarks[tip_id], hand_landmarks[pip_id], hand_landmarks[dip_id], wrist):
            fingers_count += 1

    if is_thumb_up(hand_landmarks, handedness, palm_direction):
        fingers_count += 1

    return fingers_count, palm_direction

# ==========================================
# 6. Looping Kamera OpenCV & Pemrosesan
# ==========================================
video = cv2.VideoCapture(0)

COLOR_RIGHT = (0, 200, 255)  # Kuning-oranye untuk tangan kanan
COLOR_LEFT  = (255, 100, 0)  # Biru untuk tangan kiri

prev_fingers = -1  # Menyimpan status jumlah jari sebelumnya

while True:
    success, img = video.read()
    if not success:
        break

    img = cv2.flip(img, 1)  # Efek cermin
    h, w, _ = img.shape

    # Konversi BGR -> RGB untuk MediaPipe
    rgb_image = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    mp_image  = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb_image)

    # Proses deteksi
    detection_result = detector.detect(mp_image)

    total_fingers = 0
    y_offset      = 50  # Posisi Y awal teks HUD

    if detection_result.hand_landmarks:
        hands      = detection_result.hand_landmarks
        handedness = detection_result.handedness

        for i, hand_landmarks in enumerate(hands):
            hand_label = handedness[i][0].display_name
            color      = COLOR_RIGHT if hand_label == 'Right' else COLOR_LEFT

            # Hitung jari pada tangan saat ini
            count, palm_dir = count_fingers(hand_landmarks, hand_label)
            total_fingers  += count

            # Gambar titik koordinat landmark tangan
            for landmark in hand_landmarks:
                cx, cy = int(landmark.x * w), int(landmark.y * h)
                cv2.circle(img, (cx, cy), 5, color, cv2.FILLED)

            # Tampilkan info per tangan di layar
            info_text = f'{hand_label}: {count} jari | Telapak: {palm_dir}'
            cv2.putText(img, info_text, (50, y_offset),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)
            y_offset += 40

    # ==========================================
    # 7. Pengiriman Data via PySerial (Kunci Utama)
    # ==========================================
    if ser and ser.is_open:
        # Hanya kirim jika total jari berubah agar tidak memberatkan Arduino
        if total_fingers != prev_fingers:
            try:
                ser.write(bytearray([total_fingers]))
                print(f"--> Serial Terkirim: {total_fingers}")
                prev_fingers = total_fingers
            except Exception as e:
                print(f"Gagal mengirim data serial: {e}")

    # Menggambar kotak HUD Total Jari ala kodingan kedua Anda
    cv2.rectangle(img, (20, y_offset), (320, y_offset + 80), (0, 255, 0), cv2.FILLED)
    cv2.putText(img, f"{total_fingers} LED", (45, y_offset + 55), 
                cv2.FONT_HERSHEY_SIMPLEX, 1.2, (255, 0, 0), 3)

    cv2.imshow("Finger Counter to Serial", img)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Cleanup
detector.close()
video.release()
if ser and ser.is_open:
    ser.close()
cv2.destroyAllWindows()
