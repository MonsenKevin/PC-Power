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

* **ESP32 Development Board** (Standard ESP32-WROOM or similar). I used [this one](https://www.amazon.com/dp/B0D8T53CQ5?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_2).
* **Female-to-Female DuPont Jumper Wires** I used [this one](https://www.amazon.com/dp/B01EV70C78?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_3).
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

### Step 1: Choose Your Program Version

This project includes two firmware versions:

#### **PC-Power-NoStatus.ino** (Simpler, Stateless)
- **Best for:** Users who don't need to track PC power state in the cloud
- **Features:** Supports V1 (short press) and V2 (force off) only
- **Setup:** No V3 datastream required, no Windows Task Scheduler setup needed
- **Downside:** The Blynk app cannot display whether the PC is currently on or off

#### **PC-Power-Status.ino** (Full-Featured, Stateful)
- **Best for:** Users who want to see the PC's power state in the Blynk app
- **Features:** Supports V1, V2, and V3 (status tracking); automatically syncs PC state to the cloud
- **Setup:** Requires creating V3 datastream in Blynk and loading the Windows Task Scheduler action
- **Benefit:** The Blynk app displays the current power state and updates when the PC boots/shuts down

### Step 2: Configure Blynk Datastreams

1. Go to your Blynk project and navigate to the **Datastreams** tab
2. Create the following virtual pins:

   | Virtual Pin | Name | Type | Min | Max | Purpose |
   |---|---|---|---|---|---|
   | V1 | Power Toggle | Integer | 0 | 1 | Short press (boot/soft shutdown) |
   | V2 | Force Off | Integer | 0 | 1 | Force shutdown (5-second hold) |
   | V3* | PC Status | Integer | 0 | 1 | Tracks PC power state (0=Off, 1=On) |
   
   *Only needed for **PC-Power-Status.ino**

3. For **PC-Power-Status.ino** only: After creating V3, set its initial value to match your PC's current state (0 if off, 1 if on)

### Step 3: Install and Configure the Firmware

1. Open the appropriate code in the [Arduino IDE](https://www.arduino.cc/en/software):
   - `PC-Power-NoStatus.ino` (simple) or
   - `PC-Power-Status.ino` (with status tracking)

2. Install the **ESP32 Board Package** and the **Blynk** library via the Library Manager

3. Rename `secrets_template.h` to `secrets.h` and fill in your credentials:

   ```cpp
   char ssid[] = "YOUR_WIFI_NAME";
   char pass[] = "YOUR_WIFI_PASSWORD";
   #define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"
   ```

4. Flash the code to your ESP32

### Step 4: (PC-Power-Status Only) Set Up Windows Task Scheduler

If you are using **PC-Power-Status.ino**, you must install the provided Windows Task Scheduler action to automatically update V3 when the PC shuts down. The task calls a PowerShell script that notifies Blynk of the shutdown event.

1. **Configure the PowerShell Script:**
   - Open `Run Blynk Script on Shutdown.ps1` in a text editor
   - Find the line with `YOUR_AUTH_TOKEN` placeholder
   - Replace `YOUR_AUTH_TOKEN` with your actual Blynk Auth Token (same one you put in `secrets.h`)
   - Save the file

2. **Open Task Scheduler:**
   - Press `Win + R`, type `taskschd.msc`, and press Enter

3. **Import the Task:**
   - In Task Scheduler, go to **Action** → **Import Task**
   - Navigate to and select `Run Blynk Script on Shutdown.xml` from this project folder
   - Click **Open**
   - The imported task will appear in the Task Scheduler and is now ready to use

4. **Verify the Task:**
   - The task is configured to run at shutdown
   - When your PC shuts down, the PowerShell script will execute and send a request to Blynk updating V3 to `0` (off)
   - When you boot with V1, the firmware will automatically sync V3 back to `1` (on)

## Blynk & Apple Shortcuts Configuration

### Supported Virtual Pins

| Version | V1 | V2 | V3 |
|---------|----|----|-----|
| **PC-Power-NoStatus** | ✓ Short Press | ✓ Force Off | ✗ Not supported |
| **PC-Power-Status** | ✓ Short Press | ✓ Force Off | ✓ Status Toggle |

### Virtual Pin Details

* **V1 (Short Press):** Simulates a 500ms press of the power button to turn the PC on or cleanly shut down Windows. Works with both versions.
* **V2 (Force Off):** Simulates a 5-second hold of the power button for a hard shutdown. Works with both versions.
* **V3 (Status Toggle):** *(PC-Power-Status.ino only)* Toggles and tracks the on/off state. Automatically updated when the PC boots or shuts down via Task Scheduler.

### Apple Shortcuts URLs

To trigger actions via Apple Shortcuts, use the "Get Contents of URL" action with these HTTP GET requests:

**Boot / Soft Shutdown (V1):**
```
https://blynk.cloud/external/api/update?token=YOUR_AUTH_TOKEN&v1=1
```

**Force Shutdown (V2):**
```
https://blynk.cloud/external/api/update?token=YOUR_AUTH_TOKEN&v2=1
```

**Check Status (V3 - PC-Power-Status only):**
```
https://blynk.cloud/external/api/get?token=YOUR_AUTH_TOKEN&v3
```

### Pre-Built Apple Shortcuts

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

**Check Status (PC-Power-Status only):**
```
https://www.icloud.com/shortcuts/46dbc0d17be64a94a4b15b8bdc532de0 
```

## Troubleshooting

### The Status Display is Incorrect (PC-Power-Status only)
If you update the firmware or experience a power cycle, V3 may become out of sync with your PC's actual state. To fix it:
1. Go to the Blynk cloud console
2. Find the **Datastreams** page
3. Manually update V3 to the correct state (0 = Off, 1 = On)
4. The Task Scheduler will keep it in sync going forward

### The Computer Turns On Momentarily Then Shuts Off
Verify that the wires are connected to the correct polarity. The positive wire should connect to the positive pin and the negative wire should connect to ground, forming a complete circuit. Incorrect polarity can cause unexpected behavior.

### My Computer Turns On But Won't Display Anything on the Screen
If you repeated the previous wiring issue, the BIOS may stop booting. Clearing the CMOS battery (removing and reinserting it) resolved this issue for me.

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