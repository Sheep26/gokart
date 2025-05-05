#include "./networking.h"

using namespace std;
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

    // Trim any trailing whitespace (e.g., newline)
    result.erase(result.find_last_not_of(" \n\r\t") + 1);

    return result.find("connected") != string::npos;
}

bool Networking::wifi_enabled() {
    FILE* pipe = popen("nmcli radio wifi", "r");
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

    // Trim any trailing whitespace (e.g., newline)
    result.erase(result.find_last_not_of(" \n\r\t") + 1);

    return (result == "enabled");
}

bool Networking::set_wifi(bool enabled) {
    system("nmcli radio wifi " + (enabled ? "on" : "off"));
}

void Networking::wait_for_network() {
    while (!Networking::check_network()) {
        sleep_for(milliseconds(1000));
    }
}