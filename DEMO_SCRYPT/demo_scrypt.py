from luma.core.interface.serial import i2c
from luma.oled.device import ssd1306
from PIL import Image, ImageDraw, ImageFont
import time

serial = i2c(port=0, address=0x3C)   # 0x3C is the address of oled display
device = ssd1306(serial, width=128, height=32)

while True:
    img = Image.new("1", (128, 32), "black")
    draw = ImageDraw.Draw(img)
    draw.text((10, 5), "48", fill="white")
    device.display(img)
    time.sleep(1)
