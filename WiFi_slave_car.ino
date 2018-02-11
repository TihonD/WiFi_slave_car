#include "Sakura_ezWiFi.h"

int trigPin = 8; // Ultrasonic's trigPin
int echoPin = 9; // Ultrasonic's echoPin
 
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
  analogWrite(5, 0);
  analogWrite(11, 0);
  analogWrite(6, rotSpeed);
  analogWrite(10, rotSpeed);
}
 
void backward(int rotSpeed) {  // Car moves backward
  analogWrite(6, 0);
  analogWrite(10, 0);
  analogWrite(5, rotSpeed);
  analogWrite(11, rotSpeed);
}
 
void brake() {   // Brake
  analogWrite(5, 0);
  analogWrite(6, 0);
  analogWrite(10, 0);
  analogWrite(11, 0);
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
  pinMode(5, OUTPUT); // Right backward
  pinMode(6, OUTPUT); // Right forward
 
  pinMode(11, OUTPUT); // Left backward
  pinMode(10, OUTPUT); // Left forward
 
  pinMode(8, OUTPUT); // Trig pin
  pinMode(9, INPUT); // Echo pin
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
