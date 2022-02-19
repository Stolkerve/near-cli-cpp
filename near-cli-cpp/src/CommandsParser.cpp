#include "CommandsParser.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <array>
#include <utility>
#include <unordered_map>
#include <memory>
#ifdef __linux__
#include <experimental/filesystem>
#else
#include <filesystem>
#endif

#include "logging/SimpleLogger.h"
#include "KeyPair.h"

#include "utils.h"

namespace fs = std::experimental::filesystem;

// keysPath = GetHomeDirectory() + "/.near-cpp-keys/";
// std::fstream keysFile("", std::ios::in | std::ios::binary);

void CreateCredentialsFile(std::string accountId, KeyPair pair)
{
    auto keysDir = GetHomeDirectory() + "/.near-cpp-keys/";
    std::fstream credentialsFile((keysDir + accountId + ".json"), std::ios::out | std::ios::binary);

    if (credentialsFile.is_open())
    {
        nlohmann::json fileContent = {
            {"account_id", accountId},
            {"public_key", pair.GetPublicKey()},
            {"private_key", pair.GetPrivateKey()},
        };

        credentialsFile << fileContent.dump(4);
        credentialsFile.close();
    }
    else
    {
        Logger::Error("No se puede crear el fichero de las credenciales de ", accountId);
    }
}

CommandsParser::CommandsParser(int argc, char *argv[]) :
    m_Argc(argc),
    m_Argv(argv + 1, argv + argc)
{
}

void CommandsParser::Start()
{
    fs::create_directory(GetHomeDirectory() + "/.near-cpp-keys");

    if (m_Argc <= 1)
    {
        Logger::Error("Please, pass a command");
        return;
    }
    else if (m_Argc >= 10)
    {
        Logger::Error("Bro, stop");
        return;
    }

    for (const auto &[key, value] : m_Commands)
    {
        if (m_Argv.front() == key)
        {
            value();
            return;
        }
    }

    Logger::Error("No commands found");
}

void CommandsParser::LoginCommand()
{
    bool isServerClose = false;
    std::mutex isServerCloseMutex;

    KeyPair pair;
    pair.SignRandomEd25519();

    // Inicializar el servidor para saber la respuesta de la wallet
    auto serverThread = std::thread([&](std::function<void(std::string, KeyPair)> createCredentialsFileCallback) {
            httplib::Server s;

            s.Get("/success", [&](const httplib::Request& req, httplib::Response& res) {
                const std::lock_guard<std::mutex> lock(isServerCloseMutex);
                isServerClose = true;

                // Desde este punto el servidor ya no tiene utilidad,
                // Que el hilo principal se ocupe de crear el fichero con las credenciales
                createCredentialsFileCallback(std::move(req.get_param_value("account_id")), std::move(pair));

                res.set_content("Login successfull :)", "text/plain");
            });

            s.Get("/fail", [&](const httplib::Request&, httplib::Response& res) {
                const std::lock_guard<std::mutex> lock(isServerCloseMutex);
                isServerClose = true;
                Logger::Error("Login fails :c");
                res.set_content("Login fails : (", "text/plain");
            });
            s.listen(SERVER_HOST, SERVER_PORT); 
        },
        CreateCredentialsFile
    );

    std::string finalWalletUrl("https://wallet.testnet.near.org/login/?referrer=NEAR+CLI\\&public_key=ed25519%3A");
    finalWalletUrl.append(pair.GetPublicKey().substr(8));
    finalWalletUrl.append("\\&success_url=http%3A%2F%2Flocalhost%3A5000/success\\&failure_url=http%3A%2F%2Flocalhost%3A5000/fail");

    static auto waitAndShotdown = [&]()
    {
        // Esperar por la resputas del usuario
        while (!isServerClose)
        {
        }

        // Esperar a que se redireccione a las paginas
        sleep(2);

        // Finalizar el hilo del servidor
        serverThread.detach();
    };

    #ifdef _WIN32
        finalWalletUrl.insert(0, "start ");
        system(finalWalletUrl.c_str());

        waitAndShotdown();
    #endif
    #ifdef __linux__
        finalWalletUrl.insert(0, "xdg-open ");
        system(finalWalletUrl.c_str());

        waitAndShotdown();
    #endif
    #ifdef __APPLE__
        finalWalletUrl.insert(0, "open ");
        system(finalWalletUrl.c_str());

        waitAndShotdown();
    #endif
}

nlohmann::json CommandsParser::CreateQueryMethodJson(const std::string &requestType, const nlohmann::json &params)
{
    nlohmann::json _template = {
        {"jsonrpc", "2.0"},
        {"id", "dontcare"},
        {"method", "query"},
        {"params", {
            {"request_type", requestType},
            {"finality", "final"},
        }}};

    for (auto &[key, value] : params.items())
    {
        Logger::Info(key, " : ", value);
        _template["params"][key] = value;
    }

    return _template;
}

void CommandsParser::StatusCommand() {
    httplib::Client cli("http://rpc.testnet.near.org");
    const nlohmann::json bodyJson = {
        {"jsonrpc", "2.0"},
        {"id", "dontcare"},
        {"method", "status"},
        {"params", "[]"},
    };

    auto res = cli.Post("/", bodyJson.dump(), "application/json");
    
    Logger::Info(nlohmann::json::parse(res->body)["result"].dump(2));
}

