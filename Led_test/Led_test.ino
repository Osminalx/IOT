#include <Joystick.h>
#include <AxisJoystick.h>

#define SW_PIN 9
#define VRX_PIN A0
#define VRY_PIN A1

Joystick* joystick;


const int LED = 8;
const int DELAY_TIME = 500;
int led_brightness = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
  joystick = new AxisJoystick(SW_PIN, VRX_PIN, VRY_PIN);
}

void loop() {
  Serial.print("Joystick, Multiple Reading: ");
  Serial.println(moveTitle(joystick->multipleRead()));

  delay(500);
  // put your main code here, to run repeatedly:
  
}

String moveTitle(const Joystick::Move move) {
  switch (move) {
    case Joystick::Move::NOT:
      return "NOT";
    case Joystick::Move::PRESS:
    led_brightness = 255;
    analogWrite(LED,led_brightness);
    Serial.print("Valor: ");
    Serial.println(led_brightness);
      return "PRESS";
    case Joystick::Move::UP:
    led_brightness +=25;
    if(led_brightness > 255){
      led_brightness =255;
    }
    analogWrite(LED,led_brightness);
    Serial.print("Valor: ");
    Serial.println(led_brightness);
      return "UP";
    case Joystick::Move::DOWN:
    led_brightness -=25;
    if(led_brightness < 0){
      led_brightness =0;
    }
    analogWrite(LED,led_brightness);
    Serial.print("Valor: ");
    Serial.println(led_brightness);
      return "DOWN";
    case Joystick::Move::RIGHT:
    led_brightness =0;
    analogWrite(LED,led_brightness);
    Serial.print("Valor: ");
    Serial.println(led_brightness);
      return "RIGHT";
    case Joystick::Move::LEFT:
      return "LEFT";
    default:
      return "???";
  }
}
