#include <SPI.h>
#include <WiFi101.h>
#include <HttpClient.h>
#include <ArduinoJson.h>

#include "DHT.h"
#include "arduino_secrets.h"

#define DHTPIN 9
#define DHTTYPE DHT11

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int keyIndex = 0;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

bool is_blinking = false;
const int TMP_LED = 7;
const int HUM_LED = 8;  

DHT dht(DHTPIN, DHTTYPE);

float dht_temperature = 0;
float dht_humidity = 0;

// threshold values
float TEMPERATURE_THRESHOLD = 100;
float HUMIDITY_THRESHOLD = 100;

void setup() {
  Serial.begin(9600);
  pinMode(TMP_LED, OUTPUT);
  pinMode(HUM_LED, OUTPUT);

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  server.begin();
  printWiFiStatus();

  dht.begin();
}

void loop() {
  WiFiClient client = server.available();

  dht_humidity = dht.readHumidity();
  dht_temperature = dht.readTemperature();

  sendDataToServer(3, 3, 3); // calling with mock values
  delay(500);

  // WiFi client
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

            client.print("Click <a href=\"/TMP\">here</a> to set the temperature<br>");
            client.print("Click <a href=\"/HUM\">here</a> to set the humidity<br>");
            break;
          }
          else {
            currentLine = "";
          }
        }
        else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /TMP")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();

          client.println("<form method=\"get\" action=\"/SETTMP\">");
          client.println("<label for=\"tmp\">Temperature threshold:</label><br>");
          client.println("<input type=\"input\" id=\"tmp\" name=\"tmp\"><br>");
          client.println("<input type=\"submit\" value=\"Submit\">");
          client.println("</form>");
       
          client.println();
          break;
        }
        if (currentLine.endsWith("GET /HUM")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();

          client.println("<form method=\"get\" action=\"/SETHUM\">");
          client.println("<label for=\"hum\">Humidity threshold:</label><br>");
          client.println("<input type=\"input\" id=\"hum\" name=\"hum\"><br>");
          client.println("<input type=\"submit\" value=\"Submit\">");
          client.println("</form>");

          client.println();
          break;
        }
        if (currentLine.endsWith("GET /BRIGHT")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();

          client.println("<form method=\"get\" action=\"/BRIGHTNESS\">");
          client.println("<label for=\"brightness\">Brightness:</label><br>");
          client.println("<input type=\"range\" id=\"brightness\" name=\"brightness\" min=\"0\" max=\"255\"><br>");
          client.println("<input type=\"submit\" value=\"Submit\">");
          client.println("</form>");

          client.println();
          break;
        }

        if (currentLine.startsWith("GET /SETTMP?tmp=")) {
          int separator = currentLine.indexOf('=');
          String value = currentLine.substring(separator + 1);
          TEMPERATURE_THRESHOLD = value.toInt();
          Serial.println("TEMPERATURE VALUE SET");
        }
        if (currentLine.startsWith("GET /SETHUM?hum=")) {
          int separator = currentLine.indexOf('=');
          String value = currentLine.substring(separator + 1);
          HUMIDITY_THRESHOLD = value.toInt();
          Serial.println("HUMIDITY VALUE SET");
        }
        if (currentLine.startsWith("GET /BRIGHTNESS?parameter=")) {
          int separator = currentLine.indexOf('=');
          String value = currentLine.substring(separator + 1);
          brightness_val = value.toInt();
          analogWrite(TMP_LED, brightness_val);
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }

  //triggered by tmp
  if (dht_temperature < TEMPERATURE_THRESHOLD) {
    digitalWrite(TMP_LED, LOW);
  } else {
    digitalWrite(TMP_LED, HIGH);
  }

  //triggered by hum
  if (dht_humidity < HUMIDITY_THRESHOLD) {
    digitalWrite(HUM_LED, LOW);
  } else {
    digitalWrite(HUM_LED, HIGH);
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

void sendDataToServer(float humidity, float temperature, float pressure) {
  HTTPClient httpClient;

  httpClient.begin(SUPABASE_URL);
  httpClient.addHeader("apikey", SUPABASE_API_KEY);
  httpClient.addHeader("Content-Type", "application/json");

  const size_t CAPACITY = JSON_OBJECT_SIZE(3);
  StaticJsonDocument<CAPACITY> doc;

  JsonObject data = doc.to<JsonObject>();
  data["humidity"] = humidity;
  data["temperature"] = temperature;
  data["pressure"] = pressure;

  serialize(data, jsonObject);

  // execute and get response back
  int httpCode = httpClient.POST(String(jsonObject));
  Serial.printf("\nResponse code -> %d\t", httpCode);

  if (httpCode == HTTP_CODE_OK) {
    Serial.println(httpClient.getString());
  } else {
    Serial.println(httpClient.errorToString(httpCode).c_str());
  }

  httpClient.end();
}




