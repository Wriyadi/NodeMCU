import cv2
import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision
import numpy as np

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
# 2. Fungsi Deteksi Orientasi Telapak Tangan
# ==========================================
def get_palm_direction(hand_landmarks):
    """
    Menentukan apakah telapak tangan menghadap kamera (FRONT)
    atau membelakangi kamera (BACK) menggunakan cross product vektor.

    Caranya:
    - Ambil 3 titik: pergelangan tangan (0), pangkal telunjuk (5), pangkal kelingking (17)
    - Hitung cross product dari dua vektor yang terbentuk
    - Arah Z dari cross product menentukan orientasi normal telapak

    Return: 'FRONT' jika telapak menghadap kamera, 'BACK' jika membelakangi
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

    # Cross product → vektor normal permukaan telapak
    normal = np.cross(v1, v2)

    # Jika komponen Z positif → telapak menghadap kamera (FRONT)
    # Jika komponen Z negatif → telapak membelakangi kamera (BACK)
    return 'FRONT' if normal[2] > 0 else 'BACK'


# ==========================================
# 3. Fungsi Deteksi Jari Naik ke Atas
# ==========================================
def is_finger_up(tip, pip, dip, wrist, threshold=0.02):
    """
    Mendeteksi apakah jari berdiri tegak ke atas dengan 2 kriteria:
      1. Ujung jari (tip) lebih tinggi dari PIP (lebih kecil Y karena Y terbalik di layar)
      2. Ujung jari lebih tinggi dari pergelangan tangan (wrist) → benar-benar 'ke atas'

    Parameter threshold mencegah deteksi false positive saat jari hampir datar.
    """
    tip_above_pip   = tip.y < pip.y - threshold
    tip_above_wrist = tip.y < wrist.y
    return tip_above_pip and tip_above_wrist


def is_thumb_up(hand_landmarks, handedness, palm_direction):
    """
    Deteksi jempol naik ke atas dengan mempertimbangkan:
    - Handedness (tangan kiri/kanan) karena arah X mirror
    - Orientasi telapak (menghadap/membelakangi kamera)

    Logika utama:
    - Jempol dianggap 'naik ke atas' jika ujungnya (tip) lebih tinggi dari pangkalnya (IP/MCP).
    - Sumbu Y digunakan sebagai referensi utama 'ke atas'.
    - Threshold sumbu X (jempol menjulur ke samping) dipakai sebagai fallback
      saat telapak datar, dengan arah yang disesuaikan handedness + palm_direction.
    """
    tip = hand_landmarks[4]   # Ujung jempol
    ip  = hand_landmarks[3]   # Sendi IP jempol
    mcp = hand_landmarks[2]   # Pangkal jempol (MCP)

    # Kriteria 1: Jempol berdiri ke atas (sumbu Y)
    thumb_pointing_up = tip.y < ip.y - 0.02

    # Kriteria 2: Jempol menjulur ke samping saat telapak datar
    # Arah X bergantung pada handedness dan orientasi telapak
    if handedness == 'Right':
        # Tangan kanan: setelah flip kamera, jempol menjulur ke kanan layar
        if palm_direction == 'FRONT':
            thumb_pointing_side = tip.x > ip.x + 0.02   # Menjulur ke kanan
        else:
            thumb_pointing_side = tip.x < ip.x - 0.02   # Menjulur ke kiri (telapak balik)
    else:
        # Tangan kiri: kebalikan dari tangan kanan
        if palm_direction == 'FRONT':
            thumb_pointing_side = tip.x < ip.x - 0.02   # Menjulur ke kiri
        else:
            thumb_pointing_side = tip.x > ip.x + 0.02   # Menjulur ke kanan (telapak balik)

    # Jempol dihitung jika memenuhi salah satu kriteria
    return thumb_pointing_up or thumb_pointing_side


# ==========================================
# 4. Fungsi Utama Menghitung Jari
# ==========================================
def count_fingers(hand_landmarks, handedness):
    """
    Menghitung total jari yang terangkat ke atas dengan logika:
    - 4 jari biasa: tip harus lebih tinggi dari PIP (atas layar = Y kecil)
    - Jempol: pakai fungsi khusus yang sadar handedness + orientasi
    """
    fingers_count = 0

    palm_direction = get_palm_direction(hand_landmarks)
    wrist          = hand_landmarks[0]

    # Pasangan [tip_id, pip_id, dip_id] untuk 4 jari (telunjuk → kelingking)
    FINGER_JOINTS = [
        (8,  6,  7),   # Telunjuk
        (12, 10, 11),  # Tengah
        (16, 14, 15),  # Manis
        (20, 18, 19),  # Kelingking
    ]

    for tip_id, pip_id, dip_id in FINGER_JOINTS:
        if is_finger_up(
            hand_landmarks[tip_id],
            hand_landmarks[pip_id],
            hand_landmarks[dip_id],
            wrist
        ):
            fingers_count += 1

    # Cek jempol
    if is_thumb_up(hand_landmarks, handedness, palm_direction):
        fingers_count += 1

    return fingers_count, palm_direction


# ==========================================
# 5. Looping Kamera OpenCV
# ==========================================
video = cv2.VideoCapture(0)

# Warna untuk setiap tangan
COLOR_RIGHT = (0, 200, 255)   # Kuning-oranye untuk tangan kanan
COLOR_LEFT  = (255, 100, 0)   # Biru untuk tangan kiri

while True:
    success, img = video.read()
    if not success:
        break

    img = cv2.flip(img, 1)   # Efek cermin
    h, w, _ = img.shape

    # Konversi BGR → RGB
    rgb_image = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    mp_image  = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb_image)

    # Deteksi
    detection_result = detector.detect(mp_image)

    total_fingers = 0
    y_offset      = 50   # Posisi Y awal untuk teks per tangan

    if detection_result.hand_landmarks:
        hands      = detection_result.hand_landmarks
        handedness = detection_result.handedness   # List kategori 'Left'/'Right' per tangan

        for i, hand_landmarks in enumerate(hands):
            # Ambil label tangan ('Left' atau 'Right')
            hand_label = handedness[i][0].display_name   # 'Left' atau 'Right'
            color      = COLOR_RIGHT if hand_label == 'Right' else COLOR_LEFT

            # Hitung jari
            count, palm_dir = count_fingers(hand_landmarks, hand_label)
            total_fingers  += count

            # Gambar titik sendi
            for landmark in hand_landmarks:
                cx, cy = int(landmark.x * w), int(landmark.y * h)
                cv2.circle(img, (cx, cy), 5, color, cv2.FILLED)

            # Teks info per tangan
            info_text = f'{hand_label}: {count} jari | Telapak: {palm_dir}'
            cv2.putText(img, info_text, (50, y_offset),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)
            y_offset += 40

    # Teks total semua tangan
    cv2.putText(img, f'TOTAL: {total_fingers} Jari', (50, y_offset + 10),
                cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 255, 0), 3)

    cv2.imshow("Finger Counter", img)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

detector.close()
video.release()
cv2.destroyAllWindows()
