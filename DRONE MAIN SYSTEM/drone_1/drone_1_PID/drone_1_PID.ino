#include "Wire.h"
#include <MPU6050_light.h>
#include "DShotRMT.h"
#include <WiFi.h>

MPU6050 mpu(Wire);
DShotRMT m1(GPIO_NUM_4, DSHOT300);
DShotRMT m2(GPIO_NUM_5, DSHOT300);
DShotRMT m3(GPIO_NUM_6, DSHOT300);
DShotRMT m4(GPIO_NUM_7, DSHOT300);

WiFiServer server(23);
WiFiClient client;
WiFiServer telemServer(24);
WiFiClient telem;

float Kp = 4.5, Ki = 0.0, Kd = 0.0;
float throttle = 0;

float pitchInt = 0, pitchPrev = 0, pitchDFilt = 0;
float rollInt = 0, rollPrev = 0, rollDFilt = 0;
unsigned long prevTime = 0;

bool armed = false;
unsigned long lastCmd = 0;

void applyCmd(String line) {
  lastCmd = millis();
  char cmd = toupper(line.charAt(0));
  if (cmd == 'K' && line.length() >= 3) {
    char g = toupper(line.charAt(1));
    float v = line.substring(2).toFloat();
    if (g == 'P') Kp = constrain(v, 0, 10);
    if (g == 'I') Ki = constrain(v, 0, 3);
    if (g == 'D') Kd = constrain(v, 0, 2);
    pitchInt = 0; rollInt = 0;
    String msg = "gains " + String(Kp) + " " + String(Ki) + " " + String(Kd);
    Serial.println(msg);
    if (client && client.connected()) client.println(msg);
    return;
  }
  float val = line.substring(1).toFloat();
  if (cmd == 'T') throttle = constrain(val, 0, 1500);
  if (cmd == 'A') { armed = true; pitchInt = 0; rollInt = 0; }
  if (cmd == 'D') { armed = false; throttle = 0; }
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(5);
  Serial.println("boot");
  Wire.begin(8, 9);
  mpu.begin();
  delay(1000);
  mpu.calcOffsets();
  Serial.println("mpu ok");

  m1.begin();
  m2.begin();
  m3.begin();
  m4.begin();
  for (int i = 0; i < 2000; i++) {
    m1.sendThrottle(0);
    m2.sendThrottle(0);
    m3.sendThrottle(0);
    m4.sendThrottle(0);
    delay(1);
  }
  Serial.println("ready");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("drone", "12345678", 1);
  server.begin();
  telemServer.begin();
  Serial.println("wifi up");

  prevTime = micros();
}

void readCommands() {
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() < 1) continue;
    applyCmd(line);
  }
}

void readWiFi() {
  if (!client || !client.connected()) {
    client = server.available();
    return;
  }
  while (client.available()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() < 1) continue;
    applyCmd(line);
  }
}

void sendTelem(float pitch, float roll, int a, int b, int c, int d) {
  if (!telem || !telem.connected()) telem = telemServer.available();
  static int dbg = 0;
  if (++dbg >= 100) {
    dbg = 0;
    String s = String(pitch) + " " + String(roll) + " " + a + " " + b + " " + c + " " + d;
    Serial.println(s);
    if (telem && telem.connected()) telem.println(s);
  }
}

void loop() {
  readCommands();
  readWiFi();
  mpu.update();

  unsigned long now = micros();
  float dt = (now - prevTime) / 1000000.0;
  prevTime = now;
  if (dt <= 0) dt = 0.001;

  float pitch = mpu.getAngleX();
  float roll = mpu.getAngleY();

  float pitchErr = 0 - pitch;
  pitchInt += pitchErr * dt;
  float pitchDRaw = (pitchErr - pitchPrev) / dt;
  pitchDFilt = pitchDFilt * 0.7 + pitchDRaw * 0.3;
  float pitchOut = Kp * pitchErr + Ki * pitchInt + Kd * pitchDFilt;
  pitchPrev = pitchErr;

  float rollErr = 0 - roll;
  rollInt += rollErr * dt;
  float rollDRaw = (rollErr - rollPrev) / dt;
  rollDFilt = rollDFilt * 0.7 + rollDRaw * 0.3;
  float rollOut = Kp * rollErr + Ki * rollInt + Kd * rollDFilt;
  rollPrev = rollErr;

  if (abs(pitch) > 60 || abs(roll) > 60) { armed = false; throttle = 0; }

  if (!armed) {
    m1.sendThrottle(0);
    m2.sendThrottle(0);
    m3.sendThrottle(0);
    m4.sendThrottle(0);
    pitchInt = 0; rollInt = 0;
    sendTelem(pitch, roll, 0, 0, 0, 0);
    return;
  }

  int a = constrain(throttle - pitchOut - rollOut, 48, 2047);
  int b = constrain(throttle + pitchOut + rollOut, 48, 2047);
  int c = constrain(throttle - pitchOut + rollOut, 48, 2047);
  int d = constrain(throttle + pitchOut - rollOut, 48, 2047);

  m1.sendThrottle(a);
  m2.sendThrottle(b);
  m3.sendThrottle(c);
  m4.sendThrottle(d);

  sendTelem(pitch, roll, a, b, c, d);
}