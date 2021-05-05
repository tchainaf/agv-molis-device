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
#include <PubSubClient.h>

#define RST_PIN   0
#define SDA_PIN   4
#define LED_PIN   15
#define BAT_PIN   A0
#define TOPICO_PUBLISH   "<HELIX_TOPIC>"

MFRC522 mfrc522(SDA_PIN, RST_PIN);
WiFiClient espClient;
PubSubClient MQTT(espClient);

#define SSID  "<WIFI_SSID>" 
#define PASS  "<WIFI_PASSWORD>"

const char* brokerMQTT = "<HELIX_IP>";
int brokerPort = 1883;
const char* deviceID = "<HELIX_DEVICE_ID>";

std::vector <float> batLastMeasures;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(LED_PIN, OUTPUT);
  pinMode(BAT_PIN, INPUT);

  connectWifi();
  MQTT.setServer(brokerMQTT, brokerPort);
  connectMQTT();
}

void loop() {
  checkWifiAndMQTTConnection();
  readRFID();
  readVoltage();
    
  MQTT.loop();
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
  blinkLed();
  String id = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    id.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  id.trim();
  id.toUpperCase();
  Serial.println("UID da tag: " + id);
  sendLocationToHelix(id);
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  successLed();
  Serial.println("WiFi connected!");
}

void connectMQTT() {
  while (!MQTT.connected()) {
    if (MQTT.connect(deviceID)) {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      successLed();
    } 
    else {
      Serial.println("Falha ao conectar no broker.");
      delay(2000);
    }
  }
}

void checkWifiAndMQTTConnection() {
  if (!MQTT.connected())
    connectMQTT();

  if (WiFi.status() != WL_CONNECTED)
    connectWifi();
}

void sendBatteryToHelix(float voltage) {
  String data = "v|" + String(voltage);
  if (MQTT.publish(TOPICO_PUBLISH, data.c_str()))
  {
    Serial.println("Bateria enviada para o broker!");
    successLed();
  }
}

void sendLocationToHelix(String uid) {
  String data = "l|" + uid;
  if (MQTT.publish(TOPICO_PUBLISH, data.c_str()))
  {
    Serial.println("Localização enviada para o broker!");
    successLed();
  }
}

void blinkLed(){
  digitalWrite(LED_PIN, HIGH);
  delay(2000);
  digitalWrite(LED_PIN, LOW);
}

void successLed(){
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  delay(150);
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  delay(150);
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
}
