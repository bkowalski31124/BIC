// Finalna wersja: RFID + MQTT + otwieranie zamka (bez SSL)
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Piny dla ESP32 i RC522
#define RST_PIN     5   // GPIO5 dla RST czytnika
#define SS_PIN      15  // GPIO15 dla SDA (SS)
#define ZAMEK_PIN   16  // GPIO16 dla przekaźnika

// Dane WiFi i MQTT
const char* ssid = "Nazwa WiFi";
const char* password = "Haslo WiFi";
const char* mqtt_server = "IP brokera MQTT"; // lokalny broker MQTT
const int mqtt_port = 1883; // zwykly port bez SSL

// UID karty z dostępem (zamień na swój!)
byte docelowyUID[] = {0x2E, 0x76, 0xC8, 0x01}; 

MFRC522 rfid(SS_PIN, RST_PIN);
WiFiClient espClient;
PubSubClient client(espClient);

void connectWiFi() {
  Serial.print("Laczenie z WiFi: ");
  WiFi.begin(ssid, password);
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nPolaczono z WiFi! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nBLAD: Brak polaczenia z WiFi!");
  }
}

void connectMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  Serial.print("Laczenie z MQTT...");
  if (client.connect("ESP32_RFID")) {
    Serial.println("Polaczono z MQTT!");
    client.publish("lock/status", "ESP podlaczony do MQTT");
  } else {
    Serial.print("Blad MQTT: ");
    Serial.println(client.state());
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, SS_PIN);
  rfid.PCD_Init();
  pinMode(ZAMEK_PIN, OUTPUT);
  digitalWrite(ZAMEK_PIN, LOW);

  connectWiFi();
  connectMQTT();
  Serial.println("START: System gotowy");
}

void loop() {
  if (!client.connected()) connectMQTT();
  client.loop();

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = getUIDString();
  Serial.print("Odczytano UID: ");
  Serial.println(uid);

  if (czyUIDPasuje(rfid.uid.uidByte, rfid.uid.size, docelowyUID, sizeof(docelowyUID))) {
    grantAccess(uid);
  } else {
    denyAccess(uid);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

String getUIDString() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  return uid;
}

bool czyUIDPasuje(byte *odczytanyUID, byte rozmiarOdczytanego, byte *docelowyUID, byte rozmiarDocelowego) {
  if (rozmiarOdczytanego != rozmiarDocelowego) return false;
  for (byte i = 0; i < rozmiarOdczytanego; i++) {
    if (odczytanyUID[i] != docelowyUID[i]) return false;
  }
  return true;
}

void grantAccess(String uid) {
  client.publish("lock/grant", uid.c_str());
  digitalWrite(ZAMEK_PIN, HIGH);
  Serial.println("Dostep przyznany! Zamek otwarty na 3 sekundy.");
  delay(3000);
  digitalWrite(ZAMEK_PIN, LOW);
}

void denyAccess(String uid) {
  client.publish("lock/deny", uid.c_str());
  Serial.println("Brak dostepu! Nieprawidlowa karta.");
  delay(1000);
}