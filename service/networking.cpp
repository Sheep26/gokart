#include "networking.h"

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

HTTP_Request Networking::send_http_request(const std::string url, const std::string body, const bool is_post, const bool is_json, struct curl_slist* headers) {
    CURL* curl = curl_easy_init();
    HTTP_Request response = {"", 0};

    if (curl) {
        std::string readBuffer;

        if (is_json) {
            headers = curl_slist_append(headers, "Content-Type: application/json");
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        if (is_post) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
        }

        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            response.status_code = http_code;
            response.text = readBuffer;
        } else {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
            response.status_code = -1;
        }

        if (headers) {
            curl_slist_free_all(headers);
        }

        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Error initializing CURL." << "\n";
        response.status_code = -1;
    }

    return response;
}

bool Networking::check_network() {
    FILE* pipe = popen("nmcli device status | grep wlan", "r");
    if (!pipe) {
        std::cerr << "Error: Failed to open pipe for network check." << "\n";
        return false;
    }

    char buffer[128];
    std::string result = "";

    // Read the output of the command
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    // Close the pipe
    int return_code = pclose(pipe);
    if (return_code != 0) {
        std::cerr << "Error: nmcli command failed with return code " << return_code << "\n";
        return false;
    }

    // Trim any trailing whitespace (e.g., newline)
    result.erase(result.find_last_not_of(" \n\r\t") + 1);

    return result.find("connected") != std::string::npos;
}

bool Networking::wifi_enabled() {
    FILE* pipe = popen("nmcli radio wifi", "r");
    if (!pipe) {
        std::cerr << "Error: Failed to open pipe for Wi-Fi status check." << "\n";
        return false;
    }

    char buffer[128];
    std::string result = "";

    // Read the output of the command
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    // Close the pipe and check the return code
    int return_code = pclose(pipe);
    if (return_code != 0) {
        std::cerr << "Error: nmcli command failed with return code " << return_code << "\n";
        return false;
    }

    // Trim any trailing whitespace (e.g., newline)
    result.erase(result.find_last_not_of(" \n\r\t") + 1);

    return (result == "enabled");
}

void Networking::set_wifi(bool enabled) {
    std::string cmd = "nmcli radio wifi ";
    cmd += enabled ? "on" : "off";
    FILE* pipe = popen(cmd.c_str(), "r");

    if (pipe) {
        pclose(pipe);
    } else{
        std::cerr << "Error: Failed to execute command." << "\n";
    }
}

void Networking::create_hotspot(std::string ifname, std::string ssid, std::string passwd) {
    std::string cmd = "nmcli device wifi hotspot ifname " + ifname + " ssid " + ssid + " password " + passwd;
    FILE* pipe = popen(cmd.c_str(), "r");

    if (pipe) {
        pclose(pipe);
    } else{
        std::cerr << "Error: Failed to execute command." << "\n";
    }
}

void Networking::connect_last_network() {
    std::string cmd = "nmcli device wifi connect \"$(nmcli -t -f NAME connection show --active | grep -v Hotspot | head -n1)\"";
    FILE* pipe = popen(cmd.c_str(), "r");

    if (pipe) {
        pclose(pipe);
    } else{
        std::cerr << "Error: Failed to execute command." << "\n";
    }
}

int Networking::wait_for_network() {
    int elapsed_seconds = 0;

    while (!Networking::check_network()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        elapsed_seconds++;
    }

    return elapsed_seconds;
}

void Networking::scan_wifi() {
    FILE* pipe = popen("nmcli device wifi rescan", "r");
    if (pipe) {
        pclose(pipe); // Close the pipe after executing the command
    } else {
        std::cerr << "Error: Failed to execute Wi-Fi scan command." << "\n";
    }
}

bool Networking::connect_wifi(std::string ssid, std::string passwd) {
    std::string cmd = "raspi-config nonint do_wifi_ssid_passphrase" + ssid + " " + passwd;
    FILE* pipe = popen(cmd.c_str(), "r");

    if (pipe) {
        pclose(pipe); // Close the pipe after executing the command
        return true;
    } else {
        std::cerr << "Error: Failed to connect to Wi-Fi network: " << ssid << "\n";
        return false;
    }
}

std::string Networking::list_networks() {
    FILE* pipe = popen("nmcli device wifi list", "r");
    char buffer[128];
    std::string result = "";

    if (pipe) {
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }

        int return_code = pclose(pipe);
    } else {
        std::cerr << "Error listing networks.\n";
    }

    return result;
}