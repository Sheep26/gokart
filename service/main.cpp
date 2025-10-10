#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <wiringSerial.h>
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
#include <fmt/core.h>
#include <cstdio>
#include <string>
#include <cstring>

#include "data.h"
#include "networking.h"
#include "commandListener.h"
#include "TinyGPS++.h"

#define TELEMENTRY_PIN 5
#define SHUTDOWN_PIN 6

/*
 https://www.hpinfotech.ro/SSD1309.pdf - Datasheet
 Display pinout.
 GND    --- Gnd
 VCC    --- 3.3V
 SCLK   --- SCLK
 SDA    --- MOSI
 RES    --- GPIO22
 DC     --- GPIO27
 CS     --- CE0,CE1 OR GND
*/

/*
 ESP32 Pinout
 5V  --- 5V
 GND --- GND
*/

struct Server {
    std::string ip;
    std::string rtmp_ip;
    std::string username;
    std::string passwd;
    std::string session;
    std::string id;
};

class Threads {
public:
    static void data_t();
    static void ffmpeg_t();
    static void display_t();
    static void bluetooth_server();
    static void serial_thread();
};

Server server;
CommandListener commandListener;
TinyGPSPlus gps;

std::atomic<bool> telementry_running = false;
std::atomic<bool> shutting_down = false;
bool bluetooth_running = false;
int gps_serial;

std::string safe_getenv(const char* name) {
    const char* val = getenv(name);

    if (!val) {
        std::cerr << "Missing environment variable: " << name << "\n";
        return "";
    }

    return std::string(val);
};

std::vector<std::string> split_string(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    
    return tokens;
}

void Threads::serial_thread() {
    std::cout << "Serial started.\n";
    while(true) {
        while (serialDataAvail(gps_serial)) {
            gps.encode(serialGetchar(gps_serial));
        }

        data.speed = gps.speed.kmph();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Threads::data_t() {
    // Send data to server every 100ms
    
    while (telementry_running) {
        // Set headers
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("id: " + server.id).c_str());
        headers = curl_slist_append(headers, ("session: " + server.session).c_str());

        std::string send_data = fmt::format(R"({{"num": {},"speed": {},"satellites": {},"batteryVolt": {},"batteryPercent": {}}})",
            data.num, (int) data.speed, gps.satellites.value(), data.batteryVolt, data.batteryPercent);
        
        std::cout << "Sending Data: \n" << send_data << "\n";
        
        // Make the http request.
        HTTP_Request update_request = Networking::send_http_request(server.ip + "/api/update_data", send_data, true, false, &headers);

        if (update_request.status_code != 200) {
            std::cerr << "Error: Failed to send telemetry data.\n";
            telementry_running = false;
            break;
        }
 
        // Sleep for 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
            snprintf(overlay, sizeof(overlay), "Num:%d\nSpeed:%dkmph\n\nBattery:%d%\nVoltage:%dV", data.num, data.speed, data.batteryPercent, data.batteryVolt);
            
            fprintf(f, "%s", overlay);

            // Sleep for 100ms second to allow ffmpeg to reload the file
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        fclose(f);
    });

    overlay_thread.detach();

    // ffmpeg command with drawtext filter using reloading file.
    // 720p@60
    std::string cmd =
        "ffmpeg -f v4l2 -framerate 60 -video_size 1280x720 -i /dev/video0 "
        "-f alsa -i default "
        "-b:v 2000k -maxrate 2000k -c:v libx264 -bufsize 4000k -c:a aac -b:a 192k"
        "-vf \"drawtext=fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf:"
        "textfile=/tmp/ffmpeg_overlay.txt:reload=1:x=10:y=10:fontsize=12:fontcolor=white:box=1:boxcolor=black@0.50\" "
        "-f flv rtmp://" + server.rtmp_ip + ":1935/live/stream?id=" + server.id + "&session=" + server.session;
    
    std::cout << "Running command: " << cmd << "\n";

    int ret = system(cmd.c_str());

    //telementry_running = false;
    // Remove the named pipe after use.
    system("rm -f /tmp/ffmpeg_overlay.txt");

    if (ret != 0) {
        std::cerr << "Error: ffmpeg command failed with exit code " << ret << "\n";
    }
}

