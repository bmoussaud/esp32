compile:
	arduino-cli compile --fqbn esp32:esp32:esp32doit-devkit-v1 TempSensor

upload: compile
	arduino-cli upload -p /dev/cu.SLAB_USBtoUART  --fqbn esp32:esp32:esp32doit-devkit-v1 TempSensor

