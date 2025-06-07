#include "commandListener.h"

std::string help() {
    std::string output = "Help\n";
    output += "connect_wifi <ssid> <passwd>"

    return output;
}

void CommandListener::init_commands() {
    commands.clear();
    commands.push_back({"help", help});
}

std::string CommandListener::handle_command(std::string command) {
    for (const auto& cmd : commands) {
        if (cmd.name == command) {
            return cmd.func();
        }
    }

    return "";
}