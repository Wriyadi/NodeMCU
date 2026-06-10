// =========================================================================
// CONFIGURATION: Atur tipe Relay Anda di sini
// =========================================================================
#define RELAY_ON  LOW   // Ganti ke HIGH jika modul relay Anda tipe Active-High
#define RELAY_OFF HIGH  // Ganti ke LOW jika modul relay Anda tipe Active-High

// =========================================================================
// AUTO PIN SELECTION: Otomatis mendeteksi tipe board saat di-upload
// =========================================================================
#if defined(ESP32)
  // Konfigurasi PIN untuk ESP32 (Gunakan GPIO yang aman untuk Output)
  const int relayPins[5] = {18, 19, 21, 22, 23}; 
  #define BOARD_TYPE "ESP32"

#elif defined(ESP8266)
  // Konfigurasi PIN untuk ESP8266 NodeMCU / Wemos D1 Mini
  // Pin: 5 (D1), 4 (D2), 14 (D5), 12 (D6), 13 (D7)
  // (Dihindari memakai GPIO 0 dan 2 untuk relay karena bisa mengganggu proses booting)
  const int relayPins[5] = {5, 4, 14, 12, 13}; 
  #define BOARD_TYPE "ESP8266"

#else
  // Fallback pin jika Anda menggunakan board lain (seperti Arduino Uno)
  const int relayPins[5] = {16, 5, 4, 0, 2};
  #define BOARD_TYPE "UNKNOWN"
#endif

int inByte = 0; 

void setup() {
  // Membuka komunikasi serial sesuai dengan bitrate di Python (115200)
  Serial.begin(115200);
  while (!Serial) {
    ; // Menunggu port serial siap (khusus untuk komunikasi USB native)
  }

  // Inisialisasi semua pin relay menggunakan perulangan (Array)
  for (int i = 0; i < 5; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], RELAY_OFF); // Pastikan relay mati saat pertama kali dinyalakan
  }

  establishContact();  // Menunggu sinyal pertama kali dari Python
}

void loop() {
  // Cek apakah ada data serial yang masuk dari Python
  if (Serial.available() > 0) {
    inByte = Serial.read(); // Membaca data byte (0 sampai 5)

    // Validasi data untuk memastikan rentang angka yang diterima adalah benar (0-5)
    if (inByte >= 0 && inByte <= 5) {
      
      // Mengontrol ke-5 relay secara sekuensial dan efisien
      for (int i = 0; i < 5; i++) {
        if (i < inByte) {
          digitalWrite(relayPins[i], RELAY_ON);  // Nyalakan relay jika indeks di bawah jumlah jari
        } else {
          digitalWrite(relayPins[i], RELAY_OFF); // Matikan sisa relay lainnya
        }
      }
      
    }
  }
}

// Fungsi pembuka komunikasi awal dengan program Python
void establishContact() {
  while (Serial.available() <= 0) {
    delay(100);
  }
}
