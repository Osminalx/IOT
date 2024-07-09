#include "DHT.h"
#include <Adafruit_BMP085.h>
#include <WiFi101.h>
#include "secrets.h"
#include "ThingSpeak.h"

#define DHTPIN 2 
#define DHTTYPE DHT11

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP085 bmp;

float tempSumDHT = 0.0;
float humSum = 0.0;
float tempSumBMP = 0.0;
float pressSum = 0.0;
int numReadings = 0;

// Variables para almacenar los valores de los sensores
float tempDHT = 0.0;
float hum = 0.0;
float tempBMP = 0.0;
float press = 0.0;

// Variables globales
unsigned long lastSensorReadTime = 0;  // Variable para almacenar el tiempo de la última lectura de sensor
const unsigned long sensorInterval = 2000;  // Intervalo de tiempo entre lecturas de sensor en milisegundos
int readingsCount = 0;  // Contador de lecturas de sensor

void setup() {
  Serial.begin(115200); // Initialize serial 
  dht.begin();
  if (!bmp.begin()){
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }

  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {

  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  // Verificar si ha pasado el tiempo suficiente para realizar otra lectura de sensor
  if (millis() - lastSensorReadTime >= sensorInterval) {
    lastSensorReadTime = millis();  // Actualizar el tiempo de la última lectura de sensor
    // Realizar la lectura de los sensores
    tempDHT = getTemp();
    hum = getHum();
    tempBMP = bmp.readTemperature();
    press = bmp.readPressure();

    if(tempDHT != 0.0 && hum != 0.0 && !isnan(tempBMP) && !isnan(press)){
      // Add up the readings for averaging
      tempSumDHT += tempDHT;
      humSum += hum;
      tempSumBMP += tempBMP;
      pressSum += press;
      numReadings++;
    }
    readingsCount++;  // Incrementar el contador de lecturas
  }

  // Verificar si se han realizado suficientes lecturas para enviar los datos a ThingSpeak
  if (readingsCount == 10) {
    float avgTempDHT = tempSumDHT / numReadings;
    float avgHum = humSum / numReadings;
    float avgTempBMP = tempSumBMP / numReadings;
    float avgPress = pressSum / numReadings;

    ThingSpeak.setField(1, tempDHT);
    ThingSpeak.setField(2, avgTempDHT);
    ThingSpeak.setField(3, hum);
    ThingSpeak.setField(4, avgHum);
    ThingSpeak.setField(5, tempBMP);
    ThingSpeak.setField(6, avgTempBMP);
    ThingSpeak.setField(7, press);
    ThingSpeak.setField(8, avgPress);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }

    // Reset sums and readings count
    tempSumDHT = 0.0;
    humSum = 0.0;
    tempSumBMP = 0.0;
    pressSum = 0.0;
    numReadings = 0;
    readingsCount = 0;  // Reiniciar el contador de lecturas
    delay(18000); // Esperar 18 segundos para completar los 20 segundos antes de la próxima lectura de sensor
  }

}

float getTemp(){
  float t = dht.readTemperature();
  float f = dht.readTemperature(false);
  if (isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read temp from DHT sensor!"));
    return 0.0;
  }
  return t;
}

float getHum(){
  float h = dht.readHumidity();
  if(isnan(h)){
    Serial.println(F("Failed to read hum from DHT sensor!"));
    return 0.0;
  }
  return h;
}
