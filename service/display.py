from luma.core.interface.serial import spi
from luma.oled.device import ssd1309
from luma.core.render import canvas
from PIL import ImageFont

# SPI setup: adjust GPIO pins as needed
serial = spi(device=0, port=0, gpio_DC=27, gpio_RST=22)

# Initialize SSD1309
device = ssd1309(serial, width=128, height=64)

# Draw text
with canvas(device) as draw:
    draw.text((0, 0), "Hello SSD1309!", fill=255)

# Optional: scroll text
import time
for i in range(128):
    with canvas(device) as draw:
        draw.text((i, 0), "Scrolling!", fill=255)
    time.sleep(0.05)