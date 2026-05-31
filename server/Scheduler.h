#ifndef OOP_SCHEDULER_H
#define OOP_SCHEDULER_H

#include <atomic>
#include <thread>
#include "../models/cbbank/Bank.h"
#include "../server/BankService.h"

class Scheduler {
    Bank& bank;
    BankService& service;
    std::atomic<bool> running;
    std::thread worker;

    void runLoop();

public:
    Scheduler(Bank& bankRef, BankService& serviceRef);
    ~Scheduler();

    void start();
    void stop();
};

#endif
