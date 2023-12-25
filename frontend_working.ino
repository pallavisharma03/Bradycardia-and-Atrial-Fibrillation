#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>


int receivedBPM = 0; // Variable to store the average value
String htmlContent = "";
int sensorReadings[10]; // Array to store sensor readings
int sensorIndex = 0; // Index to keep track of readings in the array

const char* ssid = "My-ssid";
const char* password = "pallu2002";

SoftwareSerial arduinoSerial(D5, D6); // RX, TX pins on NodeMCU connected to TX, RX pins on Arduino Uno
ESP8266WebServer server(80);

bool sensorRunning = false; // Flag to track sensor status

void startSensor() {
  arduinoSerial.write('1'); // Command to start pulse sensor on Arduino Uno
  sensorRunning = true; // Update sensor status
  handleRoot(); // Show the root page with updated sensor status
}

void stopSensor() {
  arduinoSerial.write('0'); // Command to stop pulse sensor on Arduino Uno
  sensorRunning = false; // Update sensor status
  handleRoot(); // Show the root page with updated sensor status
}

void handleBPM() {
  server.send(200, "text/plain", String(receivedBPM));
}

void handleRoot() {
  htmlContent = R"=====(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <title>HeartFit</title>
      <style>
        /* Body styles */
        body {
          font-family: Arial, sans-serif;
          margin: 0;
          padding: 20px;
          background-color: #ffe6e6; /* Light pink background */
          text-align: center; /* Center align content */
        }

        /* Header styles */
        h1 {
          font-size: 36px;
          color: #800000; /* Maroon color for header */
          margin-bottom: 30px;
        }

        /* Form and container styles */
        #container {
          max-width: 600px;
          margin: 0 auto;
          background-color: #ffffff; /* White background for form container */
          padding: 20px;
          border-radius: 8px;
          box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
          text-align: left; /* Align form elements to the left */
        }

        label {
          display: block;
          margin-bottom: 8px;
          font-weight: bold;
        }

        input[type="number"],
        input[type="text"],
        select,
        input[type="submit"],
        button {
          width: calc(100% - 18px);
          padding: 8px;
          margin-bottom: 15px;
          border-radius: 4px;
          border: 1px solid #ccc;
          box-sizing: border-box; /* Include padding and border in width calculation */
        }

        input[type="submit"],
        button {
          border: none;
          background-color: #800000; /* Maroon color for buttons */
          color: #fff; /* White text color */
          cursor: pointer;
          transition: background-color 0.3s;
        }

        input[type="submit"]:hover,
        button:hover {
          background-color: #5a0000; /* Darker maroon on hover */
        }
      </style>
    </head>
    <body>
      <h1>HeartFit</h1>

      <div id="container">
        <!-- Pulse Sensor Controls -->
        <div id="sensorControls">
          <h2>Pulse Sensor</h2>
          <form action='/start'><button>Start Sensor</button></form>
          <script>
            document.addEventListener('DOMContentLoaded', () => {
              const bpmElement = document.getElementById('sensorValue');
              if (bpmElement) {
                const newDiv = document.createElement('div');
                //newDiv.innerText = 'Calculating...'; // Initial value, can be updated dynamically
                bpmElement.parentNode.appendChild(newDiv);
                setInterval(fetchBPM, 1);
              }
            });
          </script>
          <div id="sensorValue" style="text-align: center;"> </div>
        </div>

        <!-- BMI Calculator -->
        <form id="bmiForm">
          <h2>Calculate BMI</h2>
          <label for="height">Height (cm):</label>
          <input type="number" id="height" name="height" min="0">

          <label for="weight">Weight (kg):</label>
          <input type="number" id="weight" name="weight" min="0">

          <button type="button" onclick="calculateBMI()">Calculate BMI</button>
        </form>

        <!-- Display BMI Result -->
        <div id="bmiResult" style="text-align: center;"></div>

        <!-- User Information -->
        <form id="userInfoForm" style="display: none;">
          <h2>User Information</h2>
          <label for="age">Age:</label>
          <input type="number" id="age" name="age" min="0">

          <label for="gender">Gender:</label>
          <select id="gender" name="gender">
            <option value="male">Male</option>
            <option value="female">Female</option>
          </select>

          <label for="smoking">Smoking Habits:</label>
          <select id="smoking" name="smoking">
            <option value="yes">Yes</option>
            <option value="no">No</option>
          </select>

          <label for="alcohol">Alcohol Consumption:</label>
          <select id="alcohol" name="alcohol">
            <option value="yes">Yes</option>
            <option value="no">No</option>
          </select>

          <label for="caffeine">Caffeine Consumption:</label>
          <select id="caffeine" name="caffeine">
            <option value="yes">Yes</option>
            <option value="no">No</option>
          </select>

          <label for="activity">Physical Activity:</label>
          <select id="activity" name="activity">
            <option value="yes">Yes</option>
            <option value="no">No</option>
          </select>

          <button type="button" onclick="sendData()">Submit</button>
        </form>
      </div>

      <script>
        function calculateBMI() {
          const height = document.getElementById('height').value;
          const weight = document.getElementById('weight').value;
          const bmiResult = document.getElementById('bmiResult');

          if (height && weight) {
            const bmi = (weight / Math.pow(height, 2) * 10000).toFixed(2);
            bmiResult.innerHTML = "<p>Your BMI is " + bmi + "</p>";
            document.getElementById('userInfoForm').style.display = 'block';
          } else {
            bmiResult.innerHTML = "<p>Please provide both height and weight.</p>";
            document.getElementById('userInfoForm').style.display = 'none';
          }
        }

        // Function to fetch BPM from server
        function fetchBPM() {
          fetch('/bpm')
            .then(response => response.text())
            .then(data => {
                 if (parseInt(data) !== 0) {
                  const bpmElement = document.getElementById('sensorValue');
                  bpmElement.innerText = 'Your BPM: ' + data;
                }
            });
        }

        function sendData() {
          // Gather data
          var age = document.getElementById('age').value;
          var gender = document.getElementById('gender').value === 'male' ? 1 : 0;
          var smoking = document.getElementById('smoking').value === 'yes' ? 1 : 0;
          var alcohol = document.getElementById('alcohol').value === 'yes' ? 1 : 0;
          var caffeine = document.getElementById('caffeine').value === 'yes' ? 1 : 0;
          var activity = document.getElementById('activity').value === 'yes' ? 1 : 0;

          var bmi = calculateBMI();  // calculate BMI
          var bpm = readBPM();  // read BPM

          // Create data object
          var data = {
            age: age,
            gender: gender,
            smoking: smoking,
            alcohol: alcohol,
            caffeine: caffeine,
            activity: activity,
            bmi: bmi,
            bpm: bpm
            };

          // Send POST request to Flask server
          fetch('http://127.0.0.1:5000/predict', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json'
            },
            body: JSON.stringify(data)
          })
            .then(response => response.json())
            .then(data => {
              // Handle response from server
              console.log(data);
            })
            .catch((error) => {
              console.error('Error:', error);
            });
        }

      </script>
    </body>
    </html>
  )=====";

server.send(200, "text/html", htmlContent);
}

void setup() {
  pinMode(D5, INPUT); // RX pin on NodeMCU
  pinMode(D6, OUTPUT); // TX pin on NodeMCU
  pinMode(A0, INPUT); // <-- Added pin mode for A0

  Serial.begin(115200);
  arduinoSerial.begin(9600); // Start serial communication with Arduino Uno
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");  
  }

  Serial.println("Connected to WiFi");
  Serial.print("NodeMCU IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/start", HTTP_GET, startSensor);
  server.on("/stop", HTTP_GET, stopSensor);
  server.on("/bpm", HTTP_GET, handleBPM); // <-- Added new route to handle BPM
  server.begin();
}

void loop() {
  server.handleClient();

  if (sensorRunning && arduinoSerial.available() > 0) {
    receivedBPM = arduinoSerial.parseInt(); // <-- Update receivedBPM instead of calling updateBPM
    handleBPM();
  }
}

