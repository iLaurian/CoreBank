#include <iostream>
#include <exception>
#include "models/cbbank/Bank.h"
#include "models/cbclient/Client.h"
#include "models/cbaccount/BankAccount.h"
#include "models/cbaccount/InvestmentAccount.h"
#include "utils/cblogger/Logger.h"
#include "utils/cbjson/JsonLoader.h"
#include "utils/cbbanking/ReportGenerator.h"

int main() {
    try {
        Logger::init();
        Logger::info("Application started");

        Bank& myBank = JsonLoader::loadBankData(
            "data/bank.json",
            "data/clients.json",
            "data/accounts.json",
            "data/bonds.json",
            "data/loans.json");

        std::cout << myBank.getSwiftCode() << "\n";

        Client* retrievedClient = myBank.getClient("1900101123456");
        std::cout << retrievedClient->getName() << "\n";
        std::cout << retrievedClient->getAddress() << "\n";
        std::cout << retrievedClient->getMonthlyIncome() << "\n";
        std::cout << retrievedClient->getCreditScore() << "\n";

        BankAccount* acc1_usd_ptr = retrievedClient->getBankAccount("RO12CBIN000000000001");
        if (acc1_usd_ptr) {
            acc1_usd_ptr->processDeposit(300.00, "2026-03-22");
            acc1_usd_ptr->processWithdrawal(50.00, "2026-03-22");

            auto clonedAccount = acc1_usd_ptr->clone();
            std::cout << *clonedAccount;
        }

        retrievedClient->transferBetweenOwnAccounts(
            "RO12CBIN000000000001",
            "RO12CBIN000000000002",
            50.00,
            "2026-03-22"
        );

        myBank.processTransfer(
            "RO12CBIN000000000001",
            "RO12CBIN000000000002",
            200.00,
            "2026-03-22"
        );

        myBank.processTransfer(
            "RO12CBIN000000000002",
            "RO12CBIN000000000003",
            150.00,
            "2026-03-22"
        );

        myBank.processTransfer(
            "RO12CBIN000000000003",
            "DE56SPRK999999999999",
            100.00,
            "2026-03-22"
        );

        if (auto* investment = dynamic_cast<InvestmentAccount*>(myBank.getClient("2900202234567")
                ->getBankAccount("RO12CBIN000000000004"))) {
            investment->creditInterest(10.0, "2026-03-22");
        }

        myBank.applyMonthlyAccountFees("2026-04-01");

        myBank.applyAnnualBondCoupons("2027-03-22");

        retrievedClient->applyInterestIfDue("2026-04-21");

        retrievedClient->closeSavingsAccount("RO12CBIN999999999999", "RO12CBIN000000000001", "2026-04-01");

        retrievedClient->removeBankAccount("RO12CBIN000000000002");

        const auto loanResult = retrievedClient->requestLoan(5000.00, 24, "2026-04-21", "RO12CBIN000000000001");
        if (loanResult.approved) {
            myBank.applyMonthlyLoanPayments("2026-05-21");
        } else {
            std::cout << "Loan request denied: " << loanResult.reason << "\n";
        }

        myBank.removeClient("0000000000000");

        std::cout << myBank;

        std::cout << "\n========== REPORT GENERATOR DEMO ==========\n\n";

        const BankAccount* demoAcc = retrievedClient->getBankAccount("RO12CBIN000000000001");
        if (demoAcc) {
            std::vector<const Transaction*> txPtrs;
            for (int i = 0; i < demoAcc->getTransactionCount(); ++i) {
                txPtrs.push_back(&demoAcc->getTransaction(i));
            }

            ReportGenerator txRpt("ACCOUNT STATEMENT - RO12CBIN000000000001");
            std::cout << txRpt.generateTable(txPtrs,
                {"Date", "Type", "Amount", "From", "To", "ID"},
                [](const Transaction* const& t, size_t col) -> std::string {
                    switch (col) {
                        case 0: return t->getDate();
                        case 1: return t->getType();
                        case 2: return std::to_string(t->getAmount());
                        case 3: return t->getSourceIBAN();
                        case 4: return t->getTargetIBAN();
                        case 5: return t->getId();
                        default: return "";
                    }
                });
        }

        {
            std::vector<Client*> clientPtrs;
            Client* c1 = myBank.getClient("1900101123456");
            Client* c2 = myBank.getClient("2900202234567");
            if (c1) clientPtrs.push_back(c1);
            if (c2) clientPtrs.push_back(c2);

            ReportGenerator clRpt("CLIENT PORTFOLIO REPORT");
            std::cout << clRpt.generateTable(clientPtrs,
                {"Name", "CNP", "Income", "Net Worth", "Score"},
                [](const Client* const& c, size_t col) -> std::string {
                    switch (col) {
                        case 0: return c->getName();
                        case 1: return c->getCNP();
                        case 2: return formatCurrency(static_cast<int>(c->getMonthlyIncome()), USD);
                        case 3: return formatCurrency(c->calculateTotalNetWorth(), USD);
                        case 4: return std::to_string(c->getCreditScore());
                        default: return "";
                    }
                });
        }

        std::cout << "\n--- formatCurrency standalone demo ---\n";
        std::cout << "Balance in USD: " << formatCurrency(1234.56, USD) << "\n";
        std::cout << "Balance in EUR: " << formatCurrency(987.65, EUR) << "\n";
        std::cout << "Balance in GBP: " << formatCurrency(500.00, GBP) << "\n";
        std::cout << "Balance in JPY: " << formatCurrency(100000, JPY) << "\n";
        std::cout << "Balance in CHF: " << formatCurrency(1500.00, CHF) << "\n";

    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
        Logger::error(std::string("Unhandled exception: ") + e.what());
    }

    return 0;
}
