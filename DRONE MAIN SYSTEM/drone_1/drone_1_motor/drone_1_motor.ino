#include "DShotRMT.h"

DShotRMT motor1(GPIO_NUM_4, DSHOT300);
DShotRMT motor2(GPIO_NUM_5, DSHOT300);
DShotRMT motor3(GPIO_NUM_6, DSHOT300);
DShotRMT motor4(GPIO_NUM_7, DSHOT300);

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
  motor1.sendThrottle(100);
  motor2.sendThrottle(100);
  motor3.sendThrottle(100);
  motor4.sendThrottle(100);
  delay(1);
}