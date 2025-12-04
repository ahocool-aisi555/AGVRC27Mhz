import cv2
import socket
import time

# === Konfigurasi UDP ===
UDP_IP = "192.168.1.177"
UDP_PORT = 4210
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_udp(data):
    sock.sendto(data.encode(), (UDP_IP, UDP_PORT))
    print(f"[UDP] Terkirim: {data}")

# === Muat Haar Cascade untuk wajah (bawaan OpenCV) ===
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

# === Kamera ===
# === Kamera ===
rtsp_url = "rtsp://admin:FJUHGV@192.168.1.178:554/Streaming/Channels/102"
cap = cv2.VideoCapture(rtsp_url)

# === Status ===
last_send_time = time.time()
last_detection_time = time.time()
send_data = "0"
action_text = "Tidak Ada Wajah"
has_sent_stop = False

def draw_overlay(frame, text):
    cv2.putText(frame, f"Aksi: {text}", (10, 60), 
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)

while True:
    ret, frame = cap.read()
    if not ret:
        print("Gagal membaca frame kamera.")
        break

    frame = cv2.flip(frame, 1)  # flip horizontal agar gerakan lebih intuitif
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # --- Deteksi wajah ---
    faces = face_cascade.detectMultiScale(
        gray,
        scaleFactor=1.0,
        minNeighbors=2,
        minSize=(70, 70)  # abaikan wajah terlalu kecil
    )

    detected = False
    cX = -1

    if len(faces) > 0:
        # Ambil wajah pertama (atau terbesar)
        x, y, w, h = faces[0]
        cX = x + w // 2
        cY = y + h // 2

        # Gambar kotak dan titik tengah
        cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
        cv2.circle(frame, (cX, cY), 5, (255, 0, 0), -1)

        detected = True
        last_detection_time = time.time()
        has_sent_stop = False

    # --- Tentukan aksi berdasarkan posisi wajah ---
    if detected:
        width = frame.shape[1]
        zona_kiri = width // 3
        zona_tengah = 2 * width // 3

        if cX < zona_kiri:
            action_text = "Kiri"
            send_data = "4"
        elif cX < zona_tengah:
            action_text = "Maju"
            send_data = "1"
        else:
            action_text = "Kanan"
            send_data = "3"
    else:
        elapsed = time.time() - last_detection_time
        if elapsed >= 10 and not has_sent_stop:
            send_data = "2"
            action_text = "Mundur"
            send_udp(send_data)
            has_sent_stop = True
        else:
            action_text = "Tidak Ada Wajah"
            send_data = "0"

    # --- Kirim data setiap 2 detik ---
    if time.time() - last_send_time >= 2:
        send_udp(send_data)
        last_send_time = time.time()

    # --- Tampilkan hasil ---
    draw_overlay(frame, action_text)
    cv2.imshow('Remote Control via Wajah', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# --- Cleanup ---
cap.release()
cv2.destroyAllWindows()
sock.close()
print("Program dihentikan.")