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
import RPi.GPIO as GPIO


BMI160_IOC_MAGIC = ord('B')
IOCTL_GET_ACCEL_X = 0x80024201  # _IOR(BMI160_IOC_MAGIC, 1, s16)
IOCTL_GET_ACCEL_Y = 0x80024202  # _IOR(BMI160_IOC_MAGIC, 2, s16)
IOCTL_GET_ACCEL_Z = 0x80024203  # _IOR(BMI160_IOC_MAGIC, 3, s16)
IOCTL_CALIBRATE_SENSOR = 0x80024204  # _IOR(BMI160_IOC_MAGIC, 4, s16)
IOCTL_GET_GYRO_X = 0x80024205  # _IOR(BMI160_IOC_MAGIC, 5, s16)
IOCTL_GET_GYRO_Y = 0x80024206 # _IOR(BMI160_IOC_MAGIC, 6, s16)
IOCTL_GET_GYRO_Z = 0x80024207 # _IOR(BMI160_IOC_MAGIC, 7, s16)

serial = i2c(port=0, address=0x3C)   # 0x3C is the address of oled display
device = ssd1306(serial, width=128, height=32)

BUTTON_PIN = 27
GPIO.setmode(GPIO.BCM) # BCM = Broadcom SOC channel
GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

GYRO_SCALE_500 = 65.5 # convert 500 dps to lsb/(deg/s)


def read_axis(fd, ioctl_cmd):
    buf = bytearray(2)  # s16 = 2 bytes
    fcntl.ioctl(fd, ioctl_cmd, buf)
    val, = struct.unpack("h", buf)
    return val


def send_calibrate():
    fd = open("/dev/bmi160_device", "rb", buffering=0)
    buf = bytearray(2) # s16 = 2 bytes
    fcntl.ioctl(fd, IOCTL_CALIBRATE_SENSOR, buf)
    val, = struct.unpack("h", buf)
    fd.close()
    return val


def draw_centered_text(draw, text):
    bbox = draw.textbbox((0, 0), text)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]

    x = (device.width - text_width) // 2
    y = (device.height - text_height) // 2

    draw.text((x, y), text, fill="white")


def get_pitch():
    global last_time
    global current_gyro_pitch

    fd = open("/dev/bmi160_device", "rb", buffering=0)
    ax = read_axis(fd, IOCTL_GET_ACCEL_X)
    ay = read_axis(fd, IOCTL_GET_ACCEL_Y)
    az = read_axis(fd, IOCTL_GET_ACCEL_Z)
    gy = read_axis(fd, IOCTL_GET_GYRO_Y)

    # Convert raw values to g
    ax_g = ax / 16384.0
    ay_g = ay / 16384.0
    az_g = az / 16384.0
    
    # calculate the delta time of measurements
    now = time.time()
    dt = now - last_time
    last_time = now

    # convert raw gyro y to rate
    rate_y = gy / GYRO_SCALE_500

    # Calculate pitch via accel data
    pitch = math.atan2(-ax_g, math.sqrt(ay_g**2 + az_g**2)) * 180.0 / math.pi

    # calculate pitch via gyro data
    current_gyro_pitch = current_gyro_pitch + (rate_y * dt)
    
    fd.close()

    return pitch, current_gyro_pitch


def do_calibration():
    img = Image.new("1", (128, 32), "black")
    draw = ImageDraw.Draw(img)
    draw_centered_text(draw, "CALIBRATING")
    device.display(img)

    status = send_calibrate()

    img = Image.new("1", (128, 32), "black")
    draw = ImageDraw.Draw(img)


    if status != 0:
        draw_centered_text(draw, "CALIBRATING\nFAILED")
    else:
        draw_centered_text(draw, "CALIBRATING\nSUCCESS")

    device.display(img)

    time.sleep(2)


last_button_state = 1 # 1 = Button unpressed, 0 = Button pressed
debounce_time = time.time()

current_gyro_pitch = 0.0
last_time = time.time()

while True:
    if select.select([sys.stdin], [], [], 0)[0]:
        user_input = (sys.stdin.readline().strip()).lower()

        if user_input == "calibrate":
            do_calibration()
            
        if user_input == "exit":
            os._exit(0)

    # read button state
    button_state = GPIO.input(BUTTON_PIN)
    # check if button is pressed but was unpressed before(Falling edge)
    if button_state == 0 and last_button_state == 1:
        # check debounce delay
        if time.time() - debounce_time > 0.25:
            do_calibration()
            debounce_time = time.time()
    last_button_state = button_state

    try:
        pitch, gyro_pitch = get_pitch()
    except Exception as e:
        img = Image.new("1", (128, 32), "black")
        draw = ImageDraw.Draw(img)

        text = "ERROR"
        draw_centered_text(draw, text)
        device.display(img)

        time.sleep(0.1)
        continue

    img = Image.new("1", (128, 32), "black")
    draw = ImageDraw.Draw(img)

    text = f"{pitch:.2f}°"
    draw_centered_text(draw, text)
    text = f"{gyro_pitch:.2f}"
    draw.text((0,0), text, fill="white")
    device.display(img)

    time.sleep(0.1)
