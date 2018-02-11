#include "Sakura_ezWiFi.h"

#define forward_left 36
#define forward_right 30
#define backward_left 37
#define backward_right 31



bool brake_range=0, brake_wifi=0;

#define trigPin 8
// Ultrasonic's trigPin
#define echoPin 9
// Ultrasonic's echoPin
 
int distance = 30; // Distance to the front car
int accuracy = 3;  // Range of correct distance
int motorSpeed = 100;

ezWiFi wifi(115200);//Включаем WiFi

char wifi_get_movecommand(void)
{
  wifi.get_req("connect.EducationRobots.RU", "/c.php?tabl=var_11&par=1");
  return wifi.buff[0];
}

int incMotor() {  // Accelerate motor
  if (motorSpeed < 250){
    motorSpeed += 3;
    return motorSpeed;
  }
}
 
int decMotor() {  // Decelerate motor
  if (motorSpeed > 80){
    motorSpeed -= 3;
    return motorSpeed;
  }
}
 
void forward(int rotSpeed) {  // Car moves forward
  analogWrite(backward_right, 0);
  analogWrite(backward_left, 0);
  analogWrite(forward_right, rotSpeed);
  analogWrite(forward_left, rotSpeed);
}
 
void backward(int rotSpeed) {  // Car moves backward
  analogWrite(forward_right, 0);
  analogWrite(forward_left, 0);
  analogWrite(backward_right, rotSpeed);
  analogWrite(backward_left, rotSpeed);
}
 
void brake() {   // Brake
  analogWrite(backward_right, 0);
  analogWrite(forward_right, 0);
  analogWrite(forward_left, 0);
  analogWrite(backward_left, 0);
}
 
int getRange() {  // Getting range without Ultrasonic lib
  int duration;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, 5000);
  return duration/58;  // Returning range in centimeters
}
 
void setup() {
  Serial.begin(9600);  // Debugging
  pinMode(backward_right, OUTPUT); // Right backward
  pinMode(forward_right, OUTPUT); // Right forward
 
  pinMode(backward_left, OUTPUT); // Left backward
  pinMode(forward_left, OUTPUT); // Left forward
 
  pinMode(trigPin, OUTPUT); // Trig pin
  pinMode(echoPin, INPUT); // Echo pin
}
void loop()
{
  int range = getRange();
 
  Serial.println(range);
  Serial.println(motorSpeed);
 
  if ((range < 10)&&(range != 0)) brake_range=1;
  else brake_range=0;

  if(wifi_get_movecommand()) brake_wifi=0;
  else brake_wifi=1;

  if(brake_wifi || brake_range) brake();
  else
  {
    if ((range > (distance + accuracy))||(range == 0)) forward(incMotor());
    if (range < (distance - accuracy)) forward(decMotor());
  }
  
  //delay(50);
}
