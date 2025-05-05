#include <iostream>
#include <wiringPi.h>
#include <data.hpp>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <format>
#include <cstdlib>
#include <atomic>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

void data_thread() {
    // Send data to server every 100ms
    while (true) {
        CURL* curl = curl_easy_init();

        if (curl) {
            string body = format(R"({
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

void ffmpeg_thread() {
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
    }
}

int main() {
    cout << "Starting gokart service." << endl;

    // Setup GPIO
    // Uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
    cout << "Initalizing GPIO" << endl;
    if (wiringPiSetupGpio() == -1) {
        cerr << "Error: Failed to initialize GPIO." << endl;
        return 1;
    }

    // Create threads
    thread ffmpeg_t(ffmpeg_thread);
    thread data_t(data_thread);

    // Detach threads to allow independent execution
    ffmpeg_t.detach();
    data_t.detach();

    // Main loop.
    while (true) {
        sleep_for(milliseconds(33));
    }

    // Return 0.
    return 0;
}