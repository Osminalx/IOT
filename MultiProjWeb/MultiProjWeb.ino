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

// include the library code:
#include <LiquidCrystal.h>
#include "DHT.h"

#define DHTPIN 9

#define DHTTYPE DHT11

int LED1 = 7;
int LED2 = 8;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

DHT dht(DHTPIN, DHTTYPE);


#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

unsigned long previousMillis = 0;
const long interval = 500; // intervalo de actualización en milisegundos

float tempDes = 1;
float humDes = 1;

void setup() {
  Serial.begin(9600);      // initialize serial communication
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  dht.begin();

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid,pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWiFiStatus();                        // you're connected now, so print out the status
}


void loop() {
  WiFiClient client = server.available();   // listen for incoming clients
  delay(500);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.print(t);
  lcd.print(" C");

  lcd.setCursor(11, 0);
  lcd.print("MT ");
  lcd.print(tempDes);


  lcd.setCursor(0, 1);
  lcd.print("H: ");
  lcd.print(h);
  lcd.print(" %");

  lcd.setCursor(11, 1);
  lcd.print("MH ");
  lcd.print(humDes);

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        currentLine += c;                   // add it to the currentLine
        // if the current line ends with HTTP GET request
        if (currentLine.endsWith("GET /")) {
          // Continue reading until the end of the HTTP request
          while (client.available()) {
            char c = client.read();
            currentLine += c;
            // Check if we've reached the end of the request
            if (currentLine.endsWith(" HTTP/1.1\r\n")) {
              // Extract temperature and humidity values
              int tempIndex = currentLine.indexOf("temp=");
              int humIndex = currentLine.indexOf("&hum=");
              if (tempIndex != -1 && humIndex != -1) {
                String tempStr = currentLine.substring(tempIndex + 5, humIndex);
                String humStr = currentLine.substring(humIndex + 5);
                tempDes = tempStr.toFloat();
                humDes = humStr.toFloat();
                // Compare with DHT readings and turn on LEDs accordingly
                if (t >= tempDes && h >= humDes) {
                  digitalWrite(LED1, HIGH);
                  digitalWrite(LED2, HIGH);
                } else if (t >= tempDes) {
                  digitalWrite(LED1, HIGH);
                  digitalWrite(LED2, LOW);
                } else if (h >= humDes) {
                  digitalWrite(LED1, LOW);
                  digitalWrite(LED2, HIGH);
                } else {
                  digitalWrite(LED1, LOW);
                  digitalWrite(LED2, LOW);
                }
              }
              // Close the connection
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
              client.println("<html><head><title>Control de LEDs</title></head><body>");
              client.println("<h1>Control de LEDs</h1>");
              client.println("<form action='/' method='get'>");
              client.println("Temperatura deseada: <input type='text' name='temp'><br>");
              client.println("Humedad deseada: <input type='text' name='hum'><br>");
              client.println("<input type='submit' value='Enviar'>");
              client.println("</form>");
              client.println("</body></html>");
              // Stop the client
              client.stop();
              Serial.println("client disonnected");
              break;
            }
          }
        }
      }
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
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}