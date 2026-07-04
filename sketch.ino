/*
  PROYEK AKHIR SISTEM MIKROKONTROLER
  Sistem Kontrol Tirai Otomatis Berbasis LDR

  Nama: Beni Mochtar Samiraharja
  NIM: 23552011382
  Kelas: TIF K 23B

  - Sensor    : LDR (Light Dependent Resistor)
  - Aktuator  : Motor Servo (penggerak tirai) + LED (lampu ruangan)
  - Platform  : Blynk (blynk.cloud)
  - Simulator : Wokwi

  Datastream Blynk:
    V0 = Nilai cahaya (ADC 0-4095)
    V1 = Status tirai (1 = terbuka, 0 = tertutup)
    V2 = Mode manual (switch: 0 = otomatis, 1 = manual)
    V3 = Kontrol tirai manual (switch: 1 = buka, 0 = tutup)
*/

// ====== Blynk Configuration ======
#define BLYNK_TEMPLATE_ID "TMPL6UUn0GgYR"
#define BLYNK_TEMPLATE_NAME "Tirai Otomatis LDR"
#define BLYNK_AUTH_TOKEN "GXT7JbOFwgoAEPH1wUkB8vzBN0DriZCe"

// ======================================================

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

const char* ssid     = "Wokwi-GUEST";
const char* password = "";

// Pin
#define LDR_PIN   34
#define SERVO_PIN 13
#define LED_PIN   25

// Ambang batas cahaya (0-4095, makin kecil = makin gelap)
#define BATAS_GELAP 1500

Servo servoTirai;
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

bool tiraiTerbuka = false;
bool modeManual   = false;
bool manualBuka   = false;

void bukaTirai() {
  servoTirai.write(90);
  tiraiTerbuka = true;
}

void tutupTirai() {
  servoTirai.write(0);
  tiraiTerbuka = false;
}

// Dipanggil saat switch Mode Manual (V2) diubah dari Blynk
BLYNK_WRITE(V2) {
  modeManual = (param.asInt() == 1);
  Serial.println(modeManual ? "Mode: MANUAL" : "Mode: OTOMATIS");
}

// Dipanggil saat switch Kontrol Tirai (V3) diubah dari Blynk
BLYNK_WRITE(V3) {
  manualBuka = (param.asInt() == 1);
  Serial.println(manualBuka ? "Perintah manual: BUKA" : "Perintah manual: TUTUP");
}

// Fungsi utama: baca sensor, kontrol tirai, kirim data (tiap 2 detik)
void kontrolTirai() {
  int nilaiCahaya = analogRead(LDR_PIN);

  // Logika kontrol
  if (modeManual) {
    if (manualBuka) bukaTirai();
    else            tutupTirai();
  } else {
    if (nilaiCahaya > BATAS_GELAP) bukaTirai();   // terang -> buka
    else                           tutupTirai();  // gelap  -> tutup
  }

  // Lampu otomatis menyala saat gelap
  digitalWrite(LED_PIN, nilaiCahaya <= BATAS_GELAP ? HIGH : LOW);

  // Kirim ke Blynk
  Blynk.virtualWrite(V0, nilaiCahaya);
  Blynk.virtualWrite(V1, tiraiTerbuka ? 1 : 0);

  // Tampilkan di LCD
  lcd.setCursor(0, 0);
  lcd.print("Cahaya: ");
  lcd.print(nilaiCahaya);
  lcd.print("    ");
  lcd.setCursor(0, 1);
  lcd.print("Tirai: ");
  lcd.print(tiraiTerbuka ? "TERBUKA " : "TERTUTUP");
  lcd.setCursor(15, 1);
  lcd.print(modeManual ? "M" : " ");

  Serial.print("Cahaya: "); Serial.print(nilaiCahaya);
  Serial.print(" | Tirai: "); Serial.print(tiraiTerbuka ? "TERBUKA" : "TERTUTUP");
  Serial.print(" | Mode: "); Serial.println(modeManual ? "MANUAL" : "OTOMATIS");
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  servoTirai.attach(SERVO_PIN);
  tutupTirai();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Tirai Otomatis");
  lcd.setCursor(0, 1);
  lcd.print("Berbasis LDR");
  delay(2000);
  lcd.clear();
  lcd.print("Connecting...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  lcd.clear();
  lcd.print("Blynk Connected!");
  Serial.println("Terhubung ke Blynk!");
  delay(1500);
  lcd.clear();

  timer.setInterval(2000L, kontrolTirai);
}

void loop() {
  Blynk.run();
  timer.run();
}
