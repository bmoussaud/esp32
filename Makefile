compile:	
	arduino-cli compile --fqbn esp32:esp32:esp32doit-devkit-v1 TempSensor

upload: compile
	arduino-cli upload -v -p /dev/cu.SLAB_USBtoUART  --fqbn esp32:esp32:esp32doit-devkit-v1 TempSensor

# https://github.com/arduino/arduino-cli/issues/876
serial:
	cat /dev/cu.SLAB_USBtoUART

libs:
	arduino-cli lib install ArduinoJson
	arduino-cli lib install WifiManager
	arduino-cli lib install "DHT sensor library"
	arduino-cli lib install "Adafruit Unified Sensor"
	arduino-cli lib install PubSubClient

