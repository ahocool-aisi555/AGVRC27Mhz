import cv2
import socket
import time
import numpy as np

# === Konfigurasi UDP ===
UDP_IP = "192.168.1.177"
UDP_PORT = 4210
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_udp(data):
    sock.sendto(data.encode(), (UDP_IP, UDP_PORT))
    print(f"[UDP] Terkirim: {data}")

# === Kamera ===
rtsp_url = "rtsp://admin:FJUHGV@192.168.1.178:554/Streaming/Channels/102"
cap = cv2.VideoCapture(rtsp_url)

# === Parameter ===
last_send_time = time.time()
last_detection_time = time.time()
send_data = "0"
action_text = "Tidak Ada Deteksi"
has_sent_stop = False

def draw_overlay(frame, text):
    cv2.putText(frame, f"Aksi: {text}", (10, 60), 
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)

while True:
    ret, frame = cap.read()
    if not ret:
        print("Tidak dapat menerima frame. Keluar...")
        break

    current_time = time.time()
    detected = False
    cX = -1  # posisi tengah objek

    # --- 1. Konversi ke HSV ---
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # --- 2. Buat mask untuk warna MERAH (dua rentang karena HSV melingkar) ---
    lower_red1 = np.array([0, 100, 100])
    upper_red1 = np.array([10, 255, 255])
    lower_red2 = np.array([160, 100, 100])
    upper_red2 = np.array([180, 255, 255])

    mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
    mask2 = cv2.inRange(hsv, lower_red2, upper_red2)
    mask = mask1 + mask2

    # --- 3. Operasi morfologi untuk membersihkan noise ---
    kernel = np.ones((5,5), np.uint8)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)

    # --- 4. Cari kontur ---
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # --- 5. Cari kontur terbesar yang mirip lingkaran ---
    if contours:
        # Urutkan dari terbesar ke terkecil
        contours = sorted(contours, key=cv2.contourArea, reverse=True)
        for cnt in contours:
            area = cv2.contourArea(cnt)
            if area < 500:  # abaikan objek terlalu kecil
                continue

            # Hitung rasio lingkaran: (4 * pi * area) / perimeter^2
            perimeter = cv2.arcLength(cnt, True)
            if perimeter == 0:
                continue
            circularity = 4 * np.pi * area / (perimeter * perimeter)

            # Jika cukup mirip lingkaran (circularity ~1.0)
            if circularity > 0.7:
                # Dapatkan bounding circle
                (x, y), radius = cv2.minEnclosingCircle(cnt)
                cX, cY = int(x), int(y)
                radius = int(radius)

                # Gambar lingkaran dan titik tengah
                cv2.circle(frame, (cX, cY), radius, (0, 255, 0), 2)
                cv2.circle(frame, (cX, cY), 5, (0, 0, 255), -1)

                detected = True
                last_detection_time = current_time
                has_sent_stop = False
                break

    # --- 6. Tentukan aksi berdasarkan posisi cX ---
    if detected:
        width = frame.shape[1]
        zona_kiri = width // 3
        zona_tengah = 2 * width // 3

        if cX < zona_kiri:
            action_text = "Kiri"
            send_data = "3"
        elif cX < zona_tengah:
            action_text = "Maju"
            send_data = "1"
        else:
            action_text = "Kanan"
            send_data = "4"

    else:
        # Tidak terdeteksi
        elapsed = current_time - last_detection_time
        if elapsed >= 10 and not has_sent_stop:
            send_data = "2"
            action_text = "Mundur"
            send_udp(send_data)
            print("[INFO] Mengirim '2' (mundur)")
            has_sent_stop = True
        else:
            action_text = "Tidak Ada Deteksi"
            send_data = "0"

    # --- 7. Kirim data setiap 2 detik ---
    if current_time - last_send_time >= 1:
        send_udp(send_data)
        last_send_time = current_time

    # --- 8. Tampilkan zona dan teks ---
    height = frame.shape[0]
    cv2.line(frame, (frame.shape[1]//3, 0), (frame.shape[1]//3, height), (255, 0, 0), 1)
    cv2.line(frame, (2*frame.shape[1]//3, 0), (2*frame.shape[1]//3, height), (255, 0, 0), 1)
    draw_overlay(frame, action_text)
    cv2.imshow('Deteksi Lingkaran Merah', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# --- Cleanup ---
cap.release()
cv2.destroyAllWindows()
sock.close()