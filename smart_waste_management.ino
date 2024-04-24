#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Address for the LCD display
Servo servo; // Servo motor object

const int trigPin = 12; // Ultrasonic sensor trigger pin
const int echoPin = 11; // Ultrasonic sensor echo pin
const int soilMoisturePin = A0; // Soil moisture sensor analog pin

bool isWasteDetected = false; // Flag to track waste detection
bool isServoStable = true; // Flag to track servo stability
bool isFirstDetection = true; // Flag to track first waste detection
int wetWasteThreshold = 1000; // Threshold for wet waste detection
int lastMoistureLevel = 0; // Variable to store the last moisture level
int maxWasteHeight = 20; // Maximum waste height in centimeters

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Waste Segregator");
  delay(2000);
  lcd.clear();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  servo.attach(9);
  servo.write(90); // Set servo to initial position
}

void loop() {
  // Run waste detection only once
  if (isFirstDetection) {
    detectWaste();
    isFirstDetection = false;
  }

  // Continuous monitoring for changes
  monitorChanges();
}

void detectWaste() {
  lcd.setCursor(0, 0);
  lcd.print("Measuring...");

  int distance = measureDistance();
  int moistureLevel = measureMoisture();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.print(" cm");

  lcd.setCursor(0, 1);
  lcd.print("Moisture: ");
  lcd.print(moistureLevel);

  // Check for initial waste detection
  if (!isWasteDetected) {
    if (moistureLevel > wetWasteThreshold) {
      lcd.clear();
      lcd.print("Dry Waste Detected");
      segregateWaste(true); // Rotate servo clockwise for dry waste
      isWasteDetected = true;
    } else {
      lcd.clear();
      lcd.print("Wet Waste Detected");
      segregateWaste(false); // Rotate servo anticlockwise for wet waste
      isWasteDetected = true;
    }
  }

  // Check for waste height
  if (distance < maxWasteHeight) {
    lcd.setCursor(0, 2);
    lcd.print("Waste Full!");
  } else {
    lcd.setCursor(0, 2);
    lcd.print("Waste Height: ");
    lcd.print(distance);
    lcd.print(" cm");
  }

  delay(2000); // Delay after initial detection
}

void monitorChanges() {
  // Continuous monitoring loop
  while (true) {
    int moistureLevel = measureMoisture();

    // Check for significant changes in moisture level
    if (abs(moistureLevel - lastMoistureLevel) >= 50) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Moisture: ");
      lcd.print(moistureLevel);

      if (moistureLevel > wetWasteThreshold) {
        lcd.setCursor(0, 0);
        lcd.print("Dry Waste Detected");
        segregateWaste(true); // Rotate servo clockwise for dry waste
        isWasteDetected = true;
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Wet Waste Detected");
        segregateWaste(false); // Rotate servo anticlockwise for wet waste
        isWasteDetected = true;
      }

      lastMoistureLevel = moistureLevel; // Update last moisture level
      delay(2000); // Delay after detection
    }

    delay(1000); // Delay between monitoring cycles
  }
}

int measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;
  return distance;
}

int measureMoisture() {
  int moisture = analogRead(soilMoisturePin);
  return moisture;
}

void segregateWaste(bool isDryWaste) {
  if (isDryWaste) {
    servo.write(180); // Rotate servo clockwise for dry waste
  } else {
    servo.write(0); // Rotate servo anticlockwise for wet waste
  }
  delay(2000);
  servo.write(90); // Reset servo position
}