void Threads::bluetooth_server() {
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    int server_sock, client_sock;
    socklen_t opt = sizeof(rem_addr);

    // Create Bluetooth socket
    server_sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    loc_addr.rc_family = AF_BLUETOOTH;
    bacpy(&loc_addr.rc_bdaddr, BDADDR_ANY);
    loc_addr.rc_channel = 1;
    bind(server_sock, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
    listen(server_sock, 1);

    std::cout << "[BT] Listening on RFCOMM channel 1...\n";

    while (true) {
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
            std::string response = commandListener.handle_command(args);
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

int main(int argc, char **argv) {
    std::cout << "Starting gokart service.\n";
    shutting_down = false;

    // Setup GPIO
    std::cout << "Init GPIO.\n";
    if (wiringPiSetupPinType(WPI_PIN_BCM) == -1) {
        std::cerr << "Error: Failed to initialize GPIO.\n";
        return 1;
    }

    // Open serial port (replace /dev/ttyS0 with /dev/ttyAMA0 if needed)
    std::cout << "Init Serial.\n";
    gps_serial = serialOpen("/dev/serial0", 9600);

    if (gps_serial < 0) {
        std::cout << "Unable to open /dev/serial0\n";
    }

    if (gps_serial >= 0) {
        std::thread serial_thread(Threads::serial_thread);

        serial_thread.detach();
    }

    // Configure server.
    std::cout << "Reading environment varibles.\n";
    server.ip = safe_getenv("SERVERIP");
    server.rtmp_ip = safe_getenv("RTMPSERVERIP");
    server.username = safe_getenv("SERVERUSERNAME");
    server.passwd = safe_getenv("SERVERPASSWD");

    std::cout << "Server configured at " << server.ip << "\n";
    
    // Set race number.
    try {
        data.num = std::stoi(safe_getenv("RACENUM"));
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Out of range: " << e.what() << std::endl;
    }
    

    // Set pin modes.
    pinMode(TELEMENTRY_PIN, INPUT);
    pinMode(SHUTDOWN_PIN, INPUT);
    pullUpDnControl(TELEMENTRY_PIN, PUD_UP);
    pullUpDnControl(SHUTDOWN_PIN, PUD_UP);

    while (true) {
        if (digitalRead(SHUTDOWN_PIN) == LOW) {
            shutting_down = true;

            // Wait a second.
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            system("shutdown now");
        }

        if (!telementry_running && digitalRead(TELEMENTRY_PIN) == LOW) {
            if (!bluetooth_running) {
                // Start bluetooth server.
                std::cout << "Starting bluetooth server.\n";
                std::thread bluetooth_thread(Threads::bluetooth_server);
                bluetooth_thread.detach();

                bluetooth_running = true;
            }

            std::cout << "Checking wifi.\n";
            if (!Networking::wifi_enabled()) {
                std::cout << "Wifi disabled, enabling wifi.\n";
                Networking::set_wifi(true);
            }

            // Scan wifi.
            std::cout << "Scanning network.\n";
            Networking::scan_wifi();
            std::cout << "Scan complete.\n";

            // Wait for network.
            std::cout << "Waiting for network.\n";
            std::cout << "You can connect to network through bluetooth terminal.\n";
            std::cout << "Network connected, took " << Networking::wait_for_network() << "s.\n";
            std::cout << "Attempting login.\n";

            // Login.
            struct curl_slist* login_headers = nullptr;

            std::string user_header = "username: " + server.username;
            std::string pass_header = "passwd: " + server.passwd;
            
            login_headers = curl_slist_append(login_headers, user_header.c_str());
            login_headers = curl_slist_append(login_headers, pass_header.c_str());
            HTTP_Request login_request = Networking::send_http_request(server.ip + "/api/login", "", false, false, &login_headers);
            if (login_request.status_code == 200) {
                std::cout << "Login successful.\n";

                std::vector<std::string> parts = split_string(login_request.text, ',');
                server.id = parts[0];
                server.session = parts[1];

                std::cout << "Id: " << server.id << "\n";
                std::cout << "Session: " << server.session << "\n";

                std::cout << "Starting telementry.\n";
                start_telementry();
            } else {
                std::cerr << "Login failed\n";
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }

    // Return 0.
    serialClose(gps_serial);
    return 0;
}