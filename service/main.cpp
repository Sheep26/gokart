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
#define TELEMENTRY_PIN = 6

bool telementry_running = false;

void oled_command(int fd, uint8_t cmd) {
    wiringPiI2CWriteReg8(fd, 0x00, cmd);
}

void oled_data(int fd, uint8_t data) {
    wiringPiI2CWriteReg8(fd, 0x40, data);
}

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

void Threads::display_t(int fd) {
    while (true) {
        oled_command(fd, 0xAF); // Display on.

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

    int fd = wiringPiI2CSetup(OLED_ADDR);

    if (fd < 0) {
        std::cerr << "Failed to init I2C\n";
        return -2;
    }

    // Set pin mode for telementry switch.
    pinMode(TELEMENTRY_PIN, INPUT);
    pullUpDnControl(TELEMENTRY_PIN, PUD_DOWN);

    // Create display thread.
    thread display_t(Threads::display_t, fd);
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