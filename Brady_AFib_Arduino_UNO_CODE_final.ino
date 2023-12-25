  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  #define USE_ARDUINO_INTERRUPTS true
  #include <PulseSensorPlayground.h>

  char command = '0';
  const int PulseWire = 0;
  const int LED13 = 13;
  int Threshold = 200;
  int buzzerPin = 8; // Define the pin for the buzzer
  int averageBPM = 0;
  int myBPM = 0;
  PulseSensorPlayground pulseSensor;

  LiquidCrystal_I2C lcd(0x27, 16, 2);

  bool sensorRunning = false;

  void setup() {
    Serial.begin(9600);
    pulseSensor.analogInput(PulseWire);
    pulseSensor.blinkOnPulse(LED13);
    pulseSensor.setThreshold(Threshold);

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Hello there!");
    pinMode(buzzerPin, OUTPUT); // Set the buzzer pin as an output
    
    if (pulseSensor.begin()) {
      Serial.println("Pulse sensor initialized!");
    }
  }

  void loop() {
    if (Serial.available() > 0) {
      char command = Serial.read();
      
      if (command == '1') {
        // Turn on the pulse sensor
        Serial.println("Pulse sensor turned ON");
        sensorRunning = true;
      } 
      else if (command == '0') {
        // Turn off the pulse sensor
        Serial.println("Pulse sensor turned OFF");
        sensorRunning = false;
      }
    }

    if (sensorRunning && myBPM < 200) {
      sensorRunning = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Welcome");
      lcd.setCursor(0, 1);
      lcd.print("To HeartFit");
      //lcd.clear();
      int validReadings = 0;
      int sum = 0;

      while (validReadings < 5) {
        int myBPM = pulseSensor.getBeatsPerMinute();

        if (pulseSensor.sawStartOfBeat() && myBPM < 200) {
          //Serial.println("♥ A HeartBeat Happened !");
          //Serial.print("BPM: ");
          //Serial.println(myBPM);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("HeartBeat felt!");
          lcd.setCursor(0, 1);
          lcd.print("Heart Rate: ");
          lcd.setCursor(12, 1);
          lcd.print(myBPM);
          delay(1000);
          sum += myBPM;
          validReadings++;
        }

        delay(1000);
      }

      int averageBPM = sum / 5;
      //Serial.print("Your BPM is: ");
      //Serial.println(averageBPM);
      Serial.println(averageBPM); // <-- Send averageBPM to ESP8266


      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Your BPM is       ");
      lcd.setCursor(13, 0);
      lcd.print(averageBPM);
      delay(5000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thank you for");
      lcd.setCursor(0, 1);
      lcd.print("using HeartFit");

      // Activate the buzzer
      tone(buzzerPin, 3200); // You can change the frequency as needed
      delay(1000); // Buzz for 1 second
      noTone(buzzerPin); // Turn off the buzzer
      // Other tones and delays...

      // To continue running even after 10 readings, set sensorRunning to false
      sensorRunning = false;
      command = '0';
    }
  }
