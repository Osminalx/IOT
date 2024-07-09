#include <AccelStepper.h>

// Definimos los pines utilizados por el motor
#define IN1 1
#define IN2 2
#define IN3 3
#define IN4 4

// Inicializamos la biblioteca AccelStepper
AccelStepper stepper(AccelStepper::FULL4WIRE, IN1, IN3, IN2, IN4);

void setup() {
  // Configuramos la velocidad máxima en pasos por segundo
  stepper.setMaxSpeed(1000);
  // Configuramos la aceleración en pasos por segundo^2
  stepper.setAcceleration(500);
  // Configuramos la velocidad inicial en pasos por segundo
  stepper.setSpeed(200);

  Serial.begin(9600);
}

void loop() {
  // Giramos una vuelta completa en sentido horario
  Serial.println("clockwise");
  stepper.moveTo(2048);
  while (stepper.currentPosition() != 2048) {
    stepper.run();
  }
  delay(1000);

  // Giramos una vuelta completa en sentido antihorario
  Serial.println("counterclockwise");
  stepper.moveTo(0);
  while (stepper.currentPosition() != 0) {
    stepper.run();
  }
  delay(500);
}
