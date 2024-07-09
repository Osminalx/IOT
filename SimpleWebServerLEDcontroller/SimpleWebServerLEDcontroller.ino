/*
  WiFi Web Server LED Blink

 A simple web server that lets you blink an LED via the web.
 This sketch will print the IP address of your WiFi Shield (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 9.

 If the IP address of your shield is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the WiFi.begin() call accordingly.

 Circuit:
 * WiFi shield attached
 * LED attached to pin 9

 created 25 Nov 2012
 by Tom Igoe
 */
#include <SPI.h>
#include <WiFi101.h>

#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int keyIndex = 0;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

int ledPin = 7;
int brightness = 0;
int fadeAmount = 5;
bool blinking = false;
int blinkInterval = 200; // Intervalo de tiempo inicial entre blinks (en milisegundos)

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid,pass);
    delay(10000);
  }
  server.begin();
  printWiFiStatus();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("new client");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.print("Click <a href=\"/H\">here</a> turn the LED on pin 7 on<br>");
            client.print("Click <a href=\"/B\">here</a> make the LED on pin 7 blink<br>");
            client.print("Click <a href=\"/L\">here</a> turn the LED on pin 7 off<br>");
            client.print("Click <a href=\"/F\">here</a> control LED brightness<br>");

            client.println();
            break;
          }
          else {
            currentLine = "";
          }
        }
        else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /H")) {
          blinking = false;
          analogWrite(ledPin, 255); // Encender el LED al máximo de brillo
        }
        if (currentLine.endsWith("GET /B")) {
          blinking = true;
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();

          client.println("<form method=\"get\" action=\"/I\">");
          client.println("<label for=\"blinkInterval\">Blink interval (1 to 10000 ms):</label><br>");
          client.println("<input type=\"number\" id=\"interval\" name=\"interval\" min=\"1\" max=\"10000\"><br>");
          client.println("<input type=\"submit\" value=\"Submit\">");
          client.println("</form>");

          client.println();
          break;
        }
        if (currentLine.endsWith("GET /L")) {
          blinking = false;
          analogWrite(ledPin, 0); // Apagar el LED
        }
        if (currentLine.endsWith("GET /F")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();

          client.println("<form method=\"get\" action=\"/A\">");
          client.println("<label for=\"brightness\">Brightness (0 to 255):</label><br>");
          client.println("<input type=\"range\" id=\"brightness\" name=\"brightness\" min=\"0\" max=\"255\"><br>");
          client.println("<input type=\"submit\" value=\"Submit\">");
          client.println("</form>");

          client.println();
          break;
        }
        if (currentLine.startsWith("GET /A?brightness=")) {
          int separator = currentLine.indexOf('=');
          String value = currentLine.substring(separator + 1);
          brightness = value.toInt();
          analogWrite(ledPin, brightness);
        }
        if (currentLine.startsWith("GET /I?interval=")) {
          int separator = currentLine.indexOf('=');
          String value = currentLine.substring(separator + 1);
          blinkInterval = value.toInt();
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }

  if (blinking) {
    analogWrite(ledPin, 255); // Encender el LED al máximo de brillo
    delay(blinkInterval);
    analogWrite(ledPin, 0); // Apagar el LED
    delay(blinkInterval);
  }
}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
