#include <WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
//#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

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

class TemperatureSensor
{

public:
    TemperatureSensor(uint8_t pin, uint8_t type)
    {
        _dht = new DHT_Unified(pin, type);
    }

    void setup()
    {
        Serial.println("Start the DHT sensor.");
        _dht->begin();
        delay(500);
        _dht->temperature().getSensor(&t_sensor);
        _dht->humidity().getSensor(&h_sensor);        
    }

    String sensor_json()
    {
        this->_read_temperature();
        StaticJsonDocument<200> doc;

        doc["sensor"] = "proto1";
        doc["counter"] = ++this->dht_counter;
        
        doc["temperature_c"] = this->t;        
        doc["temperature_c_ts"] = this->t_timestamp;
        
        doc["sensor_temp"] = t_sensor.name;
        doc["sensor_humidity"] = h_sensor.name;
        
        doc["humidity"] = this->h;
        doc["humidity_ts"] = this->h_timestamp;
        
        String output;
        serializeJson(doc, output);
        return output;
    }

private:
    void _read_temperature()
    {
        sensors_event_t event;
        _dht->temperature().getEvent(&event);
        if (isnan(event.temperature))
        {
            Serial.println(F("Error reading temperature!"));
        }
        else
        {
            Serial.print(F("Temperature: "));
            Serial.print(event.temperature);           
            Serial.println(F("°C"));
            Serial.println(F("timestamp: "));
            Serial.println(event.timestamp);
            Serial.println(F("sensor_id: "));
            Serial.println(event.sensor_id);
            this->t = event.temperature;
            this->t_timestamp = event.timestamp;            
            
        }
        // Get humidity event and print its value.
        _dht->humidity().getEvent(&event);
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

    void _dump_sensors_info()
    {
        
    }

    float h;
    float t;
    long t_timestamp;
    long h_timestamp;
    long dht_counter = 0;
    sensor_t t_sensor;
    sensor_t h_sensor;
    DHT_Unified *_dht;
};

#define DHTPIN 4
#define DHTTYPE DHT11

WiFiManager wm;
WebServer server(80);
TemperatureSensor dht11(DHTPIN, DHTTYPE);

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
    //Serial.begin(115200);

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
    Serial.println("connected...yeey :)");
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
}

void loop()
{
    server.handleClient();
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
}
