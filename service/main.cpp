#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <thread>
#include <chrono>
#include "data.h"
#include "networking.h"
#include "threads.h"
#include "ssd1306.h"
#include "OledScreen.h"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

#define TELEMENTRY_PIN = 10
#define DC = 5
#define RST = 6

bool telementry_running = false;

/*
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
    catch () {
        telementry_running = false;
    }
}

void Threads::ffmpeg_t() {
    cout << "Starting ffmpeg live video feed.";

    // Check if ffmpeg installed.
    if (system("which ffmpeg > /dev/null 2>&1") != 0) {
        cerr << "Error: ffmpeg is not installed on the system, exiting." << endl;
        return;
    }

    // Start ffmpeg.
    int ret = system("ffmpeg -f v4l2 -i /dev/video0 -f flv rtmp://gokart.sheepland.xyz/live/stream");
    if (ret != 0) {
        cerr << "Error: ffmpeg command failed with exit code " << ret << endl;
        telementry_running = false;
    }
}

void Threads::display_t() {
    OledScreen oled;

    unsigned char initcode[] = {
        // Initialisation sequence
        SSD1306_DISPLAYOFF,                    // 0xAE
        SSD1306_SETLOWCOLUMN,            // low col = 0
        SSD1306_SETHIGHCOLUMN,           // hi col = 0
        SSD1306_SETSTARTLINE,            // line #0
        SSD1306_SETCONTRAST,                   // 0x81
        0xCF,
        0xa1,                                  // setment remap 95 to 0 (?)
        SSD1306_NORMALDISPLAY,                 // 0xA6
        SSD1306_DISPLAYALLON_RESUME,           // 0xA4
        SSD1306_SETMULTIPLEX,                  // 0xA8
        0x3F,                                  // 0x3F 1/64 duty
        SSD1306_SETDISPLAYOFFSET,              // 0xD3
        0x0,                                   // no offset
        SSD1306_SETDISPLAYCLOCKDIV,            // 0xD5
        0xF0,                                  // the suggested ratio 0x80
        SSD1306_SETPRECHARGE,                  // 0xd9
        0xF1,
        SSD1306_SETCOMPINS,                    // 0xDA
        0x12,                                  // disable COM left/right remap
        SSD1306_SETVCOMDETECT,                 // 0xDB
        0x40,                                  // 0x20 is default?
        SSD1306_MEMORYMODE,                    // 0x20
        0x00,                                  // 0x0 act like ks0108
        SSD1306_SEGREMAP,
        SSD1306_COMSCANDEC,
        SSD1306_CHARGEPUMP,                    //0x8D
        0x14,

        // Enabled the OLED panel
        SSD1306_DISPLAYON
    };

    unsigned char poscode[] = {
        SSD1306_SETLOWCOLUMN,            // low col = 0
        SSD1306_SETHIGHCOLUMN,           // hi col = 0
        SSD1306_SETSTARTLINE            // line #0
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
        digitalWrite(DC, LOW);
        wiringPiSPIDataRW(0, poscode, 3);
        digitalWrite(DC, HIGH);
        wiringPiSPIDataRW(0, oled.pix_buf, 1024);

        oled.clear();

        oled.draw_circle(0,0,2048,1);
        sleep_for(milliseconds(33));
    }
}

void create_threads() {
    telementry_running = true;

    // Create threads
    thread ffmpeg_t(Threads::ffmpeg_t);
    thread data_t(Threads::data_t);

    // Detach threads to allow independent execution
    ffmpeg_t.detach();
    data_t.detach();
}

int main() {
    cout << "Starting gokart service." << endl;

    // Setup GPIO
    // Uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
    cout << "Initalizing GPIO" << endl;
    if (wiringPiSetupGpio() == -1) {
        cerr << "Error: Failed to initialize GPIO." << endl;
        return -1;
    }

    // Set pin mode for telementry switch.
    pinMode(TELEMENTRY_PIN, INPUT);
    pullUpDnControl(TELEMENTRY_PIN, PUD_DOWN);

    // Create display thread.
    thread display_t(Threads::display_t);
    display_t.detach();

    // Check if telementry enabled.
    if (digitalRead(TELEMENTRY_PIN) == HIGH){
        if (!Networking::check_network()) {
            cout << "Waiting for network." << endl;
            Networking::wait_for_network();
        }
    
        create_threads();
    
        while (true) {
            Networking::wait_for_network();
    
            if (!telementry_running) {
                create_threads();
            }
        }
    }

    // Return 0.
    return 0;
}