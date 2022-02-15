#ifdef _WIN32
#include <windows.h>
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif // _WIN_32

#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <string_view>
#include <vector>
#include <functional>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "logging/SimpleLogger.h"
#include "utils.h"
#include "KeyPair.h"

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

void Login() {
    // Inicializar el servidor para saber la respuesta de la wallet
    bool isServerClose = false;
    std::mutex isServerCloseMutex;

    KeyPair pair;
    pair.SignRandomEd25519();

    auto serverThread = std::thread([&](std::function<void(std::string, KeyPair)> createCredentialsCallback, KeyPair keyPair) {
            httplib::Server s;

            s.Get("/success", [&](const httplib::Request& req, httplib::Response& res) {
                const std::lock_guard<std::mutex> lock(isServerCloseMutex);
                isServerClose = true;

                createCredentialsCallback(std::move(req.get_param_value("account_id")), std::move(pair));
                // Desde este punto el servidor ya no tiene utilidad,
                // Que el hilo principal se ocupe de crear el fichero con las credenciales

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
        CreateCredentialsFile_Test,
        pair
    );

    std::string finalWalletUrl("https://wallet.testnet.near.org/login/?referrer=NEAR+CLI\\&public_key=ed25519%3A");
    finalWalletUrl.append(pair.GetPublicKey().substr(8));
    finalWalletUrl.append("\\&success_url=http%3A%2F%2Flocalhost%3A5000/success\\&failure_url=http%3A%2F%2Flocalhost%3A5000/fail");

    #ifdef _WIN32
        finalWalletUrl.insert(0, "start ");
        system(finalWalletUrl.c_str());
    #endif

    #ifdef __linux__
        finalWalletUrl.insert(0, "xdg-open ");
        system(finalWalletUrl.c_str());

        // Esperar por la resputas del usuario
        
        std::cout << "Waiting wallet response ";
        while (!isServerClose)
        {
            // Es la peor solucion que para un loading
            sleep(1);
            std::cout << "." << std::flush;
            sleep(1);
            std::cout << "." << std::flush;
            sleep(1);
            std::cout << "." << std::flush;
            sleep(1);
            std::cout << "\b\b\b   \b\b\b" << std::flush;
        }

        // Esperar a que se redireccione a las paginas
        sleep(2);
        serverThread.detach();

    #endif

    #ifdef __APPLE__
        finalWalletUrl.insert(0, "open ");
        system(finalWalletUrl.c_str());
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

int ParseCommands(int argc, char *argv[]) {
    // const std::vector<std::string_view> args(argv + 1, argv + argc);
    
    if (argc <= 1) {
        Logger::Error("Please, pass a command");
        return EXIT_FAILURE;
    }

    if (argc == 2 && !strcmp(argv[1], "login")) {
        Login();
    } else {
        Logger::Error("Unknow command!");
        return EXIT_FAILURE;
    }

    if (argc > 2) {
        Logger::Info("Others commands");
    }

}

int main(int argc, char *argv[])
{
    #ifdef _WIN32
        // enable ANSI sequences for windows 10:
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD consoleMode;
        GetConsoleMode(console, &consoleMode);
        consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(console, consoleMode);
    #endif

    return ParseCommands(argc, argv);
}