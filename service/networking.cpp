#include "./networking.h"

void Networking::wait_for_network() {

}

bool Networking::check_network() {
    FILE* pipe = popen("nmcli device status | grep wlan0", "r");
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