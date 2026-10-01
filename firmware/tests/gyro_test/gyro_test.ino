#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);
  mpu.begin();         
  delay(1000);
  mpu.calcOffsets();    
}

void loop() {
  mpu.update();
  
  Serial.print("X:");
  Serial.print(mpu.getAngleX());
  Serial.print(" Y:");
  Serial.print(mpu.getAngleY());
  Serial.print(" Z:");
  Serial.println(mpu.getAngleZ());
  
  delay(50);
}