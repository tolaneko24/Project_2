import serial
import re
import time
from collections import deque

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation


# ================== CẤU HÌNH ==================
# Windows ví dụ: "COM3", "COM4"
# Linux ví dụ: "/dev/ttyUSB0", "/dev/ttyACM0"
# macOS ví dụ: "/dev/tty.usbserial-xxxx"
SERIAL_PORT = "COM5"

BAUD_RATE = 9600

# Số điểm hiển thị gần nhất trên đồ thị
MAX_POINTS = 100

# Regex để bắt dòng dạng:
# ADC:121 Temp:30.2 C
pattern = re.compile(r"ADC:(\d+)\s+Temp:(\d+)\.(\d+)\s*C")


# ================== MỞ SERIAL ==================
ser = serial.Serial(
    port=SERIAL_PORT,
    baudrate=BAUD_RATE,
    timeout=1
)

time.sleep(2)  # chờ serial ổn định


# ================== BUFFER DỮ LIỆU ==================
time_data = deque(maxlen=MAX_POINTS)
temp_data = deque(maxlen=MAX_POINTS)
adc_data = deque(maxlen=MAX_POINTS)

start_time = time.time()


# ================== SETUP BIỂU ĐỒ ==================
plt.figure(figsize=(10, 5))
line_temp, = plt.plot([], [], label="Temperature (°C)")
plt.xlabel("Time (s)")
plt.ylabel("Temperature (°C)")
plt.title("LM35 Temperature Runtime Plot")
plt.grid(True)
plt.legend()


def update(frame):
    """
    Hàm này được gọi lặp lại để đọc serial và cập nhật đồ thị.
    """

    while ser.in_waiting > 0:
        raw_line = ser.readline().decode("utf-8", errors="ignore").strip()

        if not raw_line:
            continue

        print(raw_line)

        match = pattern.search(raw_line)

        if match:
            adc_val = int(match.group(1))
            temp_int = int(match.group(2))
            temp_dec = int(match.group(3))

            temp_c = temp_int + temp_dec / 10.0
            elapsed_time = time.time() - start_time

            time_data.append(elapsed_time)
            temp_data.append(temp_c)
            adc_data.append(adc_val)

    if len(time_data) > 0:
        line_temp.set_data(time_data, temp_data)

        plt.xlim(max(0, time_data[0]), max(10, time_data[-1]))

        ymin = min(temp_data) - 2
        ymax = max(temp_data) + 2

        if ymin == ymax:
            ymin -= 1
            ymax += 1

        plt.ylim(ymin, ymax)

    return line_temp,


ani = FuncAnimation(
    plt.gcf(),
    update,
    interval=200,
    cache_frame_data=False
)

try:
    plt.show()
finally:
    ser.close()
    print("Serial closed.")