# Bulletproof ESP32 PC Remote Switch

A highly resilient, hardware-level remote power management system for a desktop PC.

This project uses an ESP32 mounted entirely inside the PC case to control the motherboard's power header. It allows you to remotely turn your PC on, force shut it down, and trigger it via Apple Shortcuts or the Blynk app from anywhere in the world.

Unlike standard IoT tutorials, this code is engineered for **production-grade reliability**. It features a completely non-blocking network stack, physical button pass-through, and a self-healing watchdog to ensure the physical power button never stops working, even during internet outages or server disconnects.

## Features

* **100% Internal Hardware:** Powered directly by the motherboard's internal USB 2.0 header. No ugly micro-USB cables routed out the back of the case.
* **True Hardware Boot:** Bypasses the unreliability of Wake-on-LAN (WoL) and restrictive university/corporate NAT firewalls.
* **Non-Blocking Logic:** The internet connection manager runs in the background. If your router goes down, the ESP32 does not freeze; your physical case button will continue to turn the PC on and off instantly.
* **Self-Healing Watchdog:** If the ESP32 loses connection to the cloud for 5 continuous minutes, it assumes a memory leak or zombie state and executes a clean software reboot to flush its RAM, all without triggering the PC power pins.
* **Apple Shortcuts Integration:** Uses webhooks to trigger PC states via Siri or iOS home screen buttons.

## Hardware Requirements

* **ESP32 Development Board** (Standard ESP32-WROOM or similar)
* **Female-to-Female DuPont Jumper Wires**
* A motherboard with an internal 9-pin USB 2.0 header

> **Note:** Your motherboard BIOS must be configured to provide "Standby Power" (sometimes called ErP Ready = Disabled) to USB headers so the ESP32 stays on when the PC is off.

## Wiring Guide

### 1. Powering the ESP32 (From Internal USB Header)

* `5V` (Internal USB Header)  ➡️  `VIN` (ESP32) — *Do not use the 3.3V pin!*
* `GND` (Internal USB Header) ➡️  `GND` (ESP32)

### 2. Motherboard Power Control

* `PWR_SW +` (Motherboard Front Panel) ➡️ `GPIO 27` (ESP32)
* `PWR_SW -` (Motherboard Front Panel) ➡️ `GND` (ESP32)

### 3. Physical Case Button Pass-Through

* Case Power Button (Wire 1) ➡️ `GPIO 23` (ESP32)
* Case Power Button (Wire 2) ➡️ `3.3V` (ESP32) — *Ensure GPIO 23 is set to `INPUT_PULLDOWN` in code*

## Software Setup

1. Open the code in the [Arduino IDE](https://www.arduino.cc/en/software).
2. Install the **ESP32 Board Package** and the **Blynk** library via the Library Manager.
3. Rename `secrets_template.h` to `secrets.h` and fill in your credentials:

   ```cpp
   char ssid[] = "YOUR_WIFI_NAME";
   char pass[] = "YOUR_WIFI_PASSWORD";
   #define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"
   ```

4. Flash the code to your ESP32.

## Blynk & Apple Shortcuts Configuration

This system listens to three Virtual Pins from the Blynk Cloud:

* **V1 (Short Press):** Simulates a 500ms press of the power button to turn the PC on or cleanly shut down Windows.
* **V2 (Force Off):** Simulates a 5-second hold of the power button for a hard shutdown.
* **V3 (Status Toggle):** Toggles the stored on/off state. Called automatically after V1 and V2 actions, or can be polled directly to read the current state.

To trigger these via Apple Shortcuts, use the "Get Contents of URL" action with the following HTTP GET requests:

**Boot / Soft Shutdown (V1):**
```
https://blynk.cloud/external/api/update?token=YOUR_AUTH_TOKEN&v1=1
```

**Force Shutdown (V2):**
```
https://blynk.cloud/external/api/update?token=YOUR_AUTH_TOKEN&v2=1
```

**Check Status (V3):**
```
https://blynk.cloud/external/api/get?token=YOUR_AUTH_TOKEN&v3
```

## Why OUTPUT_OPEN_DRAIN?

This project safely interfaces a 3.3V ESP32 with a 3.3V/5V motherboard header by setting the output pin to `OUTPUT_OPEN_DRAIN`. Instead of pushing voltage into the motherboard (which is dangerous), it simply pulls the motherboard's existing standby voltage down to ground, perfectly mimicking a physical mechanical switch.

## ⚠️ Disclaimer

Working with internal computer hardware and live jumper wires carries a risk of short circuits. Always unplug your PC from the wall before wiring headers. Ensure you are sending 5V to the `VIN` pin and not directly to the 3.3V rail of the ESP32. Use this code and schematic at your own risk.
