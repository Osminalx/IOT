#include <WiFi101.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Servo.h>
#include <ArduinoJson.h>

// Configuraciones WiFi
#include "arduino_secrets.h" 
char ssid[] = SECRET_SSID;        
char pass[] = SECRET_PASS;

// Configuraciones MQTT
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic = "puerta/inteligente/iot/osmin/y/leo/status";
const char* mqtt_topic_emergencia = "puerta/inteligente/iot/osmin/y/leo/emergencias";

// Pines del DHT11
#define DHTPIN 7
#define DHTTYPE DHT11

// Pines del servomotor
#define SERVO_PIN 6

// Umbrales de temperatura y humedad
const float temp_threshold = 25.0;
const float hum_threshold = 20.0;

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
Servo servo;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin();
  servo.attach(SERVO_PIN);
  servo.write(0); // Inicialmente la puerta está cerrada (ajusta según tu configuración)
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar al servidor MQTT...");
    if (client.connect("ArduinoClient")) {
      Serial.println("Conectado");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando nuevamente en 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.print(" C, Humedad: ");
  Serial.print(humidity);
  Serial.println(" %");

  if (temperature > temp_threshold || humidity < hum_threshold) {
    Serial.println("Condiciones de emergencia detectadas.");
    abrirPuerta();
    
    // Construir el objeto JSON
    StaticJsonDocument<200> jsonDocument;
    jsonDocument["temperature"] = temperature;
    jsonDocument["humidity"] = humidity;
    
    // Convertir el objeto JSON a una cadena
    char jsonString[200];
    serializeJson(jsonDocument, jsonString);
    
    // Enviar el mensaje MQTT con la cadena JSON
    client.publish(mqtt_topic_emergencia, jsonString);
    Serial.println("Mensaje MQTT de emergencia enviado");
  } else {
    String payload = "Temperatura: " + String(temperature) + "C, Humedad: " + String(humidity) + "%";
    Serial.print("Temperatura y humedad actuales:" );
    Serial.println(payload);
  }

  delay(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  
  // Convertimos el payload a un string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  
  Serial.println(message);

  if (String(topic) == mqtt_topic) {
    // Analizamos el JSON
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
      Serial.print("Error al analizar JSON: ");
      Serial.println(error.c_str());
      return;
    }
    
    const char* status = doc["status"];
    
    if (strcmp(status, "open") == 0) {
      abrirPuerta();
    } else if (strcmp(status, "closed") == 0) {
      cerrarPuerta();
    }
  }
}

void abrirPuerta() {
  Serial.println("Abriendo puerta...");
  servo.write(180); // Ajusta según la posición de apertura de tu servomotor
  delay(2000); // Pausa de 2 segundos después de abrir la puerta
}

void cerrarPuerta() {
  Serial.println("Cerrando puerta...");
  servo.write(0); // Ajusta según la posición de cierre de tu servomotor
  delay(2000); // Pausa de 2 segundos después de cerrar la puerta
}


