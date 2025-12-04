#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Mode Access Point (tanpa password)
const char* ssid = "MyAGV";  // Nama hotspot
// Tidak ada password → AP terbuka

// Pin kendali (aktif LOW)
#define MAJU_PIN    D2
#define MUNDUR_PIN  D1
#define KANAN_PIN   D4
#define KIRI_PIN    D3

ESP8266WebServer server(80);

String urutanInput = "";
volatile bool jalankan = false;
volatile bool berhenti = false;

void setup() {
  pinMode(MAJU_PIN, OUTPUT);
  pinMode(MUNDUR_PIN, OUTPUT);
  pinMode(KANAN_PIN, OUTPUT);
  pinMode(KIRI_PIN, OUTPUT);

  stop(); // Matikan semua

  Serial.begin(115200);

  // Jalankan sebagai AP terbuka (tanpa password)
  WiFi.softAP(ssid);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Hotspot aktif: ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(IP);

  // Routing
  server.on("/", handleRoot);
  server.on("/kirim", HTTP_POST, handleKirim);
  server.on("/jalankan", HTTP_GET, handleJalankan);
  server.on("/berhenti", HTTP_GET, handleBerhenti);

  server.begin();
  Serial.println("Web server siap.");
}

void loop() {
  server.handleClient();

  if (jalankan && !berhenti) {
    jalankanUrutan(urutanInput);
    jalankan = false;
  }

  if (berhenti) {
    stop();
    berhenti = false;
  }
}

// --- Fungsi Gerakan ---
void maju() {
  digitalWrite(MAJU_PIN, LOW);
  digitalWrite(MUNDUR_PIN, HIGH);
  digitalWrite(KANAN_PIN, HIGH);
  digitalWrite(KIRI_PIN, HIGH);
  Serial.println("Maju");
}

void mundur() {
  digitalWrite(MAJU_PIN, HIGH);
  digitalWrite(MUNDUR_PIN, LOW);
  digitalWrite(KANAN_PIN, HIGH);
  digitalWrite(KIRI_PIN, HIGH);
  Serial.println("Mundur");
}

void kiri() {
  digitalWrite(MAJU_PIN, LOW);
  digitalWrite(MUNDUR_PIN, HIGH);
  digitalWrite(KANAN_PIN, HIGH);
  digitalWrite(KIRI_PIN, LOW);
  Serial.println("Kiri");
}

void kanan() {
  digitalWrite(MAJU_PIN, LOW);
  digitalWrite(MUNDUR_PIN, HIGH);
  digitalWrite(KANAN_PIN, LOW);
  digitalWrite(KIRI_PIN, HIGH);
  Serial.println("Kanan");
}

void stop() {
  digitalWrite(MAJU_PIN, HIGH);
  digitalWrite(MUNDUR_PIN, HIGH);
  digitalWrite(KANAN_PIN, HIGH);
  digitalWrite(KIRI_PIN, HIGH);
  Serial.println("Berhenti");
}

// --- Eksekusi Urutan ---
void jalankanUrutan(String urutan) {
  if (urutan == "") return;

  Serial.println("Menjalankan urutan: " + urutan);
  int start = 0;
  int end = urutan.indexOf(',');

  while (end != -1) {
    String cmd = urutan.substring(start, end);
    eksekusiPerintah(cmd.toInt());
    delay(200);
    start = end + 1;
    end = urutan.indexOf(',', start);
  }

  // Perintah terakhir
  String lastCmd = urutan.substring(start);
  if (lastCmd.length() > 0) {
    eksekusiPerintah(lastCmd.toInt());
    delay(200);
  }

  stop();
  Serial.println("Selesai.");
}

void eksekusiPerintah(int cmd) {
  if (berhenti) return;
  switch (cmd) {
    case 0: stop(); break;
    case 1: maju(); break;
    case 2: mundur(); break;
    case 3: kiri(); break;
    case 4: kanan(); break;
    default: stop(); break;
  }
}

// --- Web Handlers ---
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>AGV Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin: 0;
      padding: 20px;
      background-color: #f5f5f5;
    }
    h2 {
      color: #2c3e50;
    }
    textarea {
      width: 95%;
      max-width: 600px;
      padding: 12px;
      font-size: 16px;
      font-family: monospace;
      border: 1px solid #ccc;
      border-radius: 8px;
      resize: vertical; /* Biarkan user bisa tarik ke bawah */
    }
    button {
      padding: 12px 24px;
      font-size: 18px;
      margin: 10px 5px;
      border: none;
      border-radius: 6px;
      cursor: pointer;
    }
    #jalankanBtn { background-color: #2ecc71; color: white; }
    #berhentiBtn { background-color: #e74c3c; color: white; }
    .info {
      margin-top: 20px;
      padding: 10px;
      background-color: #ecf0f1;
      border-radius: 6px;
      display: inline-block;
      min-width: 200px;
    }
    .legend {
      margin-top: 15px;
      font-size: 14px;
      color: #7f8c8d;
      line-height: 1.5;
    }
  </style>
</head>
<body>
  <h1>AGV Sequence Controller</h1>
  <h2>D4 TRO UNESA 2025i</h2>
  <h2>www.aisi555.com</h2>

  <textarea id="urutan" rows="6" placeholder="Masukkan urutan perintah, pisahkan dengan koma.
Contoh: 1,3,0,4,2,1

0 = Stop
1 = Maju
2 = Mundur
3 = Kiri
4 = Kanan"></textarea>

  <br>
  <button id="jalankanBtn" onclick="kirim()">Jalankan</button>
  <button id="berhentiBtn" onclick="berhenti()">Berhenti</button>

  <div class="info" id="status">Status: Siap</div>

  <div class="legend">
    <p>Pastikan AGV dalam area aman sebelum menjalankan!</p>
  </div>

  <script>
    function kirim() {
      const seq = document.getElementById('urutan').value.trim();
      if (seq === '') {
        alert('Masukkan urutan perintah terlebih dahulu!');
        return;
      }
      fetch('/kirim', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'urutan=' + encodeURIComponent(seq)
      }).then(() => {
        document.getElementById('status').innerText = 'Status: Menjalankan urutan...';
        fetch('/jalankan');
      }).catch(err => {
        console.error(err);
        alert('Gagal mengirim perintah.');
      });
    }

    function berhenti() {
      fetch('/berhenti').then(() => {
        document.getElementById('status').innerText = 'Status: Berhenti darurat';
      }).catch(err => {
        console.error(err);
      });
    }
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleKirim() {
  if (server.hasArg("urutan")) {
    urutanInput = server.arg("urutan");
    // Hapus spasi dan newline berlebih
    urutanInput.replace(" ", "");
    urutanInput.replace("\r", "");
    urutanInput.replace("\n", "");

    // Validasi: hanya 0-4 dan koma
    bool valid = true;
    for (char c : urutanInput) {
      if (!(c == ',' || (c >= '0' && c <= '4'))) {
        valid = false;
        break;
      }
    }
    if (!valid) urutanInput = "";

    Serial.println("Urutan diterima: [" + urutanInput + "]");
  }
  server.send(200, "text/plain", "OK");
}

void handleJalankan() {
  jalankan = true;
  server.send(200, "text/plain", "Jalankan");
}

void handleBerhenti() {
  berhenti = true;
  server.send(200, "text/plain", "Berhenti");
}
