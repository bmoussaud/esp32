#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include <Ticker.h>
Ticker ticker;

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

int LED = LED_BUILTIN;

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
    WiFiManager wm;
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
}

void loop()
{
    // put your main code here, to run repeatedly:
}