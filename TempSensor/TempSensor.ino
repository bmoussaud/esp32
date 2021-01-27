#include <WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <Ticker.h>
Ticker ticker;

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

int LED = LED_BUILTIN;

#define PIN_RESET_BUTTON 4
int RESET = 0;

WiFiManager wm;
WebServer server(80);

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

void setup()
{
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // put your setup code here, to run once:
    Serial.begin(115200);

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
    Serial.println("HTTP server started");
    delay(100);
}

void loop()
{
    server.handleClient();

    RESET = digitalRead(PIN_RESET_BUTTON);
    if (RESET == HIGH)
    {
        Serial.println("Erase settings and restart ...");
        delay(1000);
        wm.resetSettings();
        ESP.restart();
    }
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
    StaticJsonDocument<200> doc;

    doc["temperature"] = "15";
    doc["device"] = "esp_32b";
    doc["room"] = "saloon";
    
    String output;
    serializeJson(doc, output);
    
    Serial.println("2 handle_temperature");
    Serial.println(output);
    Serial.println("/2 handle_temperature");
    
    server.send(200, "application/json", output);

    Serial.println("<<< OUT handle_temperature");
}