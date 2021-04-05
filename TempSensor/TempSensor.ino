#include <WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
//#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient

#include <Ticker.h>
#include <stdlib.h>
#include <time.h>

Ticker ticker;

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif
int LED = LED_BUILTIN;

#define PIN_RESET_BUTTON 4
int RESET = 0;

class MQTTSender
{
    std::shared_ptr<PubSubClient> client;
    WiFiClient _espClient;
    String deviceType = "ESP32";
    char clientId[40];
    char deviceId[40];
    char chipId[40];
    char infoTopic[200];
    char dataTopic[200];

public:
    MQTTSender()
    {
    }

    void setup()
    {
        Serial.println("MQTTSender| setup");
        String _chipId = String(WIFI_getChipId(), HEX);
        String _mac = String(WiFi.macAddress());
        _mac.toLowerCase();
        _mac.replace(":", "");
        _mac.replace("240ac4", "a"); // vendor = Espressif Inc.
        String _clientId = "ESP_" + _mac;

        strcpy(chipId, _chipId.c_str());
        strcpy(clientId, _clientId.c_str());
        strcpy(deviceId, _clientId.c_str());

        snprintf(infoTopic, sizeof(infoTopic), "%s%s", "sensors/info/", deviceId);
        snprintf(dataTopic, sizeof(dataTopic), "%s%s", "sensors/data/", deviceId);

        Serial.print("MQTTSender| clientId: ");
        Serial.println(clientId);

        client.reset(new PubSubClient(_espClient));
        client->setServer("192.168.1.41", 1883);
    }

    void disconnect()
    {
        if (client->connected())
        {
            _registerDevice("disconnected");
        }
        client->disconnect();
        Serial.print("MQTTSender| clientId: ");
        Serial.println(clientId);
        Serial.println("disconnected");
    }

    void reconnect()
    {
        if (!client->connected())
        {
            Serial.print("MQTTSender| attempting MQTT connection...");
            Serial.print("MQTTSender| clientId: ");
            Serial.println(clientId);
            // Attempt to connect
            if (client->connect(clientId))
            {
                Serial.println("connected");
                // Once connected, publish an announcement...
                _registerDevice("connected");
            }
            else
            {
                Serial.print("failed, rc=");
                Serial.print(client->state());
                ///Serial.println(" try again in 5 seconds");
                // Wait 5 seconds before retrying
                //delay(5000);
            }
        }
    }

    void _registerDevice(char* status)
    {
        StaticJsonDocument<2000> doc;
        JsonObject root = doc.to<JsonObject>();

        root["deviceType"] = deviceType;
        root["deviceId"] = deviceId;
        root["clientId"] = clientId;
        root["chipId"] = chipId;
        root["sketchName"] = "BMO";
        root["status"] = status;

        publishInfo(root);
    }

    void publishInfo(JsonObject root)
    {
        String output;
        serializeJson(root, output);
        publish(infoTopic, output);
    }

    void publishData(String output)
    {
        publish(dataTopic, output);
    }

    void publish(char *topic, String output)
    {
        reconnect();
        Serial.print("MQTTSender|Topic:>  ");
        Serial.println(topic);
        Serial.print("MQTTSender|output:> ");
        Serial.println(output);
        char buffer[2000];
        output.toCharArray(buffer, 2000);

        if (client->publish(topic, buffer) == true)
        {
            //Serial.println("WMM: Success sending message to register the device...");
        }
        else
        {
            Serial.println("MQTTSender| Error sending message to register the device...");
            Serial.print("MQTTSender| State:");
            Serial.print(client->state());
            Serial.println(".");
        }
    }
};

class TemperatureSensor
{

public:
    TemperatureSensor(uint8_t pin, uint8_t type)
    {
        _dhtu = new DHT_Unified(pin, type);
        _dht = new DHT(pin, type);
    }

    void setup()
    {
        Serial.println("Start the DHT sensor.");
        _dhtu->begin();
        delay(500);
        _dhtu->temperature().getSensor(&t_sensor);
        _dhtu->humidity().getSensor(&h_sensor);
    }

