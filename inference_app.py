import cv2
import numpy as np
import tensorflow as tf
import tkinter as tk
from tkinter import filedialog
import sys

# ==========================================
# 1. Konfigurasi Awal
# ==========================================
print("Memuat model, harap tunggu...")
# Pastikan 'best_model.keras' ada di folder yang sama
model = tf.keras.models.load_model('best_model.keras')

class_names = ['✋ Kertas', '✊ Batu', '✌️ Gunting']
INPUT_SIZE = (150, 150) # Sesuaikan dengan target_size saat training

# ==========================================
# 2. Fungsi Prediksi Gambar
# ==========================================
def predict_image(img_bgr, use_roi=False):
    """
    Fungsi untuk memproses gambar dan memprediksi hasilnya.
    Jika use_roi=True, kita hanya memotong bagian tengah gambar.
    """
    img_copy = img_bgr.copy()
    h, w, _ = img_copy.shape
    
    # Menentukan area yang akan dianalisis (Full image atau Kotak ROI)
    if use_roi:
        box_size = 300
        start_x = w // 2 - box_size // 2
        start_y = h // 2 - box_size // 2
        end_x = start_x + box_size
        end_y = start_y + box_size
        
        cv2.rectangle(img_copy, (start_x, start_y), (end_x, end_y), (255, 0, 0), 2)
        roi = img_copy[start_y:end_y, start_x:end_x]
        text_x, text_y = start_x, start_y - 10
    else:
        roi = img_copy
        text_x, text_y = 10, 40

    # Proses prediksi jika ROI valid
    if roi.size != 0:
        roi_rgb = cv2.cvtColor(roi, cv2.COLOR_BGR2RGB)
        roi_resized = cv2.resize(roi_rgb, INPUT_SIZE)
        img_array = roi_resized / 255.0
        img_array = np.expand_dims(img_array, axis=0)
        
        predictions = model.predict(img_array, verbose=0)
        class_idx = np.argmax(predictions[0])
        confidence = np.max(predictions[0])
        
        if confidence > 0.6:
            text = f"{class_names[class_idx]} ({confidence*100:.1f}%)"
        else:
            text = "Tidak Yakin / Bukan Tangan"
            
        cv2.putText(img_copy, text, (text_x, text_y),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 3)
                    
    return img_copy

# ==========================================
# 3. Pilihan Mode (Fungsi-fungsi)
# ==========================================

def mode_realtime():
    print("\n[INFO] Membuka kamera... Tekan 'q' untuk kembali ke menu utama.")
    video = cv2.VideoCapture(0)
    while True:
        success, img = video.read()
        if not success: break
        
        img = cv2.flip(img, 1)
        result_img = predict_image(img, use_roi=True)
        cv2.imshow("Mode Real-time (Tekan 'Q' untuk keluar)", result_img)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    video.release()
    cv2.destroyAllWindows()

def mode_ambil_foto():
    print("\n[INFO] Membuka kamera...")
    print("Petunjuk: Posisikan tangan Anda di kotak biru.")
    print("Tekan 'SPACE' untuk memotret, atau 'q' untuk batal.")
    video = cv2.VideoCapture(0)
    
    while True:
        success, img = video.read()
        if not success: break
        
        img = cv2.flip(img, 1)
        
        # Gambar kotak panduan saja
        h, w, _ = img.shape
        box_size = 300
        start_x, start_y = w // 2 - box_size // 2, h // 2 - box_size // 2
        cv2.rectangle(img, (start_x, start_y), (start_x + box_size, start_y + box_size), (0, 255, 255), 2)
        cv2.putText(img, "Tekan SPACE untuk Jepret", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0,255,255), 2)
        
        cv2.imshow("Mode Ambil Foto", img)
        key = cv2.waitKey(1) & 0xFF
        
        if key == 32: # Tombol Space ditekan
            print("[INFO] Foto diambil! Memproses...")
            # Prediksi menggunakan hasil foto asli
            result_img = predict_image(img, use_roi=True)
            cv2.imshow("Hasil Prediksi Foto (Tekan sembarang tombol untuk tutup)", result_img)
            cv2.waitKey(0) # Tunggu sampai user menekan tombol apa saja
            break
        elif key == ord('q'):
            break
            
    video.release()
    cv2.destroyAllWindows()

def mode_upload_foto():
    print("\n[INFO] Silakan pilih foto dari komputer Anda...")
    # Sembunyikan window utama tkinter
    root = tk.Tk()
    root.withdraw() 
    
    # Buka dialog pilih file
    file_path = filedialog.askopenfilename(
        title="Pilih Gambar Tangan",
        filetypes=[("Image Files", "*.jpg *.jpeg *.png")]
    )
    
    if file_path:
        print(f"[INFO] Memproses gambar: {file_path}")
        img = cv2.imread(file_path)
        
        if img is not None:
            # Prediksi full image (tanpa kotak ROI)
            result_img = predict_image(img, use_roi=False)
            
            # Resize jendela agar gambar yang terlalu besar muat di layar
            h, w = result_img.shape[:2]
            if h > 800 or w > 800:
                result_img = cv2.resize(result_img, (800, int(800 * (h/w))))
                
            cv2.imshow("Hasil Upload Foto (Tekan sembarang tombol untuk tutup)", result_img)
            cv2.waitKey(0)
            cv2.destroyAllWindows()
        else:
            print("[ERROR] Gagal membaca file gambar.")
    else:
        print("[INFO] Upload dibatalkan.")

# ==========================================
# 4. Menu Utama
# ==========================================
while True:
    print("\n" + "="*40)
    print("🤖 MENU DETEKSI GUNTING BATU KERTAS")
    print("="*40)
    print("1. Deteksi Real-time dari Kamera")
    print("2. Ambil Foto (Jepret dari Kamera)")
    print("3. Upload Foto dari Komputer")
    print("4. Keluar Program")
    
    pilihan = input("Masukkan pilihan Anda (1/2/3/4): ")
    
    if pilihan == '1':
        mode_realtime()
    elif pilihan == '2':
        mode_ambil_foto()
    elif pilihan == '3':
        mode_upload_foto()
    elif pilihan == '4':
        print("Terima kasih! Program ditutup.")
        sys.exit()
    else:
        print("Pilihan tidak valid, silakan coba lagi.")