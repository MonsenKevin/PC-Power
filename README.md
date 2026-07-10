# ESP32 PC Remote Switch

A highly resilient, hardware-level remote power management system for a desktop PC.

This project uses an ESP32 mounted entirely inside the PC case to control the motherboard's power header. It allows you to remotely turn your PC on, force shut it down, and trigger it via Apple Shortcuts or the Blynk app from anywhere in the world.

This project features a completely non-blocking network stack, physical button pass-through, and a self-healing watchdog to ensure the physical power button never stops working, even during internet outages or server disconnects.

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

Below are the links to the shortcuts I use. You can simply add them to your device and update the tokens in the URL:

**Boot:**
```
https://www.icloud.com/shortcuts/c3a3eed813e64601aa30d0a7fa88c81c
```
**Soft Shutdown**
```
https://www.icloud.com/shortcuts/d29d41408ac94f65afb998d191ebd830
```
**Force Shutdown:**
```
https://www.icloud.com/shortcuts/6711baafc62f4caf9015371f4de45f0c
```

**Check Status:**
```
https://www.icloud.com/shortcuts/46dbc0d17be64a94a4b15b8bdc532de0 
```

## Troubleshooting
### The Get Status Command is backwards / says that the PC is already on despite it being off
This happens sometimes when you update the firmware and the PC gets triggered on, then the software resets. To fix it, go to the Blynk cloud console, find the datastream page, and manually update V3 to the correct state (0 = Off, 1 = On)

### The computer turns on momentarily then shuts off
Verify that the wires are connected to the correct polarity. The positive wire should connect to the positive pin and the negative wire should connect to ground, forming a complete circuit. Incorrect polarity can cause unexpected behavior

### My computer turns on but won't display anything on the screen
If you repeated the previous wiring issue, the BIOS may stop booting. Clearing the CMOS battery (removing and reinserting it) resolved this issue for me

## Why OUTPUT_OPEN_DRAIN?

This project safely interfaces a 3.3V ESP32 with a 3.3V/5V motherboard header by setting the output pin to `OUTPUT_OPEN_DRAIN`. Instead of pushing voltage into the motherboard (which is dangerous), it simply pulls the motherboard's existing standby voltage down to ground, perfectly mimicking a physical mechanical switch.

## ⚠️ Disclaimer

Working with internal computer hardware and live jumper wires carries a risk of short circuits. Always unplug your PC from the wall before wiring headers. Ensure you are sending 5V to the `VIN` pin and not directly to the 3.3V rail of the ESP32. Use this code and schematic at your own risk.

## What can you do now?
Now that you have a computer that you can power on and off from anywhere in the world, you unlock a fun use for it.
### Remoting into your computer using Parsec
Using the [Parsec Client](https://parsec.app), which is free for individuals, you can remote into your PC anywhere with an internet connection. Download the client on both the PC and the device you're remoting from.

### Workaround for University/Corporate Networks
Most university and corporate networks restrict peer-to-peer connections. You can work around this with [Tailscale](https://tailscale.com/), which is free for personal use. Install the Tailscale client on both your PC and the device you're remoting from. Afterwards both machines will appear to be on the same network. 

#### Hardcoding Tailscale IP in Parsec
To make Parsec recognize the Tailscale IP address, hardcode it in Parsec's configuration:

**On your host PC:**
1. Open Parsec settings and go to **Network**.
2. Click the blue link at the bottom: **"edit the configuration file directly"** (opens `config.txt`).
3. Scroll to the end of the document, press Enter to create a new line, and add:
   ```
   app_custom_address = 100.x.x.x
   ```
   (Replace `100.x.x.x` with your PC's actual Tailscale IP address)
4. Save the file (Ctrl+S) and close Notepad.
5. Fully restart Parsec (right-click the Parsec icon in the system tray → **Quit**, then reopen).

By explicitly defining `app_custom_address`, Parsec will bind directly to the Tailscale tunnel, bypassing your network's physical adapter. When you connect from another device via Tailscale, Parsec will find the hardcoded IP and establish the connection instantly.