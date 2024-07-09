#include <WiFi101.h>
#include <PubSubClient.h>
#include <DHT.h>

// Credenciales de tu red WiFi
const char* ssid = "CETYS_Visitantes";
const char* password = "claseiot";

// Dirección IP o nombre de host del broker MQTT
const char* mqtt_server = "broker.hivemq.com";

// Temas MQTT
const char* topicMKR = "osmin/arduino/sensor/dht";
const char* topicTemp = "osmin/arduino/sensor/dht/temp";
const char* topicHum = "osmin/arduino/sensor/dht/hum";
const char* ledInfo = "osmin/arduino/led";

// LED
int LED = 6;

// DHT11
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();
}

void loop() {
   if (!client.connected()) {
    Serial.print("Conectando al broker MQTT...");
    if (client.connect("MKR1000Client", "", "", topicMKR, 1, true, "MKR1000: Me morí")) {
      Serial.println("Conectado!");
      client.subscribe(ledInfo);
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos");
      delay(5000);
      return;
    }
  }
  client.loop();

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

if (!isnan(temp)) {
    char tempMsg[10];
    sprintf(tempMsg, "%.2f", temp);
    client.publish(topicTemp, tempMsg,true);
    delay(2000);
}

if (!isnan(hum)) {
    char humMsg[10];
    sprintf(humMsg, "%.2f", hum);
    client.publish(topicHum, humMsg,true);
    delay(2000);
}

}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, ledInfo) == 0) {
    if ((char)payload[0] == '1') {
      digitalWrite(LED, HIGH);
    } else if ((char)payload[0] == '0') {
      digitalWrite(LED, LOW);
    }
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  printWiFiStatus();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    String clientId = "MKR1000Client";
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado");
      client.subscribe(ledInfo);
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos");
      delay(5000);
    }
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
}
