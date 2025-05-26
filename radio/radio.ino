#include <SPI.h>
#include <LoRa.h>

// === Pin Definitions ===
#define MIC_PIN       36  // ADC1_CH0 (MAX9814 OUT)
#define SPEAKER_PIN   25  // DAC1 (to audio amp input)
#define TOGGLE_PIN     1  // Push-to-talk (active HIGH)

#define NSS            5  // LoRa CS
#define RST           14  // LoRa RESET
#define DIO0           2  // LoRa DIO0

/*
| Component          | Signal      | ESP32 Pin | Notes                                      |
| ------------------ | ----------- | --------- | ------------------------------------------ |
| **MAX9814 Mic**    | VCC         | 3.3V      | Use only 3.3V (MAX9814 is 3.3V-compatible) |
|                    | GND         | GND       | Ground                                     |
|                    | OUT         | GPIO 36   | Analog input (ADC1\_CH0)                   |

| **Speaker / Amp**  | INPUT       | GPIO 25   | DAC1 output to amplifier                   |

| **Push-to-Talk**   | INPUT       | GPIO 1    | Use a pull-up resistor or `INPUT_PULLUP`   |
|                    | INPUT       | GND       | Connect to ground                          |

| **SX1278 LoRa**    | VCC         | 3.3V      | Strictly 3.3V — **do not use 5V**          |
|                    | GND         | GND       | Ground                                     |
|                    | SCK         | GPIO 18   | SPI Clock                                  |
|                    | MISO        | GPIO 19   | SPI MISO                                   |
|                    | MOSI        | GPIO 23   | SPI MOSI                                   |
|                    | NSS (CS)    | GPIO 5    | LoRa CS (aka NSS or SS)                    |
|                    | RESET       | GPIO 14   | LoRa hardware reset                        |
|                    | DIO0        | GPIO 2    | Used for packet ready                      |
|                    | ANT         | Antenna   | Connect proper antenna                     |
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