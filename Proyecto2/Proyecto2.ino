#include <ArduinoHttpClient.h>
#include <WiFi101.h>
#include "DHT.h"

#define DHTPIN 9
#define DHTTYPE DHT11

int LED1 = 7;
int LED2 = 8;

DHT dht(DHTPIN, DHTTYPE);

#include "arduino_secrets.h" 
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const char serverAddress[] = "172.20.10.10";
const int port = 8000;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

void setup() {
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  dht.begin();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  Serial.println("WIFI status: ");
  Serial.println(WiFi.status());

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.println(t);
  Serial.print("Humidity: ");
  Serial.println(h);

  String contentType = "application/json";
  String postData = "{\"id\": 0, \"temperature\": " + String(t) + ", \"humidity\": " + String(h) + "}";
  
  Serial.print("Post data: ");
  Serial.println(postData);

  client.post("/ar/sens/", contentType, postData);

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  client.get("/ar/limits/");
  statusCode = client.responseStatusCode();
  response = client.responseBody();

  // Parsear la respuesta JSON para obtener los límites de temperatura y humedad
  float tempLimit, humLimit;
  if (response.length() > 0) {
    int idx1 = response.indexOf("\"temp_limit\":") + 13;
    int idx2 = response.indexOf(",", idx1);
    tempLimit = response.substring(idx1, idx2).toFloat();

    int idx3 = response.indexOf("\"hum_limit\":") + 12;
    int idx4 = response.indexOf("}", idx3);
    humLimit = response.substring(idx3, idx4).toFloat();

    Serial.print("Temp Limit: ");
    Serial.println(tempLimit);
    Serial.print("Hum Limit: ");
    Serial.println(humLimit);

    // Controlar los LEDs según los límites recibidos
    digitalWrite(LED1, t > tempLimit ? HIGH : LOW);
    digitalWrite(LED2, h > humLimit ? HIGH : LOW);
  }

  delay(5000);
}

