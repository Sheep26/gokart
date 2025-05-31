#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <sstream>
#include <vector>

#include "data.h"
#include "networking.h"
#include "threads.h"
#include "ssd1309.h"
#include "OledScreen.h"

#define TELEMENTRY_PIN 10
#define DC 5
#define RST 6

std::atomic<bool> telementry_running;

struct Server { // I don't want to have to deal with memory realloc, lets use strings.
    std::string ip;
    std::string username;
    std::string passwd;
    std::string session;
    std::string id;
};

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

Server server;

std::vector<std::string> split_string(const std::string& input, const char* delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    
    return tokens;
}

void Threads::data_t() {
    // Send data to server every 100ms
    
    while (true) {
        if (!Networking::check_network() || !telementry_running) {
            return;
        }

            /*CURL* curl = curl_easy_init();
    
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
    
                curl_easy_setopt(curl, CURLOPT_URL, ("https://" + server.ip + "/api/update_data").c_str());
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_cstr);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(body_cstr));
                
                // Set headers
                struct curl_slist* headers = nullptr;
                headers = curl_slist_append(headers, ("id: " + server.id).c_str());
                headers = curl_slist_append(headers, ("session: " + server.session).c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                
                CURLcode res = curl_easy_perform(curl);
    
                // Clean up memory.
                curl_easy_cleanup(curl);
        
                if (res == CURLE_OK) {
                    cout << "Response:\n" << endl;
                } else {
                    cerr << "Request failed: " << curl_easy_strerror(res) << endl;
                }
            }*/

            // Set headers
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("id: " + server.id).c_str());
        headers = curl_slist_append(headers, ("session: " + server.session).c_str());

        if (!Networking::send_http_request("https://" + server.ip + "/api/update_data", fmt::format(R"({
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
            })", data.speed.current, data.speed.avg, data.speed.max, data.rpm.current, data.rpm.avg, data.rpm.max, data.power.current, data.power.avg, data.power.max, data.throttle.current, data.throttle.avg, data.throttle.max),
            true, headers)) {
            std::cerr << "Error: Failed to send telemetry data." << std::endl;
        } else {
            telementry_running = false;
            return;
        }
 
        // Sleep for 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Threads::ffmpeg_t() {
    cout << "Starting ffmpeg live video feed.";

    // Create a named pipe for ffmpeg drawtext reloading
    system("mkfifo /tmp/ffmpeg_overlay.txt");

    // Start a thread to update the overlay text file with telemetry
    std::thread overlay_thread([]() {
        while (telementry_running) {
            char overlay[128];
            snprintf(overlay, sizeof(overlay), "Speed:%dkmph\nRPM:%d\nPower:%dw\nThrottle:%d%", data.speed.current, data.rpm.current, data.power.current, data.throttle.current);
            FILE* f = fopen("/tmp/ffmpeg_overlay.txt", "w");
            if (f) {
                fprintf(f, "%s", overlay);
                fclose(f);
            }

            // Sleep for 100ms to allow ffmpeg to reload the file
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    overlay_thread.detach();

    // ffmpeg command with drawtext filter using reloading file
    std::string cmd =
        "ffmpeg -f v4l2 -i /dev/video0 "
        "-vf \"drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf:"
        "textfile=/tmp/ffmpeg_overlay.txt:reload=1:x=10:y=10:fontsize=24:fontcolor=white:box=1:boxcolor=black@0.75\" "
        "-f flv rtmp://" + server.ip + ":1935/live/stream?id=" + server.id + "&session=" + server.session;
    
    std::cout << "Running command: " << cmd << std::endl;

    int ret = system(cmd.c_str());

    telementry_running = false;
    // Remove the named pipe after use.
    system("rm -f /tmp/ffmpeg_overlay.txt");

    if (ret != 0) {
        std::cerr << "Error: ffmpeg command failed with exit code " << ret << std::endl;
    }
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
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    digitalWrite(RST,  HIGH);
    
    // init
    digitalWrite(DC, LOW);
    wiringPiSPIDataRW(0, initcode, 28); // Send init commands, 28 bytes long
    digitalWrite(DC, HIGH);

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

        digitalWrite(DC, LOW);
        wiringPiSPIDataRW(0, oled.pix_buf, 1024);
        digitalWrite(DC, HIGH);

        // Sleep for 33ms to achieve ~30 FPS
        // This is a rough approximation, actual frame rate may vary.
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

void start_telementry() {
    telementry_running = true;

    // Create threads
    std::thread ffmpeg_thread(Threads::ffmpeg_t);
    std::thread data_thread(Threads::data_t);

    // Detach threads to allow independent execution
    ffmpeg_thread.detach();
    data_thread.detach();
}

int main(int argc, char **argv) {
    std::cout << "Starting gokart service." << std::endl;

    // Configure server.
    std::cout << "Reading environment varibles" << std::endl;
    server.ip = (std::string) getenv("SERVERIP");
    server.username = (std::string) getenv("SERVERUSERNAME");
    server.passwd = (std::string) getenv("SERVERPASSWD");
    std::cout << "Server configured at " << server.ip << std::endl;
    std::cout << "Waiting for network" << std::endl;
    std::cout << "Network connected took " << std::to_string(Networking::wait_for_network()) << "s" << std::endl;
    std::cout << "attempting login" << std::endl;

    // Login.
    struct curl_slist* login_headers = nullptr;
    login_headers = curl_slist_append(login_headers, ("username: " + server.username).c_str());
    login_headers = curl_slist_append(login_headers, ("passwd: " + server.passwd).c_str());
    HTTP_Request login_request = Networking::send_http_request("https://" + server.ip + "/api/update_data", nullptr, false, login_headers);
    if (login_request.status_code == 200) {
        std::vector<std::string> parts = split_string(login_request.text, ",");
        server.id = parts[0];
        server.session = parts[1];
    } else {
        std::cerr << "Login failed" << std::endl;
    }

    // Setup GPIO
    std::cout << "Initalizing GPIO" << std::endl;
    if (wiringPiSetupPinType(WPI_PIN_BCM) == -1) {
        std::cerr << "Error: Failed to initialize GPIO." << std::endl;
        return -1;
    }

    // Set pin modes.
    pinMode(TELEMENTRY_PIN, INPUT);

    // Create display thread.
    std::display_thread(Threads::display_t);

    display_thread.detach();

    // Check if telementry enabled.
    if (digitalRead(TELEMENTRY_PIN) == HIGH){
        std::cout << "Waiting for network." << std::endl;
        std::cout << "Network connected, took " << Networking::wait_for_network() << "s" << std::endl;
    
        while (true) {
            if (Networking::check_network() && !telementry_running) {
                start_telementry();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        }
    }

    // Return 0.
    return 0;
}