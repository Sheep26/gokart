#include <iostream>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <thread>
#include <chrono>
#include "data.h"
#include "networking.h"
#include "threads.h"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

#define OLED_ADDR 0x3C
#define OLED_CMD 0x00
#define OLED_DATA 0x40
#define TELEMENTRY_PIN = 6

bool telementry_running = false;

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

void Threads::display_t(int i2cd) {
    // SSD1306 initialization commands
    unsigned char init_commands[] = {
        0xAE,       // Display OFF
        0xA8, 0x3F, // Set multiplex ratio (1 to 64)
        0x21, 0x00, 0x7F, // Set column address range (0 to 127 for 128-pixel width)
        0xD3, 0x00, // Set display offset to 0
        0x40,       // Set display start line to 0
        0xA1,       // Set segment re-map (column address 127 is mapped to SEG0)
        0xC8,       // Set COM output scan direction (remapped mode)
        0xD5, 0x80, // Set display clock divide ratio/oscillator frequency
        0xDA, 0x12, // Set COM pins hardware configuration
        0x81, 0x7F, // Set contrast control (medium contrast)
        0xA4,       // Enable display output (resume to RAM content display)
        0xDB, 0x40, // Set VCOMH deselect level
        0x20, 0x00, // Set memory addressing mode (horizontal addressing mode)
        0x8D, 0x14, // Enable charge pump regulator
        0xAF        // Display ON
    };

    // Send initialization commands
    for (unsigned char cmd : init_commands) {
        wiringPiI2CWriteReg8(i2cd, OLED_CMD, cmd);
    }

    // Clear the display (optional)
    for (int i = 0; i < 1024; i++) { // 128x64 pixels = 1024 bytes
        wiringPiI2CWriteReg8(i2cd, OLED_DATA, 0x00);
    }

    while (true) {
        // Example: Display a simple pattern or update the screen
        for (int i = 0; i < 128; i++) {
            wiringPiI2CWriteReg8(i2cd, OLED_DATA, 0xFF); // Example: Fill one column
        }

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

    int i2cd = wiringPiI2CSetup(OLED_ADDR);

    if (i2cd < 0) {
        std::cerr << "Failed to init I2C\n";
        return -2;
    }

    // Set pin mode for telementry switch.
    pinMode(TELEMENTRY_PIN, INPUT);
    pullUpDnControl(TELEMENTRY_PIN, PUD_DOWN);

    // Create display thread.
    thread display_t(Threads::display_t, i2cd);
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