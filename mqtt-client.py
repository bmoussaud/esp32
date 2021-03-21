import os
import time
import sys
import paho.mqtt.client as mqtt
import json

SERVEUR = 'gimmick01.local'
# ACCESS_TOKEN = 'DHT22_DEMO_TOKEN'

# Data capture and upload interval in seconds. Less interval will eventually hang the DHT22.
INTERVAL = 5

#sensor_data = {'temperature': 0, 'humidity': 0}
sensor_data = {}
sensor_data['sensor'] = 'proto1'
sensor_data['counter'] = 12
sensor_data['temperature'] = {'value': '12',
                              'sensor': 'esp32', 'timestamp': 12234}
sensor_data['humidity'] = {'value': '55',
                           'sensor': 'esp32', 'timestamp': 12234}

next_reading = time.time()

client = mqtt.Client()

# Set access token
# client.username_pw_set(ACCESS_TOKEN)

# Connect to ThingsBoard using default MQTT port and 60 seconds keepalive interval
client.connect(SERVEUR, 1883, 60)

client.loop_start()

try:
    while True:
        humidity, temperature = (22, 51)
        humidity = round(humidity, 2)
        temperature = round(temperature, 2)
        print(u"Temperature: {:g}\u00b0C, Humidity: {:g}%".format(
            temperature, humidity))
        sensor_data['counter'] = sensor_data['counter'] + 1
        sensor_data['temperature']['value'] = temperature
        sensor_data['temperature']['timestamp'] = sensor_data['temperature']['timestamp'] + 60
        sensor_data['humidity']['value'] = humidity
        sensor_data['humidity']['timestamp'] = sensor_data['humidity']['timestamp'] + 60
        print(json.dumps(sensor_data))
        # Sending humidity and temperature data to ThingsBoard
        client.publish('sensors/data/device_1', json.dumps(sensor_data), 1)

        next_reading += INTERVAL
        sleep_time = next_reading-time.time()
        if sleep_time > 0:
            time.sleep(sleep_time)
except KeyboardInterrupt:
    pass

client.loop_stop()
client.disconnect()
