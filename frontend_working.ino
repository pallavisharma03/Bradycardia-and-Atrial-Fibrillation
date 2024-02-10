#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/* Insert your network credentials */
#define WIFI_SSID "My-ssid"
#define WIFI_PASSWORD "pallu2002"

/* Insert Firebase project API Key */
#define API_KEY "AIzaSyDZCs0th14A4GMHnLNRoItq9uGxXzcpkAk"

/* Define the RTDB URL */
#define DATABASE_URL "heart-c01c6-default-rtdb.firebaseio.com/"

#define TEMP_REPORTING_PERIOD_MS 1000
#define ONE_WIRE_BUS D4 // Define the pin connected to the DS18B20

/* Define Firebase Data object */
FirebaseData fbdo;

/* Define Firebase Authentication and Config objects */
FirebaseAuth auth;
FirebaseConfig config;

unsigned long lastTempReportTime = 0;
unsigned long sendDataPrevMillis = 0;
int count = 0;
int validReadingCount = 0;
bool signupOK = false;
float temperatureF;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

uint32_t tsLastReport = 0;

void readTemperatureSensor() {
    sensors.requestTemperatures(); // Request temperature readings
    temperatureF = sensors.getTempFByIndex(0); // Read temperature in F

    Serial.print("Temperature: ");
    Serial.print(temperatureF);
    Serial.println(" °F");
}

void setup() {
  Serial.begin(115200);

  /* Set WiFi mode to Station mode */
  WiFi.mode(WIFI_STA);

  Serial.println("Connecting to ");
  Serial.println(WIFI_SSID);

  /* Connect to your local Wi-Fi network */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  /* Wait for connection */
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  /* Assign API key and RTDB URL */
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign up successful");
    signupOK = true;
  } else {
    Serial.printf("Sign up failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  unsigned long currentMillis = millis();

  // Process incoming serial data
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    Serial.println("Received data: " + data);

    if (currentMillis - lastTempReportTime >= TEMP_REPORTING_PERIOD_MS) 
    {
      lastTempReportTime = currentMillis;
      readTemperatureSensor();
    }

    // Split the received data into Heart Rate and SpO2 values
    int separatorIndex = data.indexOf(',');
    if (separatorIndex != -1) {
      String heartRateStr = data.substring(0, separatorIndex);
      String spo2Str = data.substring(separatorIndex + 1);

      // Convert string values to floats
      float heartRate = heartRateStr.toFloat();
      float spo2 = spo2Str.toFloat();

      // Check if the reading is valid
      if (heartRate > 40 && spo2 > 80 && heartRate < 160 && spo2 < 110) {
        count++;
        validReadingCount++;

        // Send data to the Firebase Realtime Database
        if (Firebase.ready() && signupOK) {
          /* Write data to the database */
          if (Firebase.RTDB.setFloat(&fbdo, "Heart Rate/" + String(count), heartRate)) {
            Serial.println("Heart Rate data sent successfully");
          } else {
            Serial.println("Failed to send Heart Rate data");
            Serial.println("Error Reason: " + fbdo.errorReason());
          }

          if (Firebase.RTDB.setFloat(&fbdo, "SpO2 levels/" + String(count), spo2)) {
            Serial.println("SpO2 data sent successfully");
          } else {
            Serial.println("Failed to send SpO2 data");
            Serial.println("Error Reason: " + fbdo.errorReason());
          }

          if (Firebase.RTDB.setFloat(&fbdo, "Temperature/" + String(count), temperatureF)) {
            Serial.println("SpO2 data sent successfully");
          } else {
            Serial.println("Failed to send SpO2 data");
            Serial.println("Error Reason: " + fbdo.errorReason());
          }
          // Check if the required number of valid readings has been reached
          if (validReadingCount >= 20) {
            Serial.println("Reached 20 valid readings. Stopping further readings.");
            while (1) {
              // Infinite loop to stop further readings
            }
          }
        }
      }
    }
  }
  delay(1000);
}

