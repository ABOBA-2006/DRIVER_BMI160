import fcntl
import struct
import sys
import select
from luma.core.interface.serial import i2c
from luma.oled.device import ssd1306
from PIL import Image, ImageDraw
import math
import time
import os

BMI160_IOC_MAGIC = ord('B')
IOCTL_GET_ACCEL_X = 0x80024201  # _IOR(BMI160_IOC_MAGIC, 1, s16)
IOCTL_GET_ACCEL_Y = 0x80024202  # _IOR(BMI160_IOC_MAGIC, 2, s16)
IOCTL_GET_ACCEL_Z = 0x80024203  # _IOR(BMI160_IOC_MAGIC, 3, s16)
IOCTL_CALIBRATE_SENSOR = 0x80024204  # _IOR(BMI160_IOC_MAGIC, 4, s16)

serial = i2c(port=0, address=0x3C)   # 0x3C is the address of oled display
device = ssd1306(serial, width=128, height=32)

def read_accel(fd, ioctl_cmd):
    buf = bytearray(2)  # s16 = 2 bytes
    fcntl.ioctl(fd, ioctl_cmd, buf)
    val, = struct.unpack("h", buf)
    return val

def send_calibrate(fd):
    buf = bytearray(2) # s16 = 2 bytes
    fcntl.ioctl(fd, IOCTL_CALIBRATE_SENSOR, buf)
    val, = struct.unpack("h", buf)
    return val

def get_pitch_roll():
    fd = open("/dev/bmi160_device", "rb", buffering=0)
    ax = read_accel(fd, IOCTL_GET_ACCEL_X)
    ay = read_accel(fd, IOCTL_GET_ACCEL_Y)
    az = read_accel(fd, IOCTL_GET_ACCEL_Z)

    # Convert raw values to g
    ax_g = ax / 16384.0
    ay_g = ay / 16384.0
    az_g = az / 16384.0

    # Calculate pitch and roll
    pitch = math.atan2(-ax_g, math.sqrt(ay_g**2 + az_g**2)) * 180.0 / math.pi

    return pitch

while True:
    if select.select([sys.stdin], [], [], 0)[0]:
        user_input = (sys.stdin.readline().strip()).lower()

        if user_input == "calibrate":

            img = Image.new("1", (128, 32), "black")
            draw = ImageDraw.Draw(img)
            draw.text((10, 5), "CALIBRATING", fill="white")
            device.display(img)

            fd = open("/dev/bmi160_device", "rb", buffering=0)
            send_calibrate(fd)
        if user_input == "exit":
            os._exit(0)

    try:
        pitch = get_pitch_roll()
    except Exception as e:
        pitch = -10.0, -10.0

    img = Image.new("1", (128, 32), "black")
    draw = ImageDraw.Draw(img)
    draw.text((10, 5), f"Pitch: {pitch:.2f}", fill="white")
    device.display(img)

    time.sleep(0.1)
