# ESP32 Drone Flight Controller

> 🚧 **Work in progress.** The flight controller is being developed and tuned on a 3" prototype quadcopter. Expect breaking changes.

A quadcopter flight controller written from scratch for the ESP32, instead of using ready-made firmware like Betaflight. The ESP32 reads the IMU, runs its own PID stabilization loop, and drives four brushless motors over DShot. It also hosts its own WiFi network, so I can send commands and watch live telemetry from my laptop.

The goal is to understand how a flight controller actually works by building every part of it: sensor reading, attitude control, motor mixing, safety, and communication.

## What it does now

- Reads pitch and roll angles from an MPU6050 IMU over I2C
- Runs an angle PID loop on pitch and roll, with a filtered D term
- Mixes throttle and PID outputs into four motor commands (quad X layout)
- Drives four ESCs with the DShot300 digital protocol, using the ESP32's RMT peripheral
- Creates its own WiFi access point, with:
  - a **command port** (TCP 23) for arming, throttle, and live PID tuning
  - a **telemetry port** (TCP 24) streaming angles and motor outputs
- Disarms automatically if the drone tilts past 60°

## Hardware

| Part | Details |
|---|---|
| Flight controller | ESP32-S3 |
| IMU | MPU6050 (I2C: SDA = GPIO 8, SCL = GPIO 9) |
| ESCs | DShot300-capable, one signal wire per motor |
| Motors | Small brushless motors for 3" props |
| Frame | 3" quad X (prototype) |
| Battery | 4S LiPo |

### Motor pins

| Motor | GPIO |
|---|---|
| M1 | 4 |
| M2 | 5 |
| M3 | 6 |
| M4 | 7 |

Some of the early test sketches used GPIO 17 and 16 for M3 and M4, from an older wiring setup.

### Motor mixing

```
M1 = throttle - pitch - roll
M2 = throttle + pitch + roll
M3 = throttle - pitch + roll
M4 = throttle + pitch - roll
```

Each output is clamped to the DShot throttle range (48–2047).

## How it works

Every loop, the flight controller:

1. Reads new commands from USB serial and WiFi
2. Updates the IMU and reads pitch and roll angles
3. Calculates the PID output for each axis (target angle = 0°, level hover)
4. Checks the tilt safety limit
5. Mixes throttle and PID outputs and sends them to the four motors
6. Sends a telemetry line every 100 loops

## Repository structure

```
esp32-drone-fc/
├── firmware/
│   ├── flight_controller/      Main firmware: PID, motors, WiFi commands, telemetry
│   └── tests/
│       ├── gyro_test/          Prints IMU angles to check the sensor
│       ├── motor_spin_test/    Spins all 4 motors at low throttle
│       ├── motor_mixing_test/  Tests the mixing math with fixed pitch/roll/yaw values
│       └── motor_pair_test/    Spins M3 and M4 only, for wiring checks
├── tools/
│   ├── control.py              Send commands to the drone over WiFi
│   └── telemetry.py            Print live telemetry from the drone
└── docs/
    └── build-log.md            Progress notes, problems, and fixes
```

## Requirements

### Firmware

- Arduino IDE (or PlatformIO) with the ESP32 board package by Espressif
- Libraries:
  - **MPU6050_light** by rfetick
  - **DShotRMT** by derdoktor667

### Tools

- Python 3 (no extra packages needed; only `socket` and `threading` from the standard library)

## How to use

> ⚠️ **Remove the props for all bench testing.** Brushless motors on a 4S battery can cause serious injuries.

1. Upload `firmware/flight_controller` to the ESP32.
2. Connect the battery. Keep the drone still and level while it boots, because the IMU calibrates at startup.
3. On your laptop, connect to the WiFi network the drone creates.
4. Check the connection:
   ```
   ping 192.168.4.1
   ```
5. In one terminal, start the command console:
   ```
   python tools/control.py
   ```
6. In another terminal, start the telemetry viewer:
   ```
   python tools/telemetry.py
   ```

### Commands

Type these into `control.py` (or the Arduino Serial Monitor at 115200 baud):

| Command | Example | What it does |
|---|---|---|
| `A` | `A` | Arm the motors |
| `D` | `D` | Disarm the motors and set throttle to 0 |
| `T<value>` | `T300` | Set throttle (0–1500) |
| `KP<value>` | `KP4.5` | Set the P gain (0–10) |
| `KI<value>` | `KI0.2` | Set the I gain (0–3) |
| `KD<value>` | `KD0.5` | Set the D gain (0–2) |

Changing a gain resets the integral terms and prints the new gains.

### Telemetry format

Each telemetry line is:

```
pitch roll M1 M2 M3 M4
```

Angles are in degrees, and motor values are DShot throttle values.

## Roadmap

- [x] Read the IMU and print angles
- [x] Spin motors with DShot300
- [x] Motor mixing for quad X
- [x] Angle PID on pitch and roll
- [x] WiFi commands and live gain tuning
- [x] WiFi telemetry
- [ ] Failsafe: disarm when the command link is lost
- [ ] Safe arming (only arm at zero throttle)
- [ ] Yaw control
- [ ] Fixed-rate control loop, with loop-time measurement
- [ ] Battery voltage monitoring
- [ ] Telemetry logging to a file, and plotting for PID tuning
- [ ] Stable hover on the 3" prototype
- [ ] Move the firmware to a 6" drone
- [ ] Autonomous flight (planned as a separate repository)

## Progress log

See [docs/build-log.md](docs/build-log.md).