void CommandsParser::NetworkInfoCommand()
{
    httplib::Client cli("http://rpc.testnet.near.org");
    const nlohmann::json bodyJson = {
        {"jsonrpc", "2.0"},
        {"id", "dontcare"},
        {"method", "network_info"},
        {"params", "[]"},
    };

    auto res = cli.Post("/", bodyJson.dump(), "application/json");
    
    Logger::Info(nlohmann::json::parse(res->body)["result"].dump(2));
}

void CommandsParser::GenesisConfig() {
    httplib::Client cli("http://rpc.testnet.near.org");
    const nlohmann::json bodyJson = {
        {"jsonrpc", "2.0"},
        {"id", "dontcare"},
        {"method", "EXPERIMENTAL_genesis_config"},
        {"params", "[]"},
    };

    auto res = cli.Post("/", bodyJson.dump(), "application/json");
    
    Logger::Info(nlohmann::json::parse(res->body)["result"].dump(2));
}

void CommandsParser::ProtocolConfig()
{
    httplib::Client cli("http://rpc.testnet.near.org");
    const nlohmann::json bodyJson = {
        {"jsonrpc", "2.0"},
        {"id", "dontcare"},
        {"method", "EXPERIMENTAL_protocol_config"},
        {"params", {
            {"finality", "final"}
        }},
    };

    auto res = cli.Post("/", bodyJson.dump(), "application/json");
    
    Logger::Info(nlohmann::json::parse(res->body)["result"].dump(2));
}

void CommandsParser::GasPrice() {
    httplib::Client cli("http://rpc.testnet.near.org");
    const nlohmann::json bodyJson = {
        {"jsonrpc", "2.0"},
        {"id", "dontcare"},
        {"method", "gas_price"},
        {"params", {nullptr}},
    };

    auto res = cli.Post("/", bodyJson.dump(), "application/json");
    
    Logger::Info(nlohmann::json::parse(res->body)["result"].dump(2));
}

void CommandsParser::ViewAccessKey()
{
    if (m_Argc < 3)
    {
        Logger::Error("Pass the account id. keys [account id]");
        return;
    }

    httplib::Client cli("http://rpc.testnet.near.org");
    const nlohmann::json bodyJson = {
        {"jsonrpc", "2.0"},
        {"id", "dontcare"},
        {"method", "query"},
        {"params", {
                {"request_type", "view_access_key_list"},
                {"finality", "final"},
                {"account_id", m_Argv[1]},
            }
        },
    };

    auto res = cli.Post("/", bodyJson.dump(), "application/json");
    
    Logger::Info(nlohmann::json::parse(res->body)["result"].dump(2));
}

void CommandsParser::ViewAccount()
{
    if (m_Argc < 3)
    {
        Logger::Error("Pass the account id. keys [account id]");
        return;
    }

    httplib::Client cli("http://rpc.testnet.near.org");
    const nlohmann::json bodyJson = {
        {"jsonrpc", "2.0"},
        {"id", "dontcare"},
        {"method", "query"},
        {"params", {
                {"request_type", "view_account"},
                {"finality", "final"},
                {"account_id", m_Argv[1]},
            }
        },
    };

    auto res = cli.Post("/", bodyJson.dump(), "application/json");
    
    Logger::Info(nlohmann::json::parse(res->body)["result"].dump(2));
}

void CommandsParser::ViewContractCode()
{
    if (m_Argc < 3)
    {
        Logger::Error("Pass the contract account id. keys [contract account id]");
        return;
    }

    httplib::Client cli("http://rpc.testnet.near.org");
    const nlohmann::json bodyJson = {
        {"jsonrpc", "2.0"},
        {"id", "dontcare"},
        {"method", "query"},
        {"params", {
                {"request_type", "view_code"},
                {"finality", "final"},
                {"account_id", m_Argv[1]},
            }
        },
    };

    auto res = cli.Post("/", bodyJson.dump(), "application/json");
    
    Logger::Info(nlohmann::json::parse(res->body)["result"].dump(2));
}

void CommandsParser::ViewFunction()
{
    // Logger::Info(m_Argv[1], ", ",  m_Argv[2], ", ", m_Argv[3]);
    std::string args_base64;
    if (m_Argc < 3)
    {
        Logger::Error("Pass the account id. keys [account id] [method name] \'{\"param\": 123}\'(optinal if not take any paramas)");
        return;
    }
    else if (m_Argc < 4)
    {
        Logger::Error("Pass the method name. keys [account id] [method name] \'{\"param\": 123}\'(optinal if not take any paramas)");
        return;
    }
    // Si ningun argumento del metodo es pasado, se entiendo que no tiene argumentos
    else if (m_Argc < 5)
    {
        args_base64 = "e30="; // e30= == {}
    }
    else
    {
        args_base64 = httplib::detail::base64_encode(m_Argv[3].data());
    }

    httplib::Client cli("http://rpc.testnet.near.org");
    const nlohmann::json bodyJson = {
        {"jsonrpc", "2.0"},
        {"id", "dontcare"},
        {"method", "query"},
        {"params", {
                {"request_type", "call_function"},
                {"finality", "final"},
                {"account_id",  m_Argv[1]},
                {"method_name", m_Argv[2]},
                {"args_base64", args_base64},
            }
        },
    };

    auto res = cli.Post("/", bodyJson.dump(), "application/json");
    auto resJson = nlohmann::json::parse(res->body);
    std::vector<std::uint8_t> returnValueBytes = resJson["result"]["result"].get<std::vector<std::uint8_t>>();
    std::string result( returnValueBytes.begin(), returnValueBytes.end() );
    Logger::Info(result);
}