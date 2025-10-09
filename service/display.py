from luma.core.interface.serial import spi
from luma.oled.device import ssd1309
from luma.core.render import canvas

serial = spi(device=0, port=0, gpio_DC=27, gpio_RST=22)
device = ssd1309(serial, width=128, height=64)

with canvas(device) as draw:
    draw.rectangle(device.bounding_box, outline=255, fill=0)
    draw.text((10, 10), "Test", fill=255)
