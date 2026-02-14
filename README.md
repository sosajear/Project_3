## Windshield Wiper Subsystem

| Specification | Test Process | Results |
| :---- | :---- | :---- |
| 1. Wipers only run when engine is running. | 1. Engine OFF, select any wiper mode (HI, LO, INT) 2. Engine ON, select any wiper mode | 1. No wiper movement, servo holds at 0° 2. Wipers operate according to selected mode |
| 2. HI mode: continuous sweeping at ~25 rpm (fast speed). | Engine ON, set wiper mode potentiometer to HI range (2475-3300mV) | Wiper sweeps continuously between 0° and 90° at fast speed (~2.4s per cycle) |
| 3. LO mode: continuous sweeping at ~10 rpm (slow speed). | Engine ON, set wiper mode potentiometer to LO range (1650-2475mV) | Wiper sweeps continuously between 0° and 90° at slow speed (~6.0s per cycle) |
| 4. INT mode: low-speed sweep with hesitation at 0°. | Engine ON, set wiper mode potentiometer to INT range (825-1650mV) | Wiper completes one slow sweep, hesitates at 0°, then repeats cycle |
| 5. INT delay time matches selector position (SHORT, MEDIUM, LONG). | Engine ON, INT mode selected: 1. Set delay pot to SHORT (0-1100mV) 2. Set delay pot to MEDIUM (1100-2200mV) 3. Set delay pot to LONG (2200-3300mV) | 1. ~1 second hesitation at 0° 2. ~3 second hesitation at 0° 3. ~5 second hesitation at 0° |
| 6. LCD displays correct wiper mode, including delay for INT. | Engine ON: 1. Select HI mode 2. Select LO mode 3. Select INT mode with SHORT delay 4. Select INT mode with MEDIUM delay 5. Select INT mode with LONG delay 6. Select OFF mode | 1. LCD shows "HI" 2. LCD shows "LOW" 3. LCD shows "INT" 4. LCD shows "INT" 5. LCD shows "INT" 6. LCD shows "OFF" (Note: LCD does not display delay time based on code) |
| 7. Turning wipers OFF completes current cycle and returns to 0°. | Engine ON, wipers moving in any mode, turn mode potentiometer to OFF range (0-825mV) | Wiper completes current sweep (ramp up and down), returns to 0°, then stops |
| 8. Turning engine OFF completes current cycle and returns to 0°. | Engine ON, wipers moving in any mode, press ignition button to stop engine | Wiper completes current sweep (ramp up and down), returns to 0°, then stops (engine_off flag triggers break in loops) |
| 9. Wipers stationary at 0° when OFF. | 1. Engine OFF with any mode selected 2. Engine ON with OFF mode selected | Servo holds position at 0° (LEDC_DUTY_MIN = 3.75% duty cycle) |
| 10. Wiper tasks handle mode changes correctly. | Engine ON, rapidly switch between HI, LO, INT, and OFF modes | Previous task is deleted, new task created; no crashes or stuck wipers |

## Ignition Subsystem

| Specification | Test Process | Results |
| :---- | :---- | :---- |
| 1. Display welcome message when driver sits down. | Driver seat button pressed (D_WEIGHT_BUTTON = 0) | "Welcome to enhanced alarm system model 218-W25" printed once |
| 2. Enable engine start (light the green LED) only while both seats are occupied AND both seatbelts fastened. | Four buttons: DS, DB, PS, PB 1. All buttons pressed (0) 2. Any combination missing at least one condition | 1. Green LED on 2. Green LED off |
| 3. Start the engine (light blue LED, turn off green) when ignition enabled (green LED on) and ignition button pressed. | All buttons pressed (green LED on), ignition button pressed | Blue LED on, green LED off, "Engine Started" printed |
| 4. Inhibit ignition and sound buzzer if not all safety conditions are met when ignition button pressed. | Green LED off (any missing condition), ignition button pressed | Buzzer sounds for ~2 seconds, "Ignition Prohibited" printed |
| 5. Print appropriate error messages listing all missing safety conditions when ignition inhibited. | Four buttons and ignition pressed: 1. All but passenger seat (PS) pressed 2. All but driver seatbelt (DB) pressed 3. No buttons pressed | 1. "passenger seat not occupied" 2. "driver seatbelt not on" 3. All four error messages printed |
| 6. Keep engine running even if seats/belts change after start. | Engine running, release any combination of seat or belt buttons | Engine continues running (blue LED stays on) |
| 7. Stop the engine when ignition button pressed while engine is running. | Engine running, ignition button pressed | Blue LED off, "Engine Stopped" printed |
| 8. Allow multiple start attempts after failed ignition. | Green LED off, press ignition (fails), correct missing conditions, press ignition again | First attempt: buzzer, errors. Second attempt: engine starts |
| 9. Reset welcome message behavior after engine stop. | Engine stopped, driver leaves (releases button) and sits down again | Welcome message prints again |
