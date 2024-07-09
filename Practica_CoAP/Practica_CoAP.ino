#include <WiFi101.h>
#include <SimpleCoAP.h>
#include <DHT.h>

#define DHTPIN 7
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

int ledPin = 6;
bool ledState = false;

char ssid[] = "MiRed";     // Nombre de tu red Wi-Fi
char pass[] = "MiContraseña"; // Contraseña de tu red Wi-Fi

void handleTemperature(CoapPacket &packet, IPAddress ip, int port) {
  float temperature = dht.readTemperature();
  CoapPacket response = CoapPacket(IPAddress(0,0,0,0), 0, 0);
  response.setCode(COAP_CONTENT);
  response.setContentType(COAP_TEXT_PLAIN);
  response.setPayload(String(temperature));
  sendPacket(response, ip, port);
}

void handleHumidity(CoapPacket &packet, IPAddress ip, int port) {
  float humidity = dht.readHumidity();
  CoapPacket response = CoapPacket(IPAddress(0,0,0,0), 0, 0);
  response.setCode(COAP_CONTENT);
  response.setContentType(COAP_TEXT_PLAIN);
  response.setPayload(String(humidity));
  sendPacket(response, ip, port);
}

void handleLED(CoapPacket &packet, IPAddress ip, int port) {
  if(packet.getPayload() == "on") {
    digitalWrite(ledPin, HIGH);
    ledState = true;
  } else if(packet.getPayload() == "off") {
    digitalWrite(ledPin, LOW);
    ledState = false;
  }
  CoapPacket response = CoapPacket(IPAddress(0,0,0,0), 0, 0);
  response.setCode(COAP_CHANGED);
  sendPacket(response, ip, port);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  dht.begin();
  Coap.start();
  Coap.addResource("temperature", COAP_GET, handleTemperature);
  Coap.addResource("humidity", COAP_GET, handleHumidity);
  Coap.addResource("led", COAP_PUT, handleLED);

  // Conexión a la red Wi-Fi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  Coap.loop();
}

