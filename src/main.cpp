#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoHA.h>
#include <DHT.h>

// --- Pinos ---
#define LED_PIN 18             // LED verde da irrigação
#define RED_LED_PIN 23         // NOVO: LED vermelho para baixa luz
#define DHTPIN 4
#define DHTTYPE DHT22

// --- Variáveis Globais ---
int Valor_POT;
const int HUMIDITY_THRESHOLD = 40;
const int LOW_LIGHT_THRESHOLD = 150; // NOVO: Limite para acender o LED vermelho
bool irrigationActive = false;

DHT dht(DHTPIN, DHTTYPE);

// --- Configurações da Rede ---
#define WIFI_SSID "Wokwi-GUEST"
#define MQTT_BROKER "192.168.3.16" // Verifique se este IP ainda é o seu
#define MQTT_PORT 1883
#define DEVICE_ID "esp32_simulation"

WiFiClient wifiClient;
HADevice device(DEVICE_ID);
HAMqtt mqtt(wifiClient, device);

// --- Entidades para o Home Assistant ---
HASensor lightSensor("light_level");
HABinarySensor irrigationSensor("irrigation_active");
HASensor dhtTempSensor("dht_temperature");
HASensor dhtHumSensor("dht_humidity");

void setup() {
  Serial.begin(115200);
  Serial.println("Sistema de Irrigação Inteligente v3.0");

  // Configura os pinos
  pinMode(LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT); // NOVO: Configura o pino do LED vermelho
  digitalWrite(LED_PIN, LOW);
  dht.begin();

  // Conecta ao WiFi
  WiFi.begin(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao WiFi!");
  Serial.println(WiFi.localIP());

  // Configuração do dispositivo no Home Assistant
  device.setName("ESP32 - Sensores Inteligentes");
  device.setSoftwareVersion("3.0.0");
  device.setManufacturer("Sara Santana / Lhayana Vieira");
  device.setModel("ESP32-Pro");

  // Configuração das entidades
  lightSensor.setName("Nível de luz");
  lightSensor.setUnitOfMeasurement("lux");

  irrigationSensor.setName("Sistema de irrigação");

  dhtTempSensor.setName("Temperatura DHT22");
  dhtTempSensor.setUnitOfMeasurement("°C");
  dhtHumSensor.setName("Umidade DHT22");
  dhtHumSensor.setUnitOfMeasurement("%");

  mqtt.begin(MQTT_BROKER, MQTT_PORT);
  Serial.println("Conectando ao MQTT Broker...");
}

void loop() {
  mqtt.loop();

  // --- Leitura dos Sensores ---
  Valor_POT = analogRead(34); // LDR está no pino 34
  int lightLevel = map(Valor_POT, 0, 4095, 0, 1000);
  lightSensor.setValue(String(lightLevel).c_str());

  float dhtHumidity = dht.readHumidity();
  float dhtTemp = dht.readTemperature();
  if (!isnan(dhtHumidity)) {
    dhtHumSensor.setValue(String(dhtHumidity, 1).c_str());
  }
  if (!isnan(dhtTemp)) {
    dhtTempSensor.setValue(String(dhtTemp, 1).c_str());
  }

  // NOVO: Lógica para o LED vermelho de baixa luminosidade
  if (lightLevel < LOW_LIGHT_THRESHOLD) {
    digitalWrite(RED_LED_PIN, HIGH); // Acende o LED vermelho se estiver escuro
  } else {
    digitalWrite(RED_LED_PIN, LOW); // Apaga o LED vermelho se estiver claro
  }

  // --- Lógica de Irrigação (LED Verde) ---
  if (!isnan(dhtHumidity) && dhtHumidity < HUMIDITY_THRESHOLD && !irrigationActive) {
    digitalWrite(LED_PIN, HIGH);
    irrigationActive = true;
    irrigationSensor.setState(true);
    Serial.println("Umidade Baixa: Irrigação Ativada");
  }
  else if (!isnan(dhtHumidity) && dhtHumidity >= HUMIDITY_THRESHOLD && irrigationActive) {
    digitalWrite(LED_PIN, LOW);
    irrigationActive = false;
    irrigationSensor.setState(false);
    Serial.println("Umidade Suficiente: Irrigação Desativada");
  }

  delay(1000);
}