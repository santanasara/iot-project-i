#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoHA.h>  

Servo myservo; 

int Valor_POT;
int Angulo;
int lastReportedAngle = -1; 
const int HUMIDITY_THRESHOLD = 1500; // Threshold for "dry soil" (0-4095 range)

// WiFi credentials
#define WIFI_SSID "Wokwi-GUEST" // Replace with your WiFi SSID

// MQTT broker settings (Home Assistant IP)
#define MQTT_BROKER "" // Replace with your Home Assistant IP
#define MQTT_PORT 1883

#define DEVICE_ID "esp32_servo"

WiFiClient wifiClient;
HADevice device(DEVICE_ID);
HAMqtt mqtt(wifiClient, device);

HASensor servoPositionSensor("servo_position");

HANumber servoControl("servo_control", HANumber::PrecisionP0);

void onServoCommand(HANumeric value, HANumber* sender) {
  int position = value.toInt8();
  
  position = constrain(position, 0, 180);
  
  myservo.write(position);
  
  servoPositionSensor.setValue(String(position).c_str());
  
  Serial.print("Mudar posição do Servo: ");
  Serial.println(position);
}

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

  device.setName("ESP32 Simulador de Sistema de Irrigação");
  device.setSoftwareVersion("1.0.0");
  device.setManufacturer("Sara Santana / Lhayana Vieira");
  device.setModel("ESP32");

  servoPositionSensor.setName("Posição atual do Servo");
  servoPositionSensor.setUnitOfMeasurement("°");

  servoControl.setName("Setar posição do Servo");
  servoControl.setMin(0);
  servoControl.setMax(180);
  servoControl.setStep(1);
  servoControl.onCommand(onServoCommand);

  mqtt.begin(MQTT_BROKER, MQTT_PORT);
  Serial.println("CONNECTING TO MQTT BROKER");

  myservo.attach(18);
}

void loop() {
  mqtt.loop(); 
  
  Valor_POT = analogRead(34);
  
  // Check if too dry
  if (Valor_POT < HUMIDITY_THRESHOLD) {
    Angulo = 180;
    myservo.write(Angulo);
  } else {
    Angulo = 0;
    myservo.write(Angulo);
  }
  
  if (Angulo != lastReportedAngle) {
    servoPositionSensor.setValue(String(Angulo).c_str());
    lastReportedAngle = Angulo;
    Serial.println(Angulo);
  }
  
  delay(20);
}