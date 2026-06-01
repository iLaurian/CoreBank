#include <iostream>
#include <exception>
#include <csignal>
#include <thread>
#include <chrono>
#include "utils/cblogger/Logger.h"
#include "utils/cbjson/JsonLoader.h"
#include "server/BankService.h"
#include "server/HttpServer.h"
#include "server/Scheduler.h"

namespace {
    volatile std::sig_atomic_t shutdownRequested = 0;

    void handleShutdownSignal(int) {
        shutdownRequested = 1;
    }
}

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
        std::signal(SIGINT, handleShutdownSignal);
        std::signal(SIGTERM, handleShutdownSignal);

        std::thread serverThread([&server]() {
            server.start();
        });

        while (!shutdownRequested) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        try {
            service.persist();
        } catch (const std::exception& e) {
            Logger::error(std::string("Persist failed during shutdown: ") + e.what());
        }
        server.stop();
        serverThread.join();
        scheduler.stop();
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
        Logger::error(std::string("Unhandled exception: ") + e.what());
    }

    return 0;
}
