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

const int sampleRate = 4000;          // 4 kHz
const int bufferSize = 40;            // 10 ms of audio
uint8_t audioBuffer[bufferSize];
unsigned long lastSampleMicros = 0;
int bufferIndex = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TOGGLE_PIN, INPUT_PULLUP);

  // Start LoRa (433 MHz for SX1278)
  LoRa.setPins(NSS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring.");
    while (true); // Stop here if failed
  }
  
  LoRa.setSpreadingFactor(9);      // Good compromise for ~1 km
  LoRa.setSignalBandwidth(125E3);  // Wide enough for 4kHz audio
  LoRa.setCodingRate4(5);          // Faster with basic error correction
  LoRa.setPreambleLength(8);       // Default preamble
  LoRa.setTxPower(17);             // Max power
}

void loop() {
  if (digitalRead(TOGGLE_PIN) == HIGH) {
    unsigned long currentMicros = micros();
    if (currentMicros - lastSampleMicros >= 1000000UL / sampleRate) {
      lastSampleMicros = currentMicros;

      int sample = analogRead(MIC_PIN) >> 4; // 12-bit to 8-bit
      audioBuffer[bufferIndex++] = (uint8_t)sample;

      if (bufferIndex >= bufferSize) {
        LoRa.beginPacket();
        LoRa.write(audioBuffer, bufferSize);
        LoRa.endPacket(true);
        bufferIndex = 0;
      }
    }
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