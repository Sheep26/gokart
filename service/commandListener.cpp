#include "commandListener.h"

std::vector<Command> commands = {};

std::string help(std::vector<std::string> args) {
    std::string output = "Help\n";

    for (const auto& cmd : commands) {
        output += cmd.name + "\n";
    }

    return output;
}

std::string connect_wifi(std::vector<std::string> args) {
    if (Networking::connect_wifi(args[1], args[2])) return "Connection successful.";

    return "Failed to connect";
}

std::string scan_wifi(std::vector<std::string> args) {
    Networking::scan_wifi();

    return "Scan complete.";
}

void CommandListener::init_commands() {
    commands.clear();
    commands.push_back({"help", help});
    commands.push_back({"connect_wifi", connect_wifi});
    commands.push_back({"scan_wifi", scan_wifi});
}

std::string CommandListener::handle_command(std::vector<std::string> args) {
    for (const auto& cmd : commands) {
        if (cmd.name == args[0]) {
            return cmd.func(args);
        }
    }

    return "";
}