/*Ini menggunakan ESP32 sebagai kontrol remote RC 27MHZ
  Tujuannya mengubah remote jadi  berbasis wifi yang kemudian di hubungkan ke router
  Perintah dikirimkan ke UDP, cocok untuk demo objek deteksi warna/lingkaran
  Tersedia juga APK androinya , dan python untuk berbagai deteksi object
  by : Nyoman Yudi Kurniawan
  untuk Anak Anak ku Mahasiswa  D4 Teknik Rekayasa Otomotif Unesa 2025 kelas I
  SEMANGAT BELAJAR TERUS !!
*/ 
//ini khusus ESP32
#include <WiFi.h>
#include <WiFiUdp.h>

//Jika pake esp8266 gunakan library ini
//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>

// Konfigurasi WiFi disesuaikan router atau hotspot
const char* ssid = "Gak_Perlu_Ngebut";
const char* password = "klengcinot";

// IP statis, sesuaikan dengan IP network kamu
IPAddress ip(192, 168, 1, 177);
IPAddress gateway(192, 168, 1, 1);     // biasanya IP router
IPAddress subnet(255, 255, 255, 0);

// Port UDP
unsigned int udpPort = 4210;
WiFiUDP udp;

// Pin kendali, sesuaikan kalau pin kamu berbeda
#define MAJU_PIN    15
#define MUNDUR_PIN   2
#define KANAN_PIN    4
#define KIRI_PIN     5

void setup() {
  // Inisialisasi pin sebagai output
  pinMode(MAJU_PIN, OUTPUT);
  pinMode(MUNDUR_PIN, OUTPUT);
  pinMode(KANAN_PIN, OUTPUT);
  pinMode(KIRI_PIN, OUTPUT);

  // Set awal semua pin HIGH (non-aktif karena aktif LOW)
  digitalWrite(MAJU_PIN, HIGH);
  digitalWrite(MUNDUR_PIN, HIGH);
  digitalWrite(KANAN_PIN, HIGH);
  digitalWrite(KIRI_PIN, HIGH);

  // Mulai koneksi WiFi dengan IP statis
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);

  Serial.begin(115200);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nKoneksi WiFi berhasil");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Mulai UDP
  udp.begin(udpPort);
  Serial.printf("Listening on UDP port %d\n", udpPort);
}

void maju()
{

    digitalWrite(MAJU_PIN, LOW );
    digitalWrite(MUNDUR_PIN, HIGH);
    digitalWrite(KANAN_PIN, HIGH);
    digitalWrite(KIRI_PIN, HIGH);

  Serial.println("ini maju ...");
}

void mundur()
{

    digitalWrite(MAJU_PIN, HIGH );
    digitalWrite(MUNDUR_PIN, LOW);
    digitalWrite(KANAN_PIN, HIGH);
    digitalWrite(KIRI_PIN, HIGH);

    Serial.println("ini mundur ...");

}

void kiri()
{

    digitalWrite(MAJU_PIN, LOW);
    digitalWrite(MUNDUR_PIN, HIGH);
    digitalWrite(KANAN_PIN, HIGH);
    digitalWrite(KIRI_PIN, LOW);


    Serial.println("ini ngiri ...");
  
}

void kanan()
{

    digitalWrite(MAJU_PIN, LOW );
    digitalWrite(MUNDUR_PIN, HIGH);
    digitalWrite(KANAN_PIN, LOW);
    digitalWrite(KIRI_PIN, HIGH);

    Serial.println("ini nganan ...");
}

void mandek()
{

    digitalWrite(MAJU_PIN, HIGH);
    digitalWrite(MUNDUR_PIN, HIGH);
    digitalWrite(KANAN_PIN, HIGH);
    digitalWrite(KIRI_PIN, HIGH);

    Serial.println("ini mandek ...");
}

void loop() {

/* ini test saja
    digitalWrite(MAJU_PIN,HIGH );
    digitalWrite(MUNDUR_PIN,LOW );
    digitalWrite(KANAN_PIN, HIGH);
    digitalWrite(KIRI_PIN, HIGH);

   delay(2000);

       digitalWrite(MAJU_PIN, HIGH);
    digitalWrite(MUNDUR_PIN, HIGH);
    digitalWrite(KANAN_PIN, LOW);
    digitalWrite(KIRI_PIN, HIGH);

   delay(2000);

       digitalWrite(MAJU_PIN,HIGH );
    digitalWrite(MUNDUR_PIN, HIGH);
    digitalWrite(KANAN_PIN, HIGH);
    digitalWrite(KIRI_PIN, LOW);

   delay(2000);

       digitalWrite(MAJU_PIN, HIGH);
    digitalWrite(MUNDUR_PIN, HIGH);
    digitalWrite(KANAN_PIN, HIGH);
    digitalWrite(KIRI_PIN, HIGH);

   delay(2000);

  */
  char packetBuffer[255]; // Buffer untuk menerima data UDP

  // Cek apakah ada paket UDP masuk
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = '\0'; // Null-terminate string
    }

    Serial.printf("Pesan diterima: %s\n", packetBuffer);

    // Proses perintah
    if (strcmp(packetBuffer, "1") == 0) {
      maju();
      delay(600);
      mundur();
      delay(200);
      mandek();
      
    } else if (strcmp(packetBuffer, "2") == 0) {
      mundur();
      delay(600);
      maju();
      delay(200);
      mandek();
    } else if (strcmp(packetBuffer, "3") == 0) {
      kiri();
      delay(700);
      mundur();
      delay(150);   
      mandek();
    } else if (strcmp(packetBuffer, "4") == 0) {
      kanan();
      delay(700);
      mundur();
      delay(150);   
      mandek();
    } else if (strcmp(packetBuffer, "0") == 0) {
      // Semua tetap HIGH → berhenti
      mandek();
      Serial.println("Perintah BERHENTI diterima");
    }

    
  }
  
}




