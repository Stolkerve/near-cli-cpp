#ifndef COMMANDS_PARSER_H
#define COMMANDS_PARSER_H

#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>

#include <nlohmann/json.hpp>
#include <httplib.h>

constexpr auto SERVER_HOST = "localhost";
constexpr auto SERVER_PORT = 5000;

class CommandsParser {
public:
    CommandsParser(int argc, char *argv[]);
    void Start();
private:

    nlohmann::json CreateQueryMethodJson(const std::string &requestType, const nlohmann::json &params);

    void LoginCommand();
    void StatusCommand();
    void NetworkInfoCommand();
    void GenesisConfig();
    void ProtocolConfig();
    void GasPrice();

private:
    const int m_Argc;
    const std::vector<std::string_view> m_Argv;
    const std::unordered_map<std::string, std::function<void()>> m_Commands = {
        {
            "login",
            [this]() {LoginCommand();} // El lambda es para evitar usar std::bind que es super agresivo con el tamano del binario y velocidad
        },
        {
            "status",
            [this]() {StatusCommand();}
        },
        {
            "network-info",
            [this]() {NetworkInfoCommand();}
        },

        {
            "genesis-config",
            [this]() {GenesisConfig();}
        },        
        {
            "protocol-config",
            [this]() {ProtocolConfig();}
        },        
        {
            "gas-price",
            [this]() {GasPrice();}
        },        
        
    };
};

#endif