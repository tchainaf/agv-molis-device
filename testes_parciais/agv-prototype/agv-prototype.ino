#include <HCSR04.h>
#include <Servo.h>

//Sensor óptico
#define IRL_PIN A0
#define IRR_PIN A1

//Sensor ultrassônico
#define TRIG_PIN 3
#define ECHO_PIN 2

//Servo motor
#define MT2_PIN 7 //Direção 0 ou 180
#define MT1_PIN 6

//Ponte H - L393D
#define EN2_PIN 5 //Velocidade 0 a 255
#define EN1_PIN 4

Servo mt1;
Servo mt2;
HCSR04 hc(TRIG_PIN,ECHO_PIN);

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

  if(distance < 5)
    stopMoving();
  else if (vlrIRL < 90 && vlrIRR < 90)
    moveFoward();
  else if(vlrIRL < 90)
    turnRight();
  else if (vlrIRR < 90)
    turnLeft();
  else
    stopMoving();
}

void moveFoward()
{
  mt1.write(0);
  mt2.write(180);
  analogWrite(EN1_PIN, 200);
  analogWrite(EN2_PIN, 200);
  Serial.println("Frente -------------------------------------------");
  delay(100);
}

void turnRight()
{
  mt1.write(0);
  mt2.write(180);
  analogWrite(EN1_PIN, 150);
  analogWrite(EN2_PIN, 200);
  Serial.println("Direita >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  delay(25);
}

void turnLeft()
{
  mt1.write(0);
  mt2.write(180);
  analogWrite(EN1_PIN, 200);
  analogWrite(EN2_PIN, 150);
  Serial.println("Esquerda <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  delay(25);
}

void stopMoving()
{  
  analogWrite(EN1_PIN, 0);
  analogWrite(EN2_PIN, 0);
  Serial.println("Parado xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
  delay(500);
}
