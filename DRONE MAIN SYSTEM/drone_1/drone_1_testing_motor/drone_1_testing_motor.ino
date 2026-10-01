#include "DShotRMT.h"
DShotRMT motor3(GPIO_NUM_17, DSHOT300);
DShotRMT motor4(GPIO_NUM_16, DSHOT300);

void setup() {
  motor3.begin();
  motor4.begin();
  for (int i = 0; i < 2000; i++) { motor3.sendThrottle(0); motor4.sendThrottle(0); delay(1); }
}
void loop() { motor3.sendThrottle(150); motor4.sendThrottle(150); delay(1); }