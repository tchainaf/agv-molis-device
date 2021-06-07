#include <HCSR04.h>
#include <Servo.h>

//Sensor óptico
#define IRL_PIN A0
#define IRR_PIN A1

//Sensor ultrassônico
#define ECHO_PIN 2
#define TRIG_PIN 3

//CI L293D
#define EN1_PIN 4 //Velocidade 0 a 255
#define EN2_PIN 5

//Servo motor
#define MT1_PIN 6 //Direção 0 ou 180
#define MT2_PIN 7

Servo mt1;
Servo mt2;
HCSR04 hc(TRIG_PIN, ECHO_PIN);

void setup () {
  Serial.begin(9600);
  mt1.attach(MT1_PIN);
  mt2.attach(MT2_PIN);

  pinMode(IRL_PIN, INPUT);
  pinMode(IRR_PIN, INPUT);
  pinMode(EN1_PIN, OUTPUT);
  pinMode(EN2_PIN, OUTPUT);
}

void loop () {
  double distance = hc.dist();
  Serial.print(distance);
  Serial.println(" cm");

  double vlrIRL = analogRead(IRL_PIN);
  double vlrIRR = analogRead(IRR_PIN);
  Serial.print("IR esquerda: ");
  Serial.println(vlrIRL);
  Serial.print("IR direita: ");
  Serial.println(vlrIRR);

  if (distance < 10)
    stopMoving(true);
  else if (vlrIRL < 90 && vlrIRR < 90)
    moveFoward();
  else if (vlrIRL < 90)
    turnRight();
  else if (vlrIRR < 90)
    turnLeft();
  else
    stopMoving(false);
}

void moveFoward()
{
  mt1.write(0);
  mt2.write(180);
  analogWrite(EN1_PIN, 255);
  analogWrite(EN2_PIN, 255);
  Serial.println("Frente -------------------------------------------");
  delay(50);
}

void turnRight()
{
  mt1.write(0);
  mt2.write(180);
  analogWrite(EN1_PIN, 0);
  analogWrite(EN2_PIN, 255);
  Serial.println("Direita >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  delay(5);
}

void turnLeft()
{
  mt1.write(0);
  mt2.write(180);
  analogWrite(EN1_PIN, 255);
  analogWrite(EN2_PIN, 0);
  Serial.println("Esquerda <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  delay(5);
}

void stopMoving(boolean isDistance)
{
  if (isDistance) {
    analogWrite(EN1_PIN, 0);
    analogWrite(EN2_PIN, 0);
    Serial.println("Parado xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    delay(100);
  }
  else {
    analogWrite(EN1_PIN, 0);
    analogWrite(EN2_PIN, 0);
    Serial.println("Parado xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    delay(1000);
    mt1.write(0);
    mt2.write(180);
    analogWrite(EN1_PIN, 255);
    analogWrite(EN2_PIN, 255);
    Serial.println("Frente -------------------------------------------");
    delay(300);
  }
}
