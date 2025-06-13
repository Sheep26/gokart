#include "commandListener.h"

std::string help(std::vector<std::string> args) {
    std::string output = "Help\n";

    for (const auto& cmd : commands) {
        output += cmd.name + "\n";
    }

    return output;
}

std::string connect_wifi(std::vector<std::string> args) {
    if (Networking::connnect_wifi(args[0], args[1])) return "Connection successful.";

    return "Failed to connect";
}

void CommandListener::init_commands() {
    commands.clear();
    commands.push_back({"help", help});
    commands.push_back({"connect_wifi", connect_wifi});
}

std::string CommandListener::handle_command(std::string command, std::vector<std::string> args) {
    for (const auto& cmd : commands) {
        if (cmd.name == command) {
            return cmd.func(args);
        }
    }

    return "";
}