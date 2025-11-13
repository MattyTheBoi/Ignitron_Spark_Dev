# Spark Amp Testing Tools

This directory contains tools for testing ESP32 connectivity to Spark amps without physical buttons.

## Overview

The DEBUG_MODE feature allows you to control your Spark amp using serial commands from your PC. This is useful for:

- **Verifying BLE connectivity** before building physical hardware
- **Testing preset switching** and effect toggling
- **Debugging communication** with Spark amps
- **Developing new features** without constant reflashing

## Quick Start

### 1. Flash ESP32 with DEBUG_MODE

The `platformio.ini` file already has DEBUG_MODE enabled by default. Simply flash your ESP32:

```bash
# For ESP-WROOM-32 or similar
platformio run -t upload -e esp32dev --upload-port COM3

# Replace COM3 with your port (Linux: /dev/ttyUSB0, Mac: /dev/cu.usbserial-*)
```

### 2. Install Python Requirements

```bash
pip install pyserial
```

### 3. Run the Test GUI

```bash
cd test
python spark_test_gui.py
```

### 4. Connect and Test

1. Select your COM port from the dropdown
2. Click **Connect**
3. Wait for ESP32 to connect to your Spark amp (watch the serial output)
4. Click preset buttons to test!

## Serial Command Reference

You can also use any serial terminal (Arduino IDE Serial Monitor, PuTTY, etc.) at **115200 baud**.

### Preset Control
```
p1-p4    : Switch to preset 1-4
b+ / b   : Bank up
b-       : Bank down
```

### Effect Control
```
fx1      : Toggle Noise Gate
fx2      : Toggle Compressor
fx3      : Toggle Drive
fx4      : Toggle Amp
fx5      : Toggle Modulation
fx6      : Toggle Delay
fx7      : Toggle Reverb
mode     : Toggle PRESET/FX mode
```

### Information
```
status/s : Show current status
help/h/? : Show available commands
```

## Example Session

```
> status
========== STATUS ==========
BLE Connection: CONNECTED
Operation Mode: APP
Sub Mode: PRESET
Active Bank: 0
Active Preset: 1
Preset Name: Sweet Child Of Mine

Effects:
  1. bias.noisegate  : OFF
  2. BBEOpticalComp   : ON
  3. DistortionTS9    : ON
  4. Plexi            : ON
  5. ChorusAnalog     : OFF
  6. DelayMono        : OFF
  7. bias.reverb      : ON
============================

> p2
Switching to preset 2...
OK

> b+
Bank up (from 0)...
New bank: 1

> fx3
Toggling Drive...
OK
```

## Python GUI Features

### Connection Panel
- Auto-detect available COM ports
- Connect/disconnect with one click
- Visual connection status indicator

### Preset Control
- 4 preset buttons (Preset 1-4)
- Bank Up/Down buttons
- Instant feedback in serial log

### Effect Control
- Individual buttons for all 7 effects
- Toggle between PRESET and FX modes
- Visual effect names

### Information Panel
- Status button - shows full amp state
- Help button - displays command reference
- Clear log button

### Serial Output Log
- Real-time display of all serial communication
- Color-coded messages:
  - Blue: Sent commands
  - Black: Received responses
  - Red: Errors
  - Green: Connection success

## Disabling DEBUG_MODE for Production

When you're ready to build the physical hardware, disable DEBUG_MODE:

### Option 1: Comment out in platformio.ini
```ini
build_flags = -D USE_NIMBLE -DCORE_DEBUG_LEVEL=0
    ; -D DEBUG_MODE          ; Disabled for production
    ; -D DEBUG               ; Disabled for production
```

### Option 2: Create Separate Environment
Add to `platformio.ini`:

```ini
[env:esp32dev_production]
board = esp32dev
build_flags = -D USE_NIMBLE -DCORE_DEBUG_LEVEL=0
    -Wno-maybe-uninitialized
    # No DEBUG_MODE or DEBUG flags
```

Then build with:
```bash
platformio run -t upload -e esp32dev_production
```

## Troubleshooting

### "ERROR: Not connected to Spark amp"
- Make sure your Spark amp is powered on
- Ensure Bluetooth is enabled on the amp
- Wait ~10-20 seconds after ESP32 boot for BLE connection
- Check serial output for "BLE connection to Spark established"

### Python GUI doesn't show any COM ports
- Make sure ESP32 is plugged in via USB
- Install CH340/CP2102 drivers if needed (depends on your ESP32 board)
- Try clicking "Refresh" button

### Commands don't work
- Verify you're in APP mode (check status)
- Ensure BLE connection is established
- Check that DEBUG_MODE is enabled in platformio.ini
- Try reflashing the firmware

### Serial output is garbled
- Verify baud rate is set to 115200
- Check that your COM port is correct
- Try disconnecting/reconnecting

## Hardware Requirements

### Minimal Setup (for testing)
- ESP32 board (ESP-WROOM-32, ESP32-DevKit, etc.)
- USB cable
- Spark amp (Spark 40, Mini, GO, 2, or Neo)

### Not Required for Testing
- Buttons
- LEDs
- OLED display
- Battery

## Next Steps

Once you've verified connectivity and functionality:

1. Design your physical footswitch layout
2. Wire up buttons according to [Config_Definitions.h](../src/Config_Definitions.h)
3. Add LEDs for visual feedback
4. Install OLED display for status
5. Disable DEBUG_MODE
6. Flash production firmware

## Advanced: WiFi Interface (Future)

A WiFi-based interface is planned for wireless testing. This would allow:
- Control via WiFi instead of USB serial
- Web-based GUI accessible from any device
- Same command protocol over TCP/UDP

Stay tuned for updates!

## Support

For issues or questions:
- Check the main [README.md](../README.md) for overall project documentation
- Review [CLAUDE.md](../CLAUDE.md) for architecture details
- Open an issue on the repository

Happy testing!
