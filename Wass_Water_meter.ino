#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int entryTrigPin = 9;  // Entry ultrasonic sensor trigger pin (Sensor A)
const int entryEchoPin = 10; // Entry ultrasonic sensor echo pin (Sensor A)
const int exitTrigPin = 11;  // Exit ultrasonic sensor trigger pin (Sensor B)
const int exitEchoPin = 12;  // Exit ultrasonic sensor echo pin (Sensor B)
const int maxSlots = 6;      // Maximum parking slots available

Servo gateServo;
const int servoPin = 8;      // Servo motor pin

int currentCars = 0;         // Track the number of cars in the parking area
const int gateOpenAngle = 0; // Angle for gate open
const int gateCloseAngle = 90; // Angle for gate close

void setup() {
  Serial.begin(9600);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(2);      // Set font size to increase text size
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Smart Parking");
  display.setCursor(0, 20);
  display.println("System");
  display.display();
  delay(2000); // Display the title for 2 seconds

  // Display initial parking information
  updateDisplay();

  // Set pin modes for ultrasonic sensors
  pinMode(entryTrigPin, OUTPUT);
  pinMode(entryEchoPin, INPUT);
  pinMode(exitTrigPin, OUTPUT);
  pinMode(exitEchoPin, INPUT);

  // Attach the servo to the pin
  gateServo.attach(servoPin);
  gateServo.write(gateCloseAngle); // Ensure the gate is initially closed
  Serial.println("System Initialized");
}

void loop() {
  // Check if a car is entering
  if (detectCar(entryTrigPin, entryEchoPin)) {
    if (currentCars < maxSlots) {
      currentCars++; // Increment the car count for entry
      Serial.println("Car Entered - Slot Occupied");
      Serial.print("Parking Slots Available: ");
      Serial.println(maxSlots - currentCars);
      openGate();

      // Wait for the car to reach the exit sensor to close the gate
      while (!detectCar(exitTrigPin, exitEchoPin)) {
        // Waiting for the car to reach the exit sensor (Sensor B)
      }
      closeGate();
      updateDisplay(); // Update the OLED display after a car enters
    } else {
      Serial.println("Parking Full - No Entry Allowed");
    }
    delay(1000); // Short delay to prevent multiple detections
  }

  // Check if a car is exiting
  if (detectCar(exitTrigPin, exitEchoPin)) {
    if (currentCars > 0) {
      currentCars--; // Decrement the car count for exit
      Serial.println("Car Exited - Slot Vacant");
      Serial.print("Parking Slots Available: ");
      Serial.println(maxSlots - currentCars);
      openGate();

      // Wait for the car to reach the entry sensor to close the gate
      while (!detectCar(entryTrigPin, entryEchoPin)) {
        // Waiting for the car to reach the entry sensor (Sensor A)
      }
      closeGate();
      updateDisplay(); // Update the OLED display after a car exits
    }
    delay(1000); // Short delay to prevent multiple detections
  }
}

// Function to open the gate
void openGate() {
  gateServo.write(gateOpenAngle);
  Serial.println("Gate Opened");
}

// Function to close the gate
void closeGate() {
  gateServo.write(gateCloseAngle);
  Serial.println("Gate Closed");
}

// Function to detect a car using an ultrasonic sensor
bool detectCar(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2; // Calculate distance in cm
  
  if (distance > 0 && distance < 5) { // Threshold distance for detecting a car
    return true;
  }
  return false;
}

// Function to update the OLED display with parking information
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1); // Smaller text size for title
  display.setCursor(0, 0);
  display.println("Smart Parking System");

  display.setTextSize(1); // Smaller text size for slots info
  display.setCursor(0, 30);
  display.print("Total: ");
  display.println(maxSlots);

  display.setCursor(0, 50);
  display.print("Available: ");
  display.println(maxSlots - currentCars);
  display.display();
}
