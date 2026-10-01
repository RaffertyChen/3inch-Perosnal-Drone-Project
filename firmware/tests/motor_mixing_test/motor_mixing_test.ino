#include "DShotRMT.h"

DShotRMT motor1(GPIO_NUM_4, DSHOT300);
DShotRMT motor2(GPIO_NUM_5, DSHOT300);
DShotRMT motor3(GPIO_NUM_17, DSHOT300);
DShotRMT motor4(GPIO_NUM_16, DSHOT300);

float throttle = 200;   
float pitch = 0;        
float roll = 0;         
float yaw = 0;          

void setup() {
  Serial.begin(115200);
  motor1.begin();
  motor2.begin();
  motor3.begin();
  motor4.begin();

  

  for (int i = 0; i < 2000; i++) {
    motor1.sendThrottle(0);
    motor2.sendThrottle(0);
    motor3.sendThrottle(0);
    motor4.sendThrottle(0);
    delay(1);
  }

}

void loop() {

  float m1 = throttle - pitch - roll - yaw;  
  float m2 = throttle + pitch + roll - yaw;  
  float m3 = throttle - pitch + roll + yaw;  
  float m4 = throttle + pitch - roll + yaw;  

  // Keep values in valid DShot range (48-2047), clamp to be safe
  m1 = constrain(m1, 48, 2047);
  m2 = constrain(m2, 48, 2047);
  m3 = constrain(m3, 48, 2047);
  m4 = constrain(m4, 48, 2047);

  // Send to motors
  motor1.sendThrottle((int)m1);
  motor2.sendThrottle((int)m2);
  motor3.sendThrottle((int)m3);
  motor4.sendThrottle((int)m4);

  delay(1);
}