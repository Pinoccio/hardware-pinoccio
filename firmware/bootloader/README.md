Pinoccio bootloader
===================
To compile, make sure that you have avr-gcc and friends in your path and
run:

	$ make

This should compile the uracoli lib (for radio / OTA updating) and the
main bootloader. The produced bootloader.hex can be uploaded through
avrdude normally, or through the IDE by putting it in the right place
(see boards.txt for the filename to use).
