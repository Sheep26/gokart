#include <string>
#include <vector>
#include <functional>

struct Command {
    std::string name;
    std::function<std::string()> func;
};

class CommandListener {
public:
    static void init_commands();
    static std::string handle_command(std::string command);
private:
    static std::vector<Command> commands;
};