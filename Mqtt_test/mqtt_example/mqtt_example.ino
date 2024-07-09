/*
 * Ejemplo MQTT para Arduino MKR1000 con PubSubClient
 * Publica un mensaje "¡Hola Mundo!" y se suscribe a un tema
 */

// #include <WiFi101.h>
// #include <WiFi.h>

// Define macros for the different boards
#if defined(ARDUINO_SAMD_MKR1000) // Arduino MKR1000
  #define BOARD_MKR1000
  #include <WiFi101.h>
#elif defined(ESP32) // ESP32 DOIT DEV KIT V1
  #define BOARD_ESP32
  #include <WiFi.h>
#endif


#include <PubSubClient.h>

// Credenciales de tu red WiFi
const char* ssid = "CETYS_Visitantes";
const char* password = "claseiot";

//LED
int LED = 6;

// Dirección IP o nombre de host del broker MQTT
// const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_server = "broker.hivemq.com";

// Temas MQTT
const char* topicMKR = "osmin/ejemplo/Led";
const char* topicESP32 = "ejemplo/ESP32";

//Subscriber MQTT
const char* ledInfo = "osmin/arduino/led"

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883); // Configura el broker MQTT
  client.setCallback(callback); // Función callback para mensajes entrantes
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  static long lastMsg = 0;
  if (now - lastMsg > 15000) { // Publica un mensaje cada 15 segundos
    lastMsg = now;
    char mensaje[50];
  #ifdef BOARD_MKR1000
    snprintf(mensaje, 50, "¡Hola desde MKR! #%ld", now);
  #endif
  #ifdef BOARD_ESP32
    snprintf(mensaje, 50, "¡Hola desde ESP32! #%ld", now);
  #endif
    Serial.print("Publicando mensaje: ");
    Serial.println(mensaje);
    // client.publish(topicPublicar, mensaje);
  #ifdef BOARD_MKR1000
    client.publish(topicMKR, mensaje);
  #endif
  #ifdef BOARD_ESP32
    client.publish(topicESP32, mensaje);
  #endif
  }

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
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
  #ifdef BOARD_MKR1000
    String clientId = "MKR1000Client";
  #endif
  #ifdef BOARD_ESP32
    String clientId = "ESP32Client";
  #endif

    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado");
    #ifdef BOARD_MKR1000
      client.subscribe(topicESP32);
    #endif
    #ifdef BOARD_ESP32
      client.subscribe(topicMKR);
    #endif
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}