#include <iostream>
#include <exception>
#include "utils/cblogger/Logger.h"
#include "utils/cbjson/JsonLoader.h"
#include "server/BankService.h"
#include "server/HttpServer.h"
#include "server/Scheduler.h"

int main() {
    try {
        Logger::init();
        Logger::info("Server starting");

        Bank& bank = JsonLoader::loadBankData(
            "data/bank.json",
            "data/clients.json",
            "data/accounts.json",
            "data/bonds.json",
            "data/loans.json");

        BankService service(
            bank,
            "data/bank.json",
            "data/clients.json",
            "data/accounts.json",
            "data/bonds.json",
            "data/loans.json");
        Scheduler scheduler(bank, service);
        scheduler.start();

        const int port = 8080;
        HttpServer server(service, port);
        server.start();

        scheduler.stop();
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
        Logger::error(std::string("Unhandled exception: ") + e.what());
    }

    return 0;
}
