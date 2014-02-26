#!/bin/sh

(sed '$d' Arduino-usbserial.hex; cat Arduino-usbdfu.hex) > pinoccio-16U2-combined.hex

echo "Install combined hex with the following avrdude command:"
echo "avrdude -p m16u2 -P usb -c avrisp2 -U flash:w:pinoccio-16U2-combined.hex:m -U lfuse:w:0xEF:m -U hfuse:w:0xD9:m -U efuse:w:0xF4:m"
