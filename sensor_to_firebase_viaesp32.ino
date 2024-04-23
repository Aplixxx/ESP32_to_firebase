#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>


#define DHT_SENSOR_PIN 4
#define DHT_SENSOR_TYPE DHT11
int moisturePin = 34;

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define WIFI_SSID "Aranyak’s iPhone"
#define WIFI_PASSWORD "210497Bdk"

#define API_KEY "AIzaSyCVbBy_T-Cs78XlRQEbqn60AOPCyuKEn04"
#define DATABASE_URL "https://agrobot-832c9-default-rtdb.asia-southeast1.firebasedatabase.app/" 
int moistureState = 0;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false; 

void setup(){

  dht_sensor.begin();
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Connected to cloud!");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop(){
  int moistureValue = analogRead(moisturePin);
  int moisturePercentage = map(4095 - moistureValue, 0, 4095, 0, 100);

  float temperature = dht_sensor.readTemperature();
  float humidity = dht_sensor.readHumidity();

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.setInt(&fbdo, "DHT_11/Temperature/Data", temperature)){
      Serial.print("Temperature : ");
      Serial.println(temperature);
    }
    else {
      Serial.println("Failed to Read from the Sensor");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    
    // Enter Humidity in to the DHT_11 Table
    if (Firebase.RTDB.setFloat(&fbdo, "DHT_11/Humidity/Data", humidity)){
      Serial.print("Humidity : ");
      Serial.print(humidity);
    }
    else {
      Serial.println("Failed to Read from the Sensor");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "DHT_11/Soil_Moisture/Data", moistureValue)){
        Serial.print("Soil Moisture Value: ");
        Serial.println(moistureValue);
    }
    else {
      Serial.println("Failed to Read from the Sensor");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}