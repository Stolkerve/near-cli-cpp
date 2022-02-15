#ifndef COMMANDS_PARSER_H
#define COMMANDS_PARSER_H

#include <string_view>
#include <vector>

class CommandsParser {
public:
    CommandsParser(int argc, char *argv[]);
    void Start();
private:
    void LoginCommand();

private:
    int m_Argc;
    const std::vector<std::string_view> m_Argv;
};

#endif