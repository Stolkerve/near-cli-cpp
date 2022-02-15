#include "CommandsParser.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>

#include <functional>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "utils.h"
#include "logging/SimpleLogger.h"
#include "KeyPair.h"

CommandsParser::CommandsParser(int argc, char *argv[]) :
    m_Argc(argc), m_Argv(argv + 1, argv + argc)
{
}

void CommandsParser::Start() {
    if (m_Argc <= 1) {
        Logger::Error("Please, pass a command");
        std::terminate();
    }

    if (m_Argc == 2 && !strcmp(m_Argv[0].data(), "login")) {
        LoginCommand();
    } else {
        Logger::Error("Unknow command!");
        std::terminate();
    }

    if (m_Argc > 2) {
        Logger::Info("Others commands");
    }
}

void CreateCredentialsFile_Test(std::string accountId, KeyPair pair)
{
    std::fstream credentialsFile(( accountId + ".json"), std::ios_base::out);

    nlohmann::json fileContent = {
        {"account_id", accountId},
        {"public_key", pair.GetPublicKey()},
        {"private_key", pair.GetPrivateKey()},
    };

    credentialsFile << fileContent.dump(4);
    credentialsFile.close();
}


void CommandsParser::LoginCommand() {
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

            s.Get("/fail", [&](const httplib::Request& req, httplib::Response& res) {
                const std::lock_guard<std::mutex> lock(isServerCloseMutex);
                isServerClose = true;
                Logger::Error("Login fails :c");
                res.set_content("Login fails : (", "text/plain");
            });
            s.listen("localhost", 5000);
        },
        CreateCredentialsFile_Test
    );

    std::string finalWalletUrl("https://wallet.testnet.near.org/login/?referrer=NEAR+CLI\\&public_key=ed25519%3A");
    finalWalletUrl.append(pair.GetPublicKey().substr(8));
    finalWalletUrl.append("\\&success_url=http%3A%2F%2Flocalhost%3A5000/success\\&failure_url=http%3A%2F%2Flocalhost%3A5000/fail");

    static auto waitAndShotdown = [&]() {
        // Esperar por la resputas del usuario
        while (!isServerClose) {}
        

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


void View() {
    // httplib::Client cli("http://rpc.testnet.near.org");

    // nlohmann::json bodyJson = {
    //     {"jsonrpc", "2.0"},
    //     {"id", "dontcare"},
    //     {"method", "query"},
    //     {"params", {{"request_type", "view_access_key_list"}, {"finality", "final"}, {"account_id", "example.testnet"}}},
    // };

    // Logger::Info(bodyJson.dump(2));

    // auto res = cli.Post("/", bodyJson.dump(), "application/json");
    // Logger::Info("Status: ", res->status);
    // Logger::Info("Body: ", nlohmann::json::parse(res->body).dump(2));
}
