#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <thread>
#include <chrono>
#include "data.h"
#include "networking.h"
#include "threads.h"
#include "ssd1309.h"
#include "OledScreen.h"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

#define TELEMENTRY_PIN 10
#define RADIO_BUTTON 11
#define DC 5
#define RST 6

bool telementry_running = false;

/*
 https://www.hpinfotech.ro/SSD1309.pdf - Datasheet
 Display pinout.
 GND    --- Gnd
 VCC    --- 3.3V
 DO     --- SCLK(Pin#23)
 DI     --- MOSI(Pin#19)
 RES/RST    --- GPIO18(Pin#6) (You can use Any Pin)
 DC     --- GPIO17(Pin#5) (You can use Any Pin)
 CS     --- CS0(Pin#24)
*/

void Threads::data_t() {
    // Send data to server every 100ms
    try {
        while (true) {
            if (!Networking::check_network() || !telementry_running) {
                return;
            }

            CURL* curl = curl_easy_init();
    
            if (curl) {
                string body = fmt::format(R"({
                    "speed": {},
                    "speed_avg": {},
                    "speed_max": {},
                    "rpm": {},
                    "rpm_avg": {},
                    "rpm_max": {},
                    "power": {},
                    "power_avg": {},
                    "power_max": {},
                    "throttle": {},
                    "throttle_avg": {},
                    "throttle_max": {}
                })", data.speed.current, data.speed.avg, data.speed.max, data.rpm.current, data.rpm.avg, data.rpm.max, data.power.current, data.power.avg, data.power.max, data.throttle.current, data.throttle.avg, data.throttle.max);
                const char* body_cstr = body.c_str();
    
                curl_easy_setopt(curl, CURLOPT_URL, "https://gokart.sheepland.xyz/api/update_data");
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_cstr);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(body_cstr));
        
                CURLcode res = curl_easy_perform(curl);
    
                // Clean up memory.
                curl_easy_cleanup(curl);
        
                if (res == CURLE_OK) {
                    std::cout << "Response:\n" << std::endl;
                } else {
                    std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
                }
            }
    
            sleep_for(milliseconds(100));
        }
    }
    catch (...) {
        telementry_running = false;
        return;
    }
}

void Threads::ffmpeg_t() {
    cout << "Starting ffmpeg live video feed.";

    // Start ffmpeg.
    int ret = system("ffmpeg -f v4l2 -i /dev/video0 -f flv rtmp://gokart.sheepland.xyz/live/stream");
    if (ret != 0) {
        cerr << "Error: ffmpeg command failed with exit code " << ret << endl;
    }
    
    telementry_running = false;
}

void Threads::display_t() {
    OledScreen oled;

    unsigned char initcode[] = {
        DISPLAY_OFF,                     // 0xAE: Turn display off
        SET_DISPLAY_CLOCK_DEVICE_RATIO,  0x80, // 0xD5: Set clock divide ratio/oscillator frequency
        SET_MULTIPLEX_RATIO,             0x3F, // 0xA8: Set multiplex ratio (1/64 duty cycle)
        SET_DISPLAY_OFFSET,              0x00, // 0xD3: Set display offset to 0
        SET_DISPLAY_START_LINE | 0x00,         // 0x40: Set display start line to 0
        SET_SEGMENT_REMAP_ON,                  // 0xA1: Set segment re-map (column address 127 is mapped to SEG0)
        SET_COM_OUTPUT_SCAN_DIRECTION_8,       // 0xC8: Set COM output scan direction (remapped mode)
        SET_COM_PINS_HARDWARE_CONFIG,    0x12, // 0xDA: Set COM pins hardware configuration
        SET_CONTRAST,                   0x7F,  // 0x81: Set contrast control to medium
        SET_PRECHANGE_PERIOD,           0xF1,  // 0xD9: Set pre-charge period
        SET_VCOMH_DESELECT_LEVEL,       0x40,  // 0xDB: Set VCOMH deselect level
        SET_MEMORY_ADDRESS_MODE,        0x00,  // 0x20: Set memory addressing mode to horizontal
        SET_LOW_COLUMN,                       // 0x00: Set lower column start address
        SET_HIGH_COLUMN,                      // 0x10: Set higher column start address
        SET_CHARGE_PUMP,               0x14,  // 0x8D: Enable charge pump regulator
        DEACTIVATE_SCROLL,                   // 0x2E: Deactivate scrolling
        SET_NORMAL_DISPLAY,                  // 0xA6: Set normal display mode (not inverted)
        DISPLAY_ON 
    };

    unsigned char poscode[] = {
        SET_LOW_COLUMN,            // low col = 0
        SET_HIGH_COLUMN,           // hi col = 0
        SET_DISPLAY_START_LINE     // line #0
    };

    pinMode (DC, OUTPUT);
    pinMode (RST, OUTPUT);
    wiringPiSPISetup(0, 8*1000*1000);
    
    // reset
    digitalWrite(RST,  LOW);
    sleep_for(milliseconds(50));
    digitalWrite(RST,  HIGH);
    
    // init
    digitalWrite(DC, LOW);
    wiringPiSPIDataRW(0, initcode, 28);

    while (true) {
        // Oled data.
        // Clear screen.
        oled.clear();

        // Print Hello World to display.
        oled.println("Hello world!", 16, 16, 32, 1);
        
        // Send data to display.
        digitalWrite(DC, LOW);
        wiringPiSPIDataRW(0, poscode, 3);
        digitalWrite(DC, HIGH);
        wiringPiSPIDataRW(0, oled.pix_buf, 1024);

        sleep_for(milliseconds(33));
    }
}

void Threads::radio_t() {
    // Radio loop.
    while (true) {
        if (digitalRead(RADIO_BUTTON) == HIGH) {
            // Radio, this is GPIO 4 or pin 7.
           system("arecord -fS16_LE -r 44100 -Dplughw:1,0 -c 2 - | /usr/bin/pi_fm_rds -freq 103.1 -audio -");
        }
    }
}

void start_telementry() {
    telementry_running = true;

    // Create threads
    thread ffmpeg_thread(Threads::ffmpeg_t);
    thread data_thread(Threads::data_t);
    thread radio_thread(Threads::radio_t);

    // Detach threads to allow independent execution
    ffmpeg_thread.detach();
    data_thread.detach();
    radio_thread.detach();
}

int main() {
    cout << "Starting gokart service." << endl;

    // Setup GPIO
    cout << "Initalizing GPIO" << endl;
    if (wiringPiSetupPinType(WPI_PIN_BCM) == -1) {
        cerr << "Error: Failed to initialize GPIO." << endl;
        return -1;
    }

    // Set pin modes.
    pinMode(TELEMENTRY_PIN, INPUT);
    pinMode(RADIO_BUTTON, INPUT);
    pinMode(RADIO_PIN, OUTPUT);

    // Create display thread.
    thread display_t(Threads::display_t);
    display_t.detach();

    // Check if telementry enabled.
    if (digitalRead(TELEMENTRY_PIN) == HIGH){
        cout << "Waiting for network." << endl;
        cout << "Network connected, took " << Networking::wait_for_network() << "s" << endl;
    
        while (true) {
            if (Networking::check_network() && !telementry_running) {
                start_telementry();
            }

            sleep_for(milliseconds(10000));
        }
    }

    // Return 0.
    return 0;
}