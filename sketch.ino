#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoHA.h>  

Servo myservo; 

int Valor_POT;
int Angulo;
int lastReportedAngle = -1; 

// WiFi credentials
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// MQTT broker settings (Home Assistant IP)
#define MQTT_BROKER "192.168.1.100" // Replace with your Home Assistant IP
#define MQTT_PORT 1883
#define MQTT_USERNAME "homeassistant" 
#define MQTT_PASSWORD "mqtt_password"

// Unique ID for this device
#define DEVICE_ID "esp32_servo"

WiFiClient wifiClient;
HADevice device(DEVICE_ID);
HAMqtt mqtt(wifiClient, device);

// Define a sensor to report the current position
HASensor servoPositionSensor("servo_position");

// Define a number entity for controlling the servo
HANumber servoControl("servo_control", HANumber::PrecisionP0);

void onServoCommand(HANumeric value, HANumber* sender) {
  int position = value.toInt8();
  
  // Constrain the value to servo range
  position = constrain(position, 0, 180);
  
  // Set the servo to the requested position
  myservo.write(position);
  
  // Report the new position back to Home Assistant
  servoPositionSensor.setValue(String(position).c_str());
  
  Serial.print("Set servo to position: ");
  Serial.println(position);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Irrigation system with Home Assistant MQTT");

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  // Set up Home Assistant device details
  device.setName("ESP32 Servo Controller");
  device.setSoftwareVersion("1.0.0");
  device.setManufacturer("DIY Project");
  device.setModel("ESP32 DevKit C v4");

  // Configure the servo position sensor
  servoPositionSensor.setName("Current Servo Position");
  servoPositionSensor.setUnitOfMeasurement("°");

  // Configure the servo control entity
  servoControl.setName("Set Servo Position");
  servoControl.setMin(0);
  servoControl.setMax(180);
  servoControl.setStep(1);
  servoControl.onCommand(onServoCommand);

  // Initialize MQTT connection to Home Assistant
  mqtt.begin(MQTT_BROKER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

  myservo.attach(18);
}

void loop() {
  mqtt.loop();  // Keep the MQTT connection alive
  
  // Read potentiometer and map to servo angle
  Valor_POT = analogRead(34);
  Angulo = map(Valor_POT, 0, 4095, 0, 180);
  
  // Update servo position
  myservo.write(Angulo);
  
  // Send potentiometer value to Home Assistant (only if changed)
  if (Angulo != lastReportedAngle) {
    servoPositionSensor.setValue(String(Angulo).c_str());
    lastReportedAngle = Angulo;
    Serial.println(Angulo);
  }
  
  delay(20);
}