    String sensor_json()
    {
        this->_read_temperature();
        StaticJsonDocument<512> doc;

        JsonObject root = doc.to<JsonObject>();
        root["sensor"] = "proto1";
        root["counter"] = ++this->dht_counter;
        root["temperature"] = this->t;
        root["timestamp"] = this->t_timestamp;
        root["sensor"] = t_sensor.name;
        root["humidity"] = this->h;
        root["heatIndex"] = _dht->computeHeatIndex(this->t, this->h, false);

        String output;
        serializeJson(doc, output);
        return output;
    }

private:
    void _read_temperature()
    {
        sensors_event_t event;
        _dhtu->temperature().getEvent(&event);
        if (isnan(event.temperature))
        {
            Serial.println(F("Error reading temperature!"));
        }
        else
        {
            this->t = event.temperature;
            this->t_timestamp = event.timestamp;
        }

        _dhtu->humidity().getEvent(&event);
        if (isnan(event.relative_humidity))
        {
            Serial.println(F("Error reading humidity!"));
        }
        else
        {
            this->h = event.relative_humidity;
            this->h_timestamp = event.timestamp;
        }
    }

    float h;
    float t;
    long t_timestamp;
    long h_timestamp;
    long dht_counter = 0;
    sensor_t t_sensor;
    sensor_t h_sensor;
    DHT_Unified *_dhtu;
    DHT *_dht;
};

#define DHTPIN 4
#define DHTTYPE DHT11

WiFiManager wm;
WebServer server(80);
TemperatureSensor dht11(DHTPIN, DHTTYPE);
MQTTSender mqttsender;
bool sensor = false;

void tick()
{
    //toggle state
    digitalWrite(LED, !digitalRead(LED)); // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    //if you used auto generated SSID, print it
    Serial.println(myWiFiManager->getConfigPortalSSID());
    //entered config mode, make led toggle faster
    ticker.attach(0.2, tick);
}

// the setup function runs once when you press reset or power the board
void led_setup()
{
    Serial.begin(9600);
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void led_loop()
{
    Serial.println("Hello....2");
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(100);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
    delay(1000);                     // wait for a second
}

void setup_wifi()
{
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // put your setup code here, to run once:
    Serial.begin(9600);

    //set led pin as output
    pinMode(LED, OUTPUT);
    // start ticker with 0.5 because we start in AP mode and try to connect
    ticker.attach(0.6, tick);

    delay(1000);

    //reset settings - for testing
    // wm.resetSettings();

    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wm.setAPCallback(configModeCallback);

    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    if (!wm.autoConnect())
    {
        Serial.println("failed to connect and hit timeout");
        //reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(1000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("WIFI connected...");
    ticker.detach();
    //keep LED on
    digitalWrite(LED, LOW);

    Serial.println("local ip");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.subnetMask());

    // Display an HTML interface to the project from a browser on esp32_ip_address /
    server.on("/", handle_root);
    server.on("/temp", handle_temperature);
    server.on("/sensor_on", handle_sensor_on);
    server.on("/sensor_off", handle_sensor_off);

    server.begin();
    Serial.println("HTTP server started.");
    delay(100);
}

void reset_wifi()
{
    RESET = digitalRead(PIN_RESET_BUTTON);
    if (RESET == HIGH)
    {
        Serial.println("Erase settings and restart ...");
        delay(1000);
        wm.resetSettings();
        ESP.restart();
    }
}

void setup()
{
    setup_wifi();
    dht11.setup();
    mqttsender.setup();
}

void loop()
{
    server.handleClient();
    loop_temperature(sensor, 15000);
}

// HTML & CSS contents which display on web server
String HTML = "<!DOCTYPE html>\
<html>\
  <body>\
    <h1>Welcome</h1>\
    <p>Your first Iot Project made with ESP32</p>\
    <p>&#128522;</p>\
  </body>\
</html>";
// Handle root url (/)
void handle_root()
{
    Serial.println("handle_temperature");
    Serial.println(HTML);
    Serial.println("/handle_temperature");
    server.send(200, "text/html", HTML);
}

// Handle root url (/temp)
void handle_temperature()
{
    Serial.println(">>> IN handle_temperature");
    String output;
    output = dht11.sensor_json();
    Serial.println(output);
    server.send(200, "application/json", output);
    //mqttsender.publishData(output);
}

//  Handle (/sensor_off)
void handle_sensor_off()
{
    Serial.println(">>> IN handle_sensor_off");
    mqttsender.disconnect();
    sensor = false;
    server.send(200, "text/html", "sensor_off");
}

// Handle (/sensor_on)
void handle_sensor_on()
{
    Serial.println(">>> IN handle_sensor_on");
    mqttsender.reconnect();
    sensor = true;
    server.send(200, "text/html", "sensor_on");
}

void loop_temperature(boolean sensor, int duration)
{
    if (sensor)
    {
        Serial.println(">>> IN loop_temperature ");
        mqttsender.publishData(dht11.sensor_json());
        delay(duration);
    }
}
