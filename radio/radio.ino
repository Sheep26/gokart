#include <SPI.h>
#include <LoRa.h>

// === Pin Definitions ===
#define MIC_PIN       36  // ADC1_CH0 (MAX9814 OUT)
#define SPEAKER_PIN   25  // DAC1 (to audio amp input)
#define TOGGLE_PIN     1  // Push-to-talk (active HIGH)

#define NSS            5  // LoRa CS
#define RST           14  // LoRa RESET
#define DIO0           2  // LoRa DIO0

/* Pinout for LoRa SX1278
       +----------------------+
  GND  | 1                16 | 3.3V
  RESET| 2                15 | NSS (CS)
  DIO5 | 3                14 | SCK
  DIO3 | 4                13 | MISO
  DIO4 | 5                12 | MOSI
  DIO0 | 6                11 | DIO1
  DIO1 | 7                10 | DIO2
  GND  | 8                 9 | ANT (Antenna)
       +----------------------+
*/

void setup() {
  Serial.begin(115200);
  pinMode(TOGGLE_PIN, INPUT_PULLUP);

  // Start LoRa (433 MHz for SX1278)
  LoRa.setPins(NSS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring.");
    while (true); // Stop here if failed
  }

  // Optional: Improve range with lower bandwidth
  LoRa.setSpreadingFactor(7);           // 6–12 (higher = more range, less data rate)
  LoRa.setSignalBandwidth(62.5E3);      // Narrower = more range, less speed
}

void loop() {
    if (digitalRead(TOGGLE_PIN) == HIGH) {
        // Transmit audio sample
        int sample = analogRead(MIC_PIN) >> 4; // 0–4095 → 0–255
        LoRa.beginPacket();
        LoRa.write(sample);
        LoRa.endPacket(true); // async
        delayMicroseconds(250); // ~4kHz
    } else {
        // Check for received packet
        int packetSize = LoRa.parsePacket();
        if (packetSize > 0) {
            while (LoRa.available()) {
                uint8_t sample = LoRa.read();
                dacWrite(SPEAKER_PIN, sample);
            }
        }
    }
}