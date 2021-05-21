/**
               MFRC522      NodeMCU
   Signal      Pin          Pin
   -----------------------------------------
   RST/Reset   RST          D3 / GPIO0
   SPI SS      SDA(SS)      D2 / GPIO4
   SPI MOSI    MOSI         D7 / GPIO13
   SPI MISO    MISO         D6 / GPIO12
   SPI SCK     SCK          D5 / GPIO14

   CONNECTION STATUS LED    D8 / GPIO15
   BATTERY ALARM LED        D1 / GPIO5

*/

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define RST_PIN   0
#define SDA_PIN   4
#define BAT_PIN   A0
#define CONN_PIN  15
#define ALARM_PIN 5
#define PUBLISH_TOPIC   "<HELIX_PUBLISH_TOPIC>"
#define SUBSCRIBE_TOPIC "<HELIX_SUBSCRIBE_TOPIC>"
#define BATTERY_DELAY 60000 //1 minuto

MFRC522 mfrc522(SDA_PIN, RST_PIN);
WiFiClient espClient;
PubSubClient MQTT(espClient);

#define SSID  "<WIFI_SSID>"
#define PASS  "<WIFI_PASSWORD>"

const char* brokerMQTT = "<HELIX_IP>";
int brokerPort = 1883;
const char* deviceID = "<HELIX_DEVICE_ID>";

std::vector <float> batLastMeasures;
float batLastSent = 0;
unsigned long lastSentMillis = millis();

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(CONN_PIN, OUTPUT);
  pinMode(ALARM_PIN, OUTPUT);
  pinMode(BAT_PIN, INPUT);

  blinkLed();
  digitalWrite(CONN_PIN, LOW);
  digitalWrite(ALARM_PIN, LOW);

  connectWifi();
  MQTT.setServer(brokerMQTT, brokerPort);
  MQTT.setCallback(mqttCallback);
  connectMQTT();
}

void loop() {
  checkWifiAndMQTTConnection();
  readRFID();
  readVoltage();
  checkTime();

  MQTT.loop();
}

void readVoltage () {
  float sensorValue = analogRead(BAT_PIN);
  if (sensorValue < 10) { //Desconsidera leituras muito baixas
    Serial.println("Valor baixo");
    return;
  }

  float inputVoltage = (sensorValue * 3.2) / 1023;
  float measurement = inputVoltage / 0.2; //Cálculo com base nos resistores internos do sensor 7500 / (30000 + 7500) = 0.2

  Serial.print(measurement);
  Serial.println("V");

  batLastMeasures.push_back(measurement);
  if (batLastMeasures.size() == 10) {
    float sum = 0;
    for (int i = 0; i < 10; i++)
      sum += batLastMeasures[i];

    float avg = sum / 10;
    Serial.print("Média: ");
    Serial.println(avg);

    if (avg != batLastSent)
      batLastSent = avg;

    batLastMeasures.erase(batLastMeasures.begin());
  }
}

void readRFID()
{
  Serial.println("Aproxime o cartão");

  int count = 0;
  while (!mfrc522.PICC_IsNewCardPresent()) {
    delay(100); //Aguarda cartão
    count++;

    if (count == 20) return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Não foi possível ler o cartão!");
    return;
  }

  //Lê ID da tag
  String id = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    id.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  id.trim();
  id.toUpperCase();
  Serial.println("UID da tag: " + id);
  sendLocationToHelix(id);
}

void connectWifi() {
  digitalWrite(CONN_PIN, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  blinkLed();
  Serial.println("WiFi connected!");
}

void connectMQTT() {
  digitalWrite(CONN_PIN, LOW);
  while (!MQTT.connected()) {
    if (MQTT.connect(deviceID)) {
      MQTT.subscribe(SUBSCRIBE_TOPIC);
      digitalWrite(CONN_PIN, HIGH);
      Serial.println("Conectado com sucesso ao broker MQTT!");
    }
    else {
      Serial.println("Falha ao conectar no broker.");
      delay(2000);
    }
  }
}

void checkWifiAndMQTTConnection() {
  if (WiFi.status() != WL_CONNECTED)
    connectWifi();

  if (!MQTT.connected())
    connectMQTT();
}

void sendBatteryToHelix(float voltage) {
  String data = "v|" + String(voltage);
  if (MQTT.publish(PUBLISH_TOPIC, data.c_str())) {
    blinkLed();
    Serial.println("Bateria enviada para o broker!");
  }
}

void sendLocationToHelix(String uid) {
  String data = "l|" + uid;
  if (MQTT.publish(PUBLISH_TOPIC, data.c_str())) {
    blinkLed();
    Serial.println("Localização enviada para o broker!");
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  Serial.print("Mensagem recebida! ");

  //Obtém a mensagem do payload recebido
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }

  Serial.println(msg);

  if (msg.equals("agv-molis001@on|")) {
    digitalWrite(ALARM_PIN, HIGH);
    if (MQTT.publish(PUBLISH_TOPIC, "a|on")) {
      blinkLed();
      Serial.println("Status do alarme enviado para o broker!");
    }
  }
  else if (msg.equals("agv-molis001@off|")) {
    digitalWrite(ALARM_PIN, LOW);
    if (MQTT.publish(PUBLISH_TOPIC, "a|off")) {
      blinkLed();
      Serial.println("Status do alarme enviado para o broker!");
    }
  }
}

void blinkLed() {
  digitalWrite(CONN_PIN, HIGH);
  delay(200);
  digitalWrite(CONN_PIN, LOW);
  delay(150);
  digitalWrite(CONN_PIN, HIGH);
  delay(200);
  digitalWrite(CONN_PIN, LOW);
  delay(150);
  digitalWrite(CONN_PIN, HIGH);
}

void checkTime() {
  if ((millis() - lastSentMillis) >= BATTERY_DELAY) {
    sendBatteryToHelix(batLastSent);
    lastSentMillis = millis();
  }
}
