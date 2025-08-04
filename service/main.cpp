#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <sstream>
#include <vector>
#include <string>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <unistd.h>
#include <sys/socket.h>

#include "data.h"
#include "networking.h"
#include "threads.h"
#include "ssd1309.h"
#include "OledScreen.h"
#include "commandListener.h"

#define TELEMENTRY_PIN 10
#define DISPLAY_PIN 11
#define SHUTDOWN_PIN 12
#define DC 17
#define RST 18

/*
 https://www.hpinfotech.ro/SSD1309.pdf - Datasheet
 Display pinout.
 GND    --- Gnd
 VCC    --- 3.3V
 DO     --- SCLK
 DI     --- MOSI
 RES/RST    --- GPIO18(Pin#12)
 DC     --- GPIO17(Pin#11)
 CS     --- CE0_N OR GND
*/

/*
 ESP32 Pinout
 5V  --- 5V
 GND --- GND
*/

struct Server {
    std::string ip;
    std::string username;
    std::string passwd;
    std::string session;
    std::string id;
};

Server server;
CommandListener commandListener;
std::atomic<bool> telementry_running;
std::atomic<bool> display_on;
std::atomic<bool> shutting_down;

std::vector<std::string> split_string(const std::string& input, char delimiter) {
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
    
    while (telementry_running) {
            /*CURL* curl = curl_easy_init();
    
            if (curl) {
                string body = fmt::format(R"({{
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
    }})", data.speed.current, data.speed.avg, data.speed.max, data.rpm.current, data.rpm.avg, data.rpm.max, data.power.current, data.power.avg, data.power.max, data.throttle.current, data.throttle.avg, data.throttle.max);
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

        if (Networking::send_http_request("https://" + server.ip + "/api/update_data", fmt::format(R"({{
                "speed": {},
                "rpm": {},
                "batteryVolt": {},
                "batteryPercent": {},
            }})", data.speed, data.rpm, data.batteryVolt, data.batteryPercent),
            true, headers).status_code != 200) {
            std::cerr << "Error: Failed to send telemetry data.\n";
            telementry_running = false;
            break;
        }
 
        // Sleep for 1000ms
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void Threads::ffmpeg_t() {
    std::cout << "Starting ffmpeg live video feed.";

    // Create a named pipe for ffmpeg drawtext reloading
    system("touch /tmp/ffmpeg_overlay.txt");

    // Start a thread to update the overlay text file with telemetry
    std::thread overlay_thread([]() {
        FILE* f = fopen("/tmp/ffmpeg_overlay.txt", "w");
        
        while (telementry_running) {
            // Check if file handle exists.
            if (!f) {
                telementry_running = false;

                break;
            }

            char overlay[128];
            snprintf(overlay, sizeof(overlay), "Num:%d\nSpeed:%dkmph\nrpm:%d\nBattery:V%d", data.num, data.speed, data.rpm, data.battery);
            
            fprintf(f, "%s", overlay);

            // Sleep for 100ms second to allow ffmpeg to reload the file
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        fclose(f);
    });

    overlay_thread.detach();

    // ffmpeg command with drawtext filter using reloading file.
    // also does 720p@60
    std::string cmd =
        "ffmpeg -f v4l2 -framerate 60 -video_size 1280x720 -i /dev/video0 "
        "-vf \"drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf:"
        "textfile=/tmp/ffmpeg_overlay.txt:reload=1:x=10:y=10:fontsize=12:fontcolor=white:box=1:boxcolor=black@0.50\" "
        "-f flv rtmp://" + server.ip + ":1935/live/stream?id=" + server.id + "&session=" + server.session;
    
    std::cout << "Running command: " << cmd << "\n";

    int ret = system(cmd.c_str());

    telementry_running = false;
    // Remove the named pipe after use.
    system("rm -f /tmp/ffmpeg_overlay.txt");

    if (ret != 0) {
        std::cerr << "Error: ffmpeg command failed with exit code " << ret << "\n";
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

        if (!shutting_down) {
            // Print Hello World to display.
            oled.println("Hello world!", 16, 16, 32, 1);
        } else {
            oled.println("Shutting down.", 16, 16, 32, 1);
        }
        
        // Send data to display.
        digitalWrite(DC, LOW);
        wiringPiSPIDataRW(0, poscode, 3);
        digitalWrite(DC, HIGH);

        digitalWrite(DC, LOW);
        wiringPiSPIDataRW(0, oled.pix_buf, 1024);
        digitalWrite(DC, HIGH);

        if (!display_on) {
            // Power off the display.
            digitalWrite(DC, LOW);
            wiringPiSPIDataRW(0, 0xAE, 1);
            digitalWrite(DC, HIGH);

            break;
        }

        // Sleep for 33ms to achieve ~30 FPS
        // This is a rough approximation, actual frame rate may vary.
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

void Threads::bluetooth_server() {
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    int server_sock, client_sock;
    socklen_t opt = sizeof(rem_addr);

    // Create Bluetooth socket
    server_sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = 1;
    bind(server_sock, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
    listen(server_sock, 1);

    std::cout << "[BT] Listening on RFCOMM channel 1...\n";

    while (telementry_running) {
        char client_addr[18] = { 0 };
        client_sock = accept(server_sock, (struct sockaddr *)&rem_addr, &opt);
        ba2str(&rem_addr.rc_bdaddr, client_addr);
        std::cout << "[BT] Client connected: " << client_addr << "\n";

        char buf[1024];
        int bytes_read;
        while ((bytes_read = read(client_sock, buf, sizeof(buf) - 1)) > 0) {
            buf[bytes_read] = '\0';
            std::string cmd(buf);
            // Trim newline if needed
            cmd.erase(cmd.find_last_not_of("\r\n") + 1);
            std::vector<std::string> parts = split_string(cmd, ' ');
            std::vector<std::string> args(parts.begin() + 1, parts.end());
            std::string response = commandListener.handle_command(args[0], args);
            std::cout << "[BT]" << response << "\n";
            write(client_sock, response.c_str(), response.length());
        }

        std::cout << "[BT] Client disconnected.\n";
        close(client_sock);
    }

    close(server_sock);
    std::cout << "[BT] Server stopped.\n";
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

void start_display_thread() {
    std::thread display_thread(Threads::display_t);
    display_thread.detach();

    display_on = true;
}

int main(int argc, char **argv) {
    std::cout << "Starting gokart service.\n";

    shutting_down = false;

    // Setup GPIO
    std::cout << "Init GPIO.\n";
    if (wiringPiSetupPinType(WPI_PIN_BCM) == -1) {
        std::cerr << "Error: Failed to initialize GPIO.\n";
        return -1;
    }
    
    std::cout << "Init SPI.\n";
    if (wiringPiSPISetup(0, 8*1000*1000) == -1) {
        std::cerr << "Error: Failed to initialize SPI.\n";
        return -1;
    }

    // Configure server.
    std::cout << "Reading environment varibles.\n";
    server.ip = (std::string) getenv("SERVERIP");
    server.username = (std::string) getenv("SERVERUSERNAME");
    server.passwd = (std::string) getenv("SERVERPASSWD");
    std::cout << "Server configured at " << server.ip << "\n";
    
    // Set race number.
    data.num = (std::string) getenv("RACENUM");

    // Set pin modes.
    pinMode(TELEMENTRY_PIN, INPUT_PULLUP);
    pinMode(DISPLAY_PIN, INPUT_PULLUP);
    pinMode(SHUTDOWN_PIN, INPUT_PULLUP);

    // Create display thread.
    start_display_thread();

    while (true) {
        if (digitalRead(SHUTDOWN_PIN) == LOW) {
            shutting_down = true;

            // Wait a second.
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            system("shutdown now");
        }

        if (!telementry_running && digitalRead(TELEMENTRY_PIN) == LOW) {
            std::cout << "Starting bluetooth server.\n";
            std::thread bluetooth_thread(Threads::bluetooth_server);
            bluetooth_thread.detach();

            std::cout << "Waiting for network.\n";
            std::cout << "Network connected, took " << Networking::wait_for_network() << "s.\n";

            // Login to server.
            std::cout << "Waiting for network.\n";
            std::cout << "Network connected took " << std::to_string(Networking::wait_for_network()) << "s.\n";
            std::cout << "Attempting login.\n";

            // Login.
            struct curl_slist* login_headers = nullptr;
            login_headers = curl_slist_append(login_headers, ("username: " + server.username).c_str());
            login_headers = curl_slist_append(login_headers, ("passwd: " + server.passwd).c_str());
            HTTP_Request login_request = Networking::send_http_request("https://" + server.ip + "/api/update_data", nullptr, false, login_headers);
            if (login_request.status_code == 200) {
                std::vector<std::string> parts = split_string(login_request.text, ',');
                server.id = parts[0];
                server.session = parts[1];
            } else {
                std::cerr << "Login failed\n";
                return 1;
            }

            std::cout << "Starting telementry.\n";
            start_telementry();
        }

        if ((digitalRead(TELEMENTRY_PIN) == LOW) != display_on) {
            if (!display_on) {
                start_display_thread();
            }

            display_on = digitalRead(TELEMENTRY_PIN) == LOW;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }

    // Return 0.
    return 0;
}