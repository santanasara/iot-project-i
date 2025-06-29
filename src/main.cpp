#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoHA.h>  
#include <DHT.h>

#define LED_PIN 18
#define DHTPIN 4
#define DHTTYPE DHT22

int Valor_POT;
const int HUMIDITY_THRESHOLD = 40.0; 
bool irrigationActive = false;       

DHT dht(DHTPIN, DHTTYPE);

// WiFi credentials
#define WIFI_SSID "Wokwi-GUEST" 

#define MQTT_BROKER "172.21.69.246" 
#define MQTT_PORT 1883

#define DEVICE_ID "esp32_simulation"

WiFiClient wifiClient;
HADevice device(DEVICE_ID);
HAMqtt mqtt(wifiClient, device);

HASensor lightSensor("light_level");
HABinarySensor irrigationSensor("irrigation_active");
HASensor dhtTempSensor("dht_temperature");
HASensor dhtHumSensor("dht_humidity");

void setup() {
  Serial.begin(115200);
  Serial.println("Irrigation system with Home Assistant MQTT");

  // Connect to WiFi
  WiFi.begin(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  device.setName("ESP32 - Sensores Inteligentes");
  device.setSoftwareVersion("1.0.0");
  device.setManufacturer("Sara Santana / Lhayana Vieira");
  device.setModel("ESP32");

  lightSensor.setName("Nível de luz");
  lightSensor.setUnitOfMeasurement("lux");

  irrigationSensor.setName("Sistema de irrigação");

  dhtTempSensor.setName("Temperatura DHT22");
  dhtTempSensor.setUnitOfMeasurement("°C");
  dhtHumSensor.setName("Umidade DHT22");
  dhtHumSensor.setUnitOfMeasurement("%");

  mqtt.begin(MQTT_BROKER, MQTT_PORT);
  Serial.println("CONNECTING TO MQTT BROKER");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  dht.begin();
}

void loop() {
  mqtt.loop(); 
  Valor_POT = analogRead(34);
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

