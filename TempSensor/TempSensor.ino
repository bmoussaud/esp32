#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif



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
    delay(100);                     // wait for a second
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
    delay(1000);                     // wait for a second
}


const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
WebServer server(80);

void handleRoot()
{
    String page = "<!DOCTYPE html>";
    page += "<head>";
    page += "    <title>Serveur ESP32</title>";
    page += "    <meta http-equiv='refresh' content='60' name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8'/>";
    page += "</head>";

    page += "<body lang='fr'>";
    page += "    <h1>Serveur</h1>";
    page += "    <p>Ce serveur est hébergé sur un ESP32</p>";
    page += "    <i>Créé par Benoit Moussaud</i>";
    page += "</body>";

    page += "</html>";

    server.send(200, "text/html", page);
}

void handleNotFound()
{
    server.send(404, "text/plain", "404: Not found");
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n");

    WiFi.persistent(false);
    WiFi.begin(ssid, password);
    Serial.print(ssid);
    Serial.print("Tentative de connexion...");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    Serial.println("\n");
    Serial.println("Connexion etablie!");
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.println("Serveur web actif!");
}

void loop()
{
    server.handleClient();
}