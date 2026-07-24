//*** INITIAL SETUP ***
//You MUST fill in the Blynk authentication key and your Wifi SSID/Password before using
//You MUST rename the secrets_template.h to secrets.h in order for it to work

#include "secrets.h"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// --- PIN DEFINITIONS ---
#define POWER_PIN 27
#define BUTTON_PIN 23 

// --- VARIABLES ---
bool lastButtonState = LOW;
bool buttonState = LOW;

// --- HELPER FUNCTION: PRESS THE BUTTON ---
void triggerPower(int holdMs) {
  Serial.print("Triggering Power Pin for ");
  Serial.print(holdMs);
  Serial.println("ms");
  
  digitalWrite(POWER_PIN, LOW);  // Press button
  delay(holdMs);                 // Hold it
  digitalWrite(POWER_PIN, HIGH); // Release button
  
  Serial.println("Trigger done");
}

// --- CLOUD TRIGGER: NORMAL PRESS (Virtual Pin V1) ---
// Apple Shortcut URL: https://blynk.cloud/external/api/update?token=YOUR_AUTH_TOKEN&v1=1
BLYNK_WRITE(V1) {
  int pinValue = param.asInt(); 
  if (pinValue == 1) {
    Serial.println("Cloud Command Received: Short Press");
    triggerPower(500);
  }
}

// --- CLOUD TRIGGER: FORCE OFF (Virtual Pin V2) ---
// Apple Shortcut URL: https://blynk.cloud/external/api/update?token=YOUR_AUTH_TOKEN&v2=1
BLYNK_WRITE(V2) {
  int pinValue = param.asInt(); 
  if (pinValue == 1) {
    Serial.println("Cloud Command Received: FORCE OFF");
    triggerPower(5000);
  }
}


void setup() {
  Serial.begin(115200);
  
  // 1. Setup Pins
  pinMode(POWER_PIN, OUTPUT_OPEN_DRAIN); 
  digitalWrite(POWER_PIN, HIGH);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);

  // 2. Initialize Wi-Fi and Blynk Background Engine
  // We use WiFi.begin and Blynk.config instead of Blynk.begin()
  // This ensures the ESP32 NEVER freezes up if the router is off during boot.
  Serial.println("Initializing background network services...");
  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN);
  Serial.println("Boot complete. Local hardware live.");
}

void loop() {
  static unsigned long lastNetworkCheck = 0;
  static unsigned long disconnectStartTime = 0; // Tracks when we went offline
  
  // 1. SMART CONNECTION MANAGER WITH BUILT-IN WATCHDOG
  if (Blynk.connected()) {
    Blynk.run(); // Only process cloud tasks if we are actively online
    disconnectStartTime = 0; // Reset the offline stopwatch because we are healthy!
  } else {
    // --- START THE STOPWATCH ---
    // The exact moment we drop offline, record the timestamp
    if (disconnectStartTime == 0) {
      disconnectStartTime = millis();
    }

    // --- THE 5-MINUTE SELF-HEAL ---
    // If we have been continuously disconnected for more than 5 minutes (300,000 ms),
    // assume a memory leak or network lockup occurred and force a clean software reboot.
    if (millis() - disconnectStartTime > 300000UL) {
      Serial.println("Disconnected for 5 continuous minutes. Forcing reboot to self-heal...");
      delay(1000);
      ESP.restart(); // Flushes RAM. Safe to do because software reboots don't trigger the power pin!
    }

    // --- BACKGROUND RECONNECTION ATTEMPTS ---
    // While waiting out the 5 minutes, still try to reconnect every 30 seconds
    if (millis() - lastNetworkCheck > 30000) {
      lastNetworkCheck = millis();
      Serial.println("Network offline. Checking status...");
      
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi link down. Re-triggering background connection...");
        WiFi.disconnect();
        WiFi.begin(ssid, pass);
      } else {
        Serial.println("WiFi OK. Attempting Blynk reconnect...");
        Blynk.connect(1000); // 1-second quick handshake attempt
      }
    }
  }

  // 2. HANDLE THE PHYSICAL CASE BUTTON
  buttonState = digitalRead(BUTTON_PIN);
  
  if (buttonState == HIGH && lastButtonState == LOW) {
    Serial.println("Physical Button Pressed!");
    triggerPower(500); 
    delay(500); // Simple debounce to prevent double-clicks
  }
  
  lastButtonState = buttonState;
}
