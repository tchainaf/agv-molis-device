/**
 *             MFRC522      NodeMCU
 * Signal      Pin          Pin
 * -----------------------------------------
 * RST/Reset   RST          D3 / GPIO0
 * SPI SS      SDA(SS)      D2 / GPIO4
 * SPI MOSI    MOSI         D7 / GPIO13
 * SPI MISO    MISO         D6 / GPIO12
 * SPI SCK     SCK          D5 / GPIO14
 */

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>

#define RST_PIN   0
#define SDA_PIN   4
#define BAT_PIN   A0

MFRC522 mfrc522(SDA_PIN, RST_PIN);

std::vector <float> batLastMeasures;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(BAT_PIN, INPUT);
}

void loop() {
  readRFID();
  readVoltage();
}

void readVoltage () {
  float sensorValue = analogRead(BAT_PIN);
  if (sensorValue < 10) return; //Desconsidera leituras muito baixas

  float inputVoltage = (sensorValue * 3.2) / 1023;
  float measurement = inputVoltage / 0.2; //Cálculo com base nos resistores internos do sensor 7500 / (30000 + 7500) = 0.2
  
  Serial.print(measurement);
  Serial.println("V");
  
  batLastMeasures.push_back(measurement);
  if (batLastMeasures.size() == 10)
  {
    float sum = 0;
    for(int i = 0; i < 10; i++)
      sum += batLastMeasures[i];

    Serial.print("Média: ");
    Serial.println(sum/10);
    batLastMeasures.erase(batLastMeasures.begin());
  }
}

void readRFID()
{
  Serial.println("Aproxime o cartão");

  int count = 0;
  while (!mfrc522.PICC_IsNewCardPresent())
  {
    delay(100); //Aguarda cartão
    count++;
    
    if(count == 20) return;
  }
  if (!mfrc522.PICC_ReadCardSerial())
  {
    Serial.println("Não foi possível ler o cartão!");
    return;
  }

  //Lê ID da tag
  String id = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    id.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  id.trim();
  id.toUpperCase();
  Serial.println("UID da tag: " + id);
}
