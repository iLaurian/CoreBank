#include "Scheduler.h"
#include <chrono>
#include "../utils/cbdate/DateUtils.h"

Scheduler::Scheduler(Bank& bankRef, BankService& serviceRef)
    : bank(bankRef), service(serviceRef), running(false) {
}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    if (running.load()) return;
    running.store(true);
    worker = std::thread([this]() { runLoop(); });
}

void Scheduler::stop() {
    if (!running.load()) return;
    running.store(false);
    if (worker.joinable()) {
        worker.join();
    }
}

void Scheduler::runLoop() {
    using namespace std::chrono;
    while (running.load()) {
        const std::string today = getCurrentDate();
        {
            std::lock_guard<std::mutex> lock(service.getMutex());
            bank.applyMonthlyAccountFees(today);
            bank.applyInterestForAll(today);
            bank.applyAnnualBondCoupons(today);
            bank.applyMonthlyLoanPayments(today);
            service.persist();
        }

        for (int i = 0; i < 86400 && running.load(); ++i) {
            std::this_thread::sleep_for(seconds(1));
        }
    }
}
