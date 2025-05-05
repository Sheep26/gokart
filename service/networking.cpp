#include "./networking.h"

using namespace std::this_thread;
using namespace std::chrono;

bool Networking::check_network() {
    FILE* pipe = popen("nmcli device status | grep wlan", "r");
    if (!pipe) {
        cerr << "Error: Failed to open pipe for network check." << std::endl;
        return false;
    }

    char buffer[128];
    string result = "";

    // Read the output of the command
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    // Close the pipe
    int return_code = pclose(pipe);
    if (return_code != 0) {
        cerr << "Error: nmcli command failed with return code " << return_code << endl;
        return false;
    }

    // Trim any trailing whitespace (e.g., newline)
    result.erase(result.find_last_not_of(" \n\r\t") + 1);

    return result.find("connected") != string::npos;
}

bool Networking::wifi_enabled() {
    FILE* pipe = popen("nmcli radio wifi", "r");
    if (!pipe) {
        cerr << "Error: Failed to open pipe for Wi-Fi status check." << endl;
        return false;
    }

    char buffer[128];
    string result = "";

    // Read the output of the command
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    // Close the pipe and check the return code
    int return_code = pclose(pipe);
    if (return_code != 0) {
        cerr << "Error: nmcli command failed with return code " << return_code << endl;
        return false;
    }

    // Trim any trailing whitespace (e.g., newline)
    result.erase(result.find_last_not_of(" \n\r\t") + 1);

    return (result == "enabled");
}

void Networking::set_wifi(bool enabled) {
    string cmd = "nmcli radio wifi ";
    cmd += enabled ? "on" : "off";
    FILE* pipe = popen(cmd.c_str(), "r");

    if (pipe) {
        pclose(pipe);
    } else{
        cerr << "Error: Failed to execute command.";
    }
}

int Networking::wait_for_network() {
    int elapsed_seconds = 0;

    while (!Networking::check_network()) {
        this_thread::sleep_for(chrono::seconds(1));
        elapsed_seconds++;
    }

    return elapsed_seconds;
}

void Networking::scan_wifi() {
    FILE* pipe = popen("nmcli device wifi rescan", "r");
    if (pipe) {
        pclose(pipe); // Close the pipe after executing the command
    } else {
        cerr << "Error: Failed to execute Wi-Fi scan command." << endl;
    }
}

void Networking::connect_wifi(string ssid, string passwd) {
    string cmd = "nmcli device wifi connect \"" + ssid + "\" password \"" + passwd + "\"";
    FILE* pipe = popen(cmd.c_str(), "r");

    if (pipe) {
        pclose(pipe); // Close the pipe after executing the command
    } else {
        cerr << "Error: Failed to connect to Wi-Fi network: " << ssid << endl;
    }
}