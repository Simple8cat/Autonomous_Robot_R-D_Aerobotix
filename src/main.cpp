#include <Arduino.h>
#include <WiFi.h>
#include "odometry.h"
#include "web.h"
// defining motor pins

#define Motor1A 16
#define Motor1B 17
#define Motor2A 18
#define Motor2B 19
#define pwm1 25
#define pwm2 26
#define StbyPin 27

// defining encoders pins

#define Encoder1A 35
#define Encoder1B 34
#define Encoder2A 32
#define Encoder2B 13

// defining interupts callback functions
void print_data(void *parameter)
{
  while (1)
  {
    Serial.print("X: ");
    Serial.print(current_pose.x);
    Serial.print(" Y: ");
    Serial.print(current_pose.y);
    Serial.print(" Theta: ");
    Serial.println(current_pose.theta * 180.0 / M_PI); // Convert to degrees
    vTaskDelay(100 / portTICK_PERIOD_MS);              // Print every 500 ms
  }
}
void IRAM_ATTR handleEncoder1()
{
  // Read both signals to determine direction
  if (digitalRead(Encoder1A) == digitalRead(Encoder1B))
    delta_left++; // CW
  else
    delta_left--; // CCW
}
void IRAM_ATTR handleEncoder2()
{
  // Read both signals to determine direction
  if (digitalRead(Encoder2A) == digitalRead(Encoder2B))
    delta_right++; // CW
  else
    delta_right--; // CCW
}

// defining motor control functions

void motor1Forward(int speed)
{
  digitalWrite(Motor1A, HIGH);
  digitalWrite(Motor1B, LOW);
  analogWrite(pwm1, speed);
}
void motor1Backward(int speed)
{
  digitalWrite(Motor1A, LOW);
  digitalWrite(Motor1B, HIGH);
  analogWrite(pwm1, speed);
}
void motor2Forward(int speed)
{
  digitalWrite(Motor2A, HIGH);
  digitalWrite(Motor2B, LOW);
  analogWrite(pwm2, speed);
}
void motor2Backward(int speed)
{
  digitalWrite(Motor2A, LOW);
  digitalWrite(Motor2B, HIGH);
  analogWrite(pwm2, speed);
}
void motorStop()
{
  digitalWrite(StbyPin, LOW); // Disable motors
}
// put function declarations here:
void init_pins()
{
  pinMode(Motor1A, OUTPUT);
  pinMode(Motor1B, OUTPUT);
  pinMode(Motor2A, OUTPUT);
  pinMode(Motor2B, OUTPUT);
  pinMode(pwm1, OUTPUT);
  pinMode(pwm2, OUTPUT);
  pinMode(StbyPin, OUTPUT);
  pinMode(Encoder1A, INPUT_PULLUP);
  pinMode(Encoder1B, INPUT_PULLUP);
  pinMode(Encoder2A, INPUT_PULLUP);
  pinMode(Encoder2B, INPUT_PULLUP);
  digitalWrite(StbyPin, HIGH); // Enable motors
  attachInterrupt(digitalPinToInterrupt(Encoder1A), handleEncoder1, RISING);
  attachInterrupt(digitalPinToInterrupt(Encoder2A), handleEncoder2, RISING);
}
void init_web()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();
  Serial.println("WebSocket server ready!");
}
void setup()
{

  Serial.begin(115200);
  init_web();
  init_pins();

  xTaskCreatePinnedToCore(
      update_pose,     // Task function
      "odometry_task", // Name
      4096,            // Stack
      NULL,            // Task parameter
      5,               // Priority
      NULL,            // Task handle
      1                // Core ID → 0 or 1
  );
  xTaskCreatePinnedToCore(
      print_data,   // Task function
      "print_task", // Name
      4096,         // Stack
      NULL,         // Task parameter
      5,            // Priority
      NULL,         // Task handle
      1             // Core ID → 0 or 1
  );
}

void loop()
{
  web_socket_task();
}
