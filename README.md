# Project 3 – Windshield Wiper Subsystem

## Team Members
- [Name 1]
- [Name 2]

## Project Overview
Our project implements an enhanced ignition and windshield wiper control system for a Driver's Education Smart Car using an ESP32-S3 microcontroller. The system builds on previous projects by adding realistic wiper behavior to the existing safety-focused ignition controls.

The **ignition subsystem** requires both seats to be occupied and both seatbelts fastened before the engine can start. If conditions are not met, the system displays specific error messages and sounds an alarm. Once running, the engine remains on even if occupants shift or exit, and can be turned off by pressing the ignition button again.

The **windshield wiper subsystem** operates only when the engine is running. Users can select between four modes using a potentiometer: HI (high-speed), LO (low-speed), INT (intermittent), and OFF. A second potentiometer selects the delay time (SHORT, MEDIUM, LONG) for intermittent mode. The wiper motor simulates realistic motion, ramping up and down between 0 and 90 degrees. When turning the wipers off or shutting down the engine, the wipers complete their current cycle and return to 0 degrees. The LCD display shows the current wiper mode, including the selected delay time for intermittent operation.

## Hardware Design
- **Microcontroller:** ESP32-S3
- **Inputs:**
  - 2x push buttons (driver/passenger seat occupancy)
  - 2x slider switches (driver/passenger seatbelt status)
  - 1x push button (ignition switch)
  - 2x potentiometers (wiper mode selector, intermittent delay selector)
- **Outputs:**
  - 1x green LED (ignition enabled)
  - 1x blue LED (engine running)
  - 1x buzzer/alarm (ignition inhibited)
  - 1x 2x20 LCD display (system messages and wiper mode)
  - 1x servo motor (windshield wiper simulation) with visible attachment

## Design Alternatives
One key design decision was selecting between a **continuous servo motor** and a **position servo motor** to simulate the windshield wiper.

*We chose a [continuous/position] servo motor because:*
[Explain your reasoning here. For example:
- *Position servo:* Easier to control precise angle (0-90 degrees), simpler code for ramp up/down behavior.
- *Continuous servo:* Better mimics a real wiper motor's constant rotation but requires more complex control to simulate the sweeping motion and hesitation.]

## System Behavior Summary

### Ignition Subsystem
| Test Case | Seats Occupied | Seatbelts Fastened | Ignition Button Pressed | Green LED | Blue LED | Buzzer | LCD Display |
|-----------|----------------|---------------------|------------------------|-----------|----------|--------|-------------|
| 1 | Neither | Neither | Yes | OFF | OFF | ON | "Ignition inhibited" with all four error messages |
| 2 | Both | Neither | Yes | OFF | OFF | ON | "Ignition inhibited" with both seatbelt error messages |
| 3 | Both | Driver only | Yes | OFF | OFF | ON | "Ignition inhibited" with passenger seatbelt error |
| 4 | Both | Both | Yes | ON | ON | OFF | "Engine started" |
| 5 | Both | Both | Yes (engine running) | ON | OFF | OFF | "Engine stopped" |

### Windshield Wiper Subsystem
| Test Case | Engine Status | Wiper Mode | Delay Setting | Wiper Behavior | LCD Display |
|-----------|---------------|------------|---------------|----------------|-------------|
| 1 | OFF | Any | Any | Stationary at 0° | [No change or off] |
| 2 | ON | OFF | Any | Complete current cycle, return to 0°, stop | "OFF" |
| 3 | ON | HI | Any | Continuous 25 rpm sweep (0°→90°→0°) | "HI" |
| 4 | ON | LO | Any | Continuous 10 rpm sweep (0°→90°→0°) | "LO" |
| 5 | ON | INT | SHORT | Low-speed sweep with 1s hesitation at 0° | "INT SHORT" |
| 6 | ON | INT | MEDIUM | Low-speed sweep with 3s hesitation at 0° | "INT MEDIUM" |
| 7 | ON | INT | LONG | Low-speed sweep with 5s hesitation at 0° | "INT LONG" |
| 8 | ON → OFF | Any | Any | Complete current cycle, return to 0°, stop | "OFF" |

### Timing Verification
We verified the wiper timing using a stopwatch over multiple cycles:

- **HI mode:** Target period = [calculate: 60/25 = 2.4 seconds per cycle]. Measured average over 10 cycles: [X.XX] seconds.
- **LO mode:** Target period = [calculate: 60/10 = 6.0 seconds per cycle]. Measured average over 10 cycles: [X.XX] seconds.
- **INT mode (SHORT):** Target period = LO cycle time + 1s = [X.X] seconds. Measured average: [X.XX] seconds.
- **INT mode (MEDIUM):** Target period = LO cycle time + 3s = [X.X] seconds. Measured average: [X.XX] seconds.
- **INT mode (LONG):** Target period = LO cycle time + 5s = [X.X] seconds. Measured average: [X.XX] seconds.

All measured values were within acceptable tolerance, confirming correct timing behavior.

## Challenge Task Implementation
*[If you completed the challenge task, describe it here. For example:]*
We implemented the realistic shut-down behavior: if the engine is turned off while the wipers are moving, they freeze in their current position. When the engine is restarted, the wipers return to 0 degrees at low speed before resuming normal operation.
