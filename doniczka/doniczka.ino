#include "DHT.h"

// --- DHT11 ---
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- LDR ---
#define LDR_PIN A0
#define DARK_THRESHOLD 300  // dla diody, jeśli chcesz

// --- Czujnik wilgotności gleby ---
#define SOIL_PIN A1
const int SOIL_DRY = 700;
const int SOIL_WET = 500;

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(13, OUTPUT); // wbudowana dioda LED
}

void loop() {
  // --- DHT11 ---
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Błąd odczytu z DHT11!");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.print(" °C, Wilgotność powietrza: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  // --- LDR ---
  int rawLight = analogRead(LDR_PIN);
  int lightValue = 1023 - rawLight;

  Serial.print("Poziom światła (odwrócony): ");
  Serial.println(lightValue);

  // Reakcja LED jeśli ciemno
  if (lightValue < DARK_THRESHOLD) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }

  // --- Czujnik wilgotności gleby ---
  int soilRaw = analogRead(SOIL_PIN);
  int moisturePercent = map(soilRaw, SOIL_DRY, SOIL_WET, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Wilgotność gleby: ");
  Serial.print(moisturePercent);
  Serial.println(" %");

  Serial.println("-------------------------");

  delay(3600);
}