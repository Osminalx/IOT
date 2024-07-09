#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define LED_PIN 19

Adafruit_BMP085 bmp;
WiFiClient wifi;

#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const char serverAddress[] = "http://172.20.10.10:8000";

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    while (1)
      ;
  }
}

void loop() {
  float presion = bmp.readPressure();

  Serial.print("Presion: ");
  Serial.println(presion);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(String(serverAddress) + "/esp/sens/");

    http.addHeader("Content-Type", "application/json");

    String postData = "{\"id\": 0, \"presion\": " + String(presion) + "}";
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(response);


      http.begin(String(serverAddress) + "/esp/limits/");
      httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        response = http.getString();
        Serial.print("GET Response code: ");
        Serial.println(httpResponseCode);
        Serial.print("GET Response: ");
        Serial.println(response);

        // Parsear la respuesta JSON y comparar con los límites obtenidos
        // Aquí debes agregar la lógica para comparar los valores de presión con los límites obtenidos
      } else {
        Serial.print("Error in GET request: ");
        Serial.println(httpResponseCode);
      }
      // Parsear la respuesta JSON para obtener los límites de temperatura y humedad
      float presLimit;
      if (response.length() > 0) {
        int idx1 = response.indexOf("\"pres_limit\":") + 13;
        int idx2 = response.indexOf(",", idx1);
        presLimit = response.substring(idx1, idx2).toFloat();

        Serial.print("Pres Limit: ");
        Serial.println(presLimit);

        // Controlar el LED según los límites recibidos
        float BarPress = presion / 100000.0; // Puedes ajustar este valor según tu necesidad
        digitalWrite(LED_PIN, BarPress >= presLimit ? HIGH : LOW);

        // Realizar la solicitud GET para obtener los límites
      }
    } else {
      Serial.print("Error in HTTP request: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Unable to connect to server");
  }

  delay(5000);
}
