# controller_subtests

Breadboard prototyping tests for a custom Sound Voltex (SDVX) controller built on the Raspberry Pi Pico 2 H (RP2350).

Each subfolder is a self-contained test with its own `CMakeLists.txt` and `main.c`.

---

## Project Structure

```
controller_subtests/
├── CMakeLists.txt          ← top-level, registers all subtests
├── pico_sdk_import.cmake
├── serial_monitor.py       ← Python serial monitor for debugging
├── mx_switch_test/
│   ├── CMakeLists.txt
│   └── main.c              ← Cherry MX Brown button + onboard LED test
└── encoder_test/
    ├── CMakeLists.txt
    └── main.c              ← Bourns PEC11R rotary encoder test
```

---

## Hardware

| Component | Part |
|---|---|
| Microcontroller | Raspberry Pi Pico 2 H (RP2350, pre-soldered headers) |
| Buttons | Cherry MX Brown switches (jumper wires soldered directly to pins) |
| Encoders | Bourns PEC11R-xxxxK (no detents, with push switch) |

---

## Subtest: mx_switch_test

Tests a single Cherry MX Brown switch on GPIO 0. Jumper wires are soldered directly to the two switch pins. Toggles the onboard LED on press/release and prints state over USB serial.

**Wiring:**
```
Switch Pin 1  → GPIO 0  (pin 1)
Switch Pin 2  → GND
```

> **Note:** Uses the Pico's internal pull-up — no external resistor needed. Pin reads HIGH at rest and LOW when pressed.

---

## Subtest: encoder_test

Tests a Bourns PEC11R rotary encoder — quadrature rotation via interrupt and push switch via polling.

**Wiring:**
```
ENC A    → GPIO 10  (pin 14)
ENC C    → GND
ENC B    → GPIO 11  (pin 15)
SW pin 1 → GPIO 12  (pin 16)
SW pin 2 → GND
```

**How it works:**
- An interrupt fires on every rising/falling edge of channel A
- Inside the ISR, A and B are compared — if they differ, direction is CW (+1), if same, direction is CCW (-1)
- The push switch is polled in the main loop with debounce

---

## Building

Requires the [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk). Set `PICO_SDK_PATH` in your environment or let CLion handle it via the CMake Environment field.

```bash
mkdir cmake-build-debug && cd cmake-build-debug
cmake ..
make
```

Each subtest outputs its own `.uf2` into its own subfolder:
```
cmake-build-debug/
├── mx_switch_test/
│   └── mx_switch_test.uf2
└── encoder_test/
    └── encoder_test.uf2
```

**To flash:**
1. Hold BOOTSEL on the Pico while plugging in USB
2. It mounts as a drive called `RP2350`
3. Drag and drop the `.uf2` onto the `RP2350` drive

---

## Serial Monitor

Use the included `serial_monitor.py` to watch output over USB serial.

**Requirements:**
```bash
pip install pyserial
```

**Run:**
```bash
python serial_monitor.py
```

Defaults to `COM3` at `115200` baud. Edit `PORT` at the top of the script if your Pico is on a different port (check Device Manager → Ports (COM & LPT)).

## CLion Serial Monitor (alternative)

CLion has a built-in serial monitor under **Tools → Serial Monitor**.

Settings that must be enabled for it to work with the Pico:
- **Enable HW Flow Control** — on
- **Local Echo** — on
- **DTR** — enabled (this is what actually triggers the Pico to send output)

Without DTR enabled, the monitor connects but receives nothing.

---

## Notes

- Both encoder A and B pins use the Pico's internal pull-ups — no external resistors needed for basic testing
- For final PCB design, add a hardware filter (10KΩ + 0.01µF on each channel) per the PEC11R datasheet recommendation to reduce contact bounce at higher RPM
- The encoder's position counter is unbounded (`volatile int`) — for HID use, report the delta each frame rather than an absolute value
