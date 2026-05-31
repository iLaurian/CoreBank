#include "HttpServer.h"
#include <nlohmann/json.hpp>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <atomic>
#include "../utils/cbjson/JsonHelpers.h"
#include "../utils/cbexception/BankExceptions.h"
#include "../utils/cbdate/DateUtils.h"
#include "../utils/cblogger/Logger.h"
#include "../utils/cbcurrency/CurrencyConverter.h"

HttpServer::HttpServer(BankService& serviceRef, int listenPort)
    : service(serviceRef), port(listenPort) {
}

void HttpServer::setJson(httplib::Response& res, const nlohmann::json& payload, int status) {
    res.status = status;
    res.set_content(payload.dump(), "application/json");
}

void HttpServer::setError(httplib::Response& res, int status, const std::string& code, const std::string& message) {
    setJson(res, makeError(code, message), status);
}

static bool parseJsonBody(const httplib::Request& req, nlohmann::json& body, httplib::Response& res) {
    try {
        body = nlohmann::json::parse(req.body);
        return true;
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(makeError("VALIDATION_ERROR", std::string("Invalid JSON: ") + e.what()).dump(), "application/json");
        return false;
    }
}

static int nextRequestId() {
    static std::atomic<int> counter{1};
    return counter.fetch_add(1);
}

static std::string normalizeIban(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    for (unsigned char c : value) {
        if (std::isspace(c)) {
            continue;
        }
        result.push_back(static_cast<char>(std::toupper(c)));
    }
    return result;
}

void HttpServer::registerRoutes(httplib::Server& server) {
    server.Get("/health", [&](const httplib::Request&, httplib::Response& res) {
        nlohmann::json payload{
            {"status", "ok"},
            {"date", getCurrentDate()}
        };
        setJson(res, payload, 200);
    });

    server.Get("/bank", [&](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(service.getMutex());
        const Bank& bank = Bank::instance();
        nlohmann::json payload{
            {"name", bank.getName()},
            {"swiftCode", bank.getSwiftCode()},
            {"totalAssetsUSD", bank.calculateTotalBankAssets()}
        };
        setJson(res, payload, 200);
    });

    server.Get("/clients", [&](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(service.getMutex());
        nlohmann::json payload;
        payload["clients"] = nlohmann::json::array();
        for (const auto& client : service.getClients()) {
            payload["clients"].push_back({
                {"cnp", client->getCNP()},
                {"name", client->getName()},
                {"address", client->getAddress()},
                {"monthlyIncome", client->getMonthlyIncome()}
            });
        }
        setJson(res, payload, 200);
    });

    server.Get(R"(/clients/([0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string cnp = req.matches[1];
        std::lock_guard<std::mutex> lock(service.getMutex());
        try {
            const Client* client = service.getClient(cnp);
            setJson(res, clientToJson(*client), 200);
        } catch (const NotFoundError& e) {
            setError(res, 404, "NOT_FOUND", e.what());
        }
    });

    server.Post("/clients", [&](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json body;
        if (!parseJsonBody(req, body, res)) return;
        try {
            const std::string cnp = body.at("cnp").get<std::string>();
            const std::string name = body.at("name").get<std::string>();
            const std::string address = body.at("address").get<std::string>();
            const double income = body.at("monthlyIncome").get<double>();
            std::lock_guard<std::mutex> lock(service.getMutex());
            const Client* client = service.registerClient(cnp, name, address, income);
            service.persist();
            nlohmann::json payload{
                {"cnp", client->getCNP()},
                {"name", client->getName()},
                {"address", client->getAddress()},
                {"monthlyIncome", client->getMonthlyIncome()}
            };
            setJson(res, payload, 201);
        } catch (const ValidationError& e) {
            setError(res, 400, "VALIDATION_ERROR", e.what());
        } catch (const BankException& e) {
            setError(res, 409, "CONFLICT", e.what());
        }
    });

    server.Delete(R"(/clients/([0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string cnp = req.matches[1];
        try {
            std::lock_guard<std::mutex> lock(service.getMutex());
            service.removeClient(cnp);
            service.persist();
            res.status = 204;
        } catch (const NotFoundError& e) {
            setError(res, 404, "NOT_FOUND", e.what());
        }
    });

    server.Get(R"(/clients/([0-9]+)/credit-score)", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string cnp = req.matches[1];
        std::lock_guard<std::mutex> lock(service.getMutex());
        try {
            const Client* client = service.getClient(cnp);
            setJson(res, nlohmann::json{{"cnp", cnp}, {"creditScore", client->getCreditScore()}}, 200);
        } catch (const NotFoundError& e) {
            setError(res, 404, "NOT_FOUND", e.what());
        }
    });

    server.Get(R"(/clients/([0-9]+)/net-worth)", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string cnp = req.matches[1];
        std::lock_guard<std::mutex> lock(service.getMutex());
        try {
            const Client* client = service.getClient(cnp);
            setJson(res, nlohmann::json{{"cnp", cnp}, {"netWorthUSD", client->calculateTotalNetWorth()}}, 200);
        } catch (const NotFoundError& e) {
            setError(res, 404, "NOT_FOUND", e.what());
        }
    });

    server.Get(R"(/clients/([0-9]+)/accounts)", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string cnp = req.matches[1];
        std::lock_guard<std::mutex> lock(service.getMutex());
        try {
            const Client* client = service.getClient(cnp);
            nlohmann::json payload;
            payload["cnp"] = cnp;
            payload["accounts"] = nlohmann::json::array();
            for (const auto& account : client->getAccounts()) {
                payload["accounts"].push_back(accountToJson(*account));
            }
            setJson(res, payload, 200);
        } catch (const NotFoundError& e) {
            setError(res, 404, "NOT_FOUND", e.what());
        }
    });

    server.Get(R"(/clients/([0-9]+)/loans)", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string cnp = req.matches[1];
        std::lock_guard<std::mutex> lock(service.getMutex());
        try {
            Client* client = service.getClient(cnp);
            nlohmann::json payload;
            payload["cnp"] = cnp;
            payload["loans"] = nlohmann::json::array();
            for (const auto& loan : client->getLoans()) {
                payload["loans"].push_back(loanToJson(loan));
            }
            setJson(res, payload, 200);
        } catch (const NotFoundError& e) {
            setError(res, 404, "NOT_FOUND", e.what());
        }
    });

    auto createAccountHandler = [&](AccountKind kind) {
        return [&, kind](const httplib::Request& req, httplib::Response& res) {
            const std::string cnp = req.matches[1];
            nlohmann::json body;
            if (!parseJsonBody(req, body, res)) return;
            try {
                const double initialBalance = body.at("initialBalance").get<double>();
                const std::string currencyCode = body.at("currency").get<std::string>();
                const std::string inceptionDate = body.value("inceptionDate", "");
                const std::string maturityDate = body.value("maturityDate", "");
                const Currency currency = CurrencyConverter::parseCurrency(currencyCode);

                std::lock_guard<std::mutex> lock(service.getMutex());
                const BankAccount* account = service.createAccount(cnp, kind, initialBalance, currency, inceptionDate, maturityDate);
                service.persist();
                setJson(res, accountToJson(*account), 201);
            } catch (const ValidationError& e) {
                setError(res, 400, "VALIDATION_ERROR", e.what());
            } catch (const NotFoundError& e) {
                setError(res, 404, "NOT_FOUND", e.what());
            }
        };
    };

    server.Post(R"(/clients/([0-9]+)/accounts/personal)", createAccountHandler(AccountKind::Personal));
    server.Post(R"(/clients/([0-9]+)/accounts/savings)", createAccountHandler(AccountKind::Savings));
    server.Post(R"(/clients/([0-9]+)/accounts/retirement)", createAccountHandler(AccountKind::Retirement));
    server.Post(R"(/clients/([0-9]+)/accounts/investment)", createAccountHandler(AccountKind::Investment));

    auto accountGetHandler = [&](AccountKind kind) {
        return [&, kind](const httplib::Request& req, httplib::Response& res) {
            const int requestId = nextRequestId();
            Logger::info("[" + std::to_string(requestId) + "] GET account route=" + req.path);
            const std::string rawIban = req.matches[1];
            const std::string iban = normalizeIban(rawIban);
            std::lock_guard<std::mutex> lock(service.getMutex());
            const BankAccount* account = service.findAccount(iban, kind);
            if (!account) {
                Logger::warn("[" + std::to_string(requestId) + "] Account lookup returned null");
                setError(res, 404, "NOT_FOUND", "Account not found");
                return;
            }
            setJson(res, accountToJson(*account), 200);
        };
    };

    server.Get(R"(/accounts/personal/([A-Z0-9]+))", accountGetHandler(AccountKind::Personal));
    server.Get(R"(/accounts/savings/([A-Z0-9]+))", accountGetHandler(AccountKind::Savings));
    server.Get(R"(/accounts/retirement/([A-Z0-9]+))", accountGetHandler(AccountKind::Retirement));
    server.Get(R"(/accounts/investment/([A-Z0-9]+))", accountGetHandler(AccountKind::Investment));

    auto transactionsHandler = [&](AccountKind kind) {
        return [&, kind](const httplib::Request& req, httplib::Response& res) {
            const int requestId = nextRequestId();
            Logger::info("[" + std::to_string(requestId) + "] GET transactions route=" + req.path);
            const std::string rawIban = req.matches[1];
            const std::string iban = normalizeIban(rawIban);
            std::lock_guard<std::mutex> lock(service.getMutex());
            const BankAccount* account = service.findAccount(iban, kind);
            if (!account) {
                Logger::warn("[" + std::to_string(requestId) + "] Transactions lookup returned null");
                setError(res, 404, "NOT_FOUND", "Account not found");
                return;
            }
            nlohmann::json payload;
            payload["iban"] = iban;
            payload["transactions"] = nlohmann::json::array();
            for (int i = 0; i < account->getTransactionCount(); ++i) {
                payload["transactions"].push_back(transactionToJson(account->getTransaction(i)));
            }
            setJson(res, payload, 200);
        };
    };

    server.Get(R"(/accounts/personal/([A-Z0-9]+)/transactions)", transactionsHandler(AccountKind::Personal));
    server.Get(R"(/accounts/savings/([A-Z0-9]+)/transactions)", transactionsHandler(AccountKind::Savings));
    server.Get(R"(/accounts/retirement/([A-Z0-9]+)/transactions)", transactionsHandler(AccountKind::Retirement));
    server.Get(R"(/accounts/investment/([A-Z0-9]+)/transactions)", transactionsHandler(AccountKind::Investment));

    auto depositHandler = [&](AccountKind kind) {
        return [&, kind](const httplib::Request& req, httplib::Response& res) {
            const int requestId = nextRequestId();
            Logger::info("[" + std::to_string(requestId) + "] POST deposit route=" + req.path);
            nlohmann::json body;
            if (!parseJsonBody(req, body, res)) return;
            const std::string rawIban = req.matches[1];
            const std::string iban = normalizeIban(rawIban);
            const double amount = body.at("amount").get<double>();
            const std::string dateStr = body.at("date").get<std::string>();
            std::lock_guard<std::mutex> lock(service.getMutex());
            BankAccount* account = service.findAccount(iban, kind);
            if (!account) {
                Logger::warn("[" + std::to_string(requestId) + "] Deposit lookup returned null");
                setError(res, 404, "NOT_FOUND", "Account not found");
                return;
            }
            account->processDeposit(amount, dateStr);
            service.persist();
            setJson(res, nlohmann::json{{"iban", iban}, {"balance", account->getBalance()}}, 200);
        };
    };

    auto withdrawHandler = [&](AccountKind kind) {
        return [&, kind](const httplib::Request& req, httplib::Response& res) {
            const int requestId = nextRequestId();
            Logger::info("[" + std::to_string(requestId) + "] POST withdraw route=" + req.path);
            nlohmann::json body;
            if (!parseJsonBody(req, body, res)) return;
            const std::string rawIban = req.matches[1];
            const std::string iban = normalizeIban(rawIban);
            const double amount = body.at("amount").get<double>();
            const std::string dateStr = body.at("date").get<std::string>();
            std::lock_guard<std::mutex> lock(service.getMutex());
            BankAccount* account = service.findAccount(iban, kind);
            if (!account) {
                Logger::warn("[" + std::to_string(requestId) + "] Withdraw lookup returned null");
                setError(res, 404, "NOT_FOUND", "Account not found");
                return;
            }
            if (!account->processWithdrawal(amount, dateStr)) {
                setError(res, 409, "CONFLICT", "Withdrawal rejected");
                return;
            }
            service.persist();
            setJson(res, nlohmann::json{{"iban", iban}, {"balance", account->getBalance()}}, 200);
        };
    };

    auto transferHandler = [&](AccountKind kind) {
        return [&, kind](const httplib::Request& req, httplib::Response& res) {
            const int requestId = nextRequestId();
            Logger::info("[" + std::to_string(requestId) + "] POST transfer route=" + req.path);
            nlohmann::json body;
            if (!parseJsonBody(req, body, res)) return;
            const std::string rawFromIban = req.matches[1];
            const std::string rawToIban = body.at("toIban").get<std::string>();
            const std::string fromIban = normalizeIban(rawFromIban);
            const std::string toIban = normalizeIban(rawToIban);
            const double amount = body.at("amount").get<double>();
            const std::string dateStr = body.at("date").get<std::string>();
            std::lock_guard<std::mutex> lock(service.getMutex());
            const BankAccount* account = service.findAccount(fromIban, kind);
            if (!account) {
                Logger::warn("[" + std::to_string(requestId) + "] Transfer lookup returned null");
                setError(res, 404, "NOT_FOUND", "Account not found");
                return;
            }
            const bool ok = service.processTransfer(fromIban, toIban, amount, dateStr);
            if (!ok) {
                setError(res, 409, "CONFLICT", "Transfer rejected");
                return;
            }
            service.persist();
            setJson(res, nlohmann::json{{"fromIban", fromIban}, {"toIban", toIban}, {"status", "completed"}}, 200);
        };
    };

    server.Post(R"(/accounts/personal/([A-Z0-9]+)/deposit)", depositHandler(AccountKind::Personal));
    server.Post(R"(/accounts/savings/([A-Z0-9]+)/deposit)", depositHandler(AccountKind::Savings));
    server.Post(R"(/accounts/retirement/([A-Z0-9]+)/deposit)", depositHandler(AccountKind::Retirement));
    server.Post(R"(/accounts/investment/([A-Z0-9]+)/deposit)", depositHandler(AccountKind::Investment));

    server.Post(R"(/accounts/personal/([A-Z0-9]+)/withdraw)", withdrawHandler(AccountKind::Personal));
    server.Post(R"(/accounts/savings/([A-Z0-9]+)/withdraw)", withdrawHandler(AccountKind::Savings));
    server.Post(R"(/accounts/retirement/([A-Z0-9]+)/withdraw)", withdrawHandler(AccountKind::Retirement));
    server.Post(R"(/accounts/investment/([A-Z0-9]+)/withdraw)", withdrawHandler(AccountKind::Investment));

    server.Post(R"(/accounts/personal/([A-Z0-9]+)/transfer)", transferHandler(AccountKind::Personal));
    server.Post(R"(/accounts/savings/([A-Z0-9]+)/transfer)", transferHandler(AccountKind::Savings));
    server.Post(R"(/accounts/retirement/([A-Z0-9]+)/transfer)", transferHandler(AccountKind::Retirement));
    server.Post(R"(/accounts/investment/([A-Z0-9]+)/transfer)", transferHandler(AccountKind::Investment));

    server.Post(R"(/accounts/savings/([A-Z0-9]+)/close)", [&](const httplib::Request& req, httplib::Response& res) {
        const int requestId = nextRequestId();
        Logger::info("[" + std::to_string(requestId) + "] POST savings close route=" + req.path);
        nlohmann::json body;
        if (!parseJsonBody(req, body, res)) return;
        const std::string rawSavingsIban = req.matches[1];
        const std::string rawTargetIban = body.at("targetIban").get<std::string>();
        const std::string savingsIban = normalizeIban(rawSavingsIban);
        const std::string targetIban = normalizeIban(rawTargetIban);
        const std::string dateStr = body.at("date").get<std::string>();
        std::lock_guard<std::mutex> lock(service.getMutex());
        Client* client = service.findClientByAccountIBAN(savingsIban);
        if (!client) {
            Logger::warn("[" + std::to_string(requestId) + "] Savings close client lookup returned null");
            setError(res, 404, "NOT_FOUND", "Client not found for savings account");
            return;
        }
        if (!client->closeSavingsAccount(savingsIban, targetIban, dateStr)) {
            setError(res, 409, "CONFLICT", "Savings close failed");
            return;
        }
        service.persist();
        setJson(res, nlohmann::json{{"closedIban", savingsIban}, {"targetIban", targetIban}, {"status", "closed"}}, 200);
    });

    server.Delete(R"(/accounts/([A-Z0-9]+))", [&](const httplib::Request& req, httplib::Response& res) {
        const int requestId = nextRequestId();
        Logger::info("[" + std::to_string(requestId) + "] DELETE account route=" + req.path);
        const std::string rawIban = req.matches[1];
        const std::string iban = normalizeIban(rawIban);
        std::lock_guard<std::mutex> lock(service.getMutex());
        if (!service.removeAccount(iban)) {
            Logger::warn("[" + std::to_string(requestId) + "] Account delete returned null");
            setError(res, 404, "NOT_FOUND", "Account not found");
            return;
        }
        service.persist();
        res.status = 204;
    });

    server.Post(R"(/accounts/investment/([A-Z0-9]+)/bonds)", [&](const httplib::Request& req, httplib::Response& res) {
        const int requestId = nextRequestId();
        Logger::info("[" + std::to_string(requestId) + "] POST investment bond route=" + req.path);
        nlohmann::json body;
        if (!parseJsonBody(req, body, res)) return;
        const std::string rawIban = req.matches[1];
        const std::string iban = normalizeIban(rawIban);
        std::lock_guard<std::mutex> lock(service.getMutex());
        BankAccount* base = service.findAccount(iban, AccountKind::Investment);
        if (!base) {
            Logger::warn("[" + std::to_string(requestId) + "] Investment bond lookup returned null");
            setError(res, 404, "NOT_FOUND", "Investment account not found");
            return;
        }
        auto* investment = dynamic_cast<InvestmentAccount*>(base);
        Bond bond;
        bond.id = body.at("id").get<std::string>();
        bond.faceValue = body.at("faceValue").get<double>();
        bond.annualCouponRate = body.at("annualCouponRate").get<double>();
        bond.issueDate = body.at("issueDate").get<std::string>();
        bond.maturityDate = body.at("maturityDate").get<std::string>();
        const std::string currencyCode = body.at("currency").get<std::string>();
        try {
            bond.currency = CurrencyConverter::parseCurrency(currencyCode);
        } catch (const ValidationError& e) {
            setError(res, 400, "VALIDATION_ERROR", e.what());
            return;
        }
        bond.lastCouponYear = body.value("lastCouponYear", 0);

        if (!investment->addBond(bond, getCurrentDate())) {
            setError(res, 409, "CONFLICT", "Bond purchase failed");
            return;
        }
        service.persist();
        setJson(res, nlohmann::json{{"status", "added"}, {"bondId", bond.id}}, 201);
    });

    server.Get(R"(/accounts/investment/([A-Z0-9]+)/bonds)", [&](const httplib::Request& req, httplib::Response& res) {
        const int requestId = nextRequestId();
        Logger::info("[" + std::to_string(requestId) + "] GET investment bonds route=" + req.path);
        const std::string rawIban = req.matches[1];
        const std::string iban = normalizeIban(rawIban);
        std::lock_guard<std::mutex> lock(service.getMutex());
        BankAccount* base = service.findAccount(iban, AccountKind::Investment);
        if (!base) {
            Logger::warn("[" + std::to_string(requestId) + "] Investment bonds lookup returned null");
            setError(res, 404, "NOT_FOUND", "Investment account not found");
            return;
        }
        auto* investment = dynamic_cast<InvestmentAccount*>(base);
        nlohmann::json payload;
        payload["iban"] = iban;
        payload["bonds"] = nlohmann::json::array();
        for (const auto& bond : investment->getBonds()) {
            payload["bonds"].push_back(bondToJson(bond));
        }
        setJson(res, payload, 200);
    });

    server.Post("/transfers", [&](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json body;
        if (!parseJsonBody(req, body, res)) return;
        const std::string rawFromIban = body.at("fromIban").get<std::string>();
        const std::string rawToIban = body.at("toIban").get<std::string>();
        const std::string fromIban = normalizeIban(rawFromIban);
        const std::string toIban = normalizeIban(rawToIban);
        const double amount = body.at("amount").get<double>();
        const std::string dateStr = body.at("date").get<std::string>();
        std::lock_guard<std::mutex> lock(service.getMutex());
        const bool ok = service.processTransfer(fromIban, toIban, amount, dateStr);
        if (!ok) {
            setError(res, 409, "CONFLICT", "Transfer rejected");
            return;
        }
        service.persist();
        setJson(res, nlohmann::json{{"status", "completed"}}, 200);
    });

    server.Post(R"(/clients/([0-9]+)/transfers/own)", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string cnp = req.matches[1];
        nlohmann::json body;
        if (!parseJsonBody(req, body, res)) return;
        const std::string rawFromIban = body.at("fromIban").get<std::string>();
        const std::string rawToIban = body.at("toIban").get<std::string>();
        const std::string fromIban = normalizeIban(rawFromIban);
        const std::string toIban = normalizeIban(rawToIban);
        const double amount = body.at("amount").get<double>();
        const std::string dateStr = body.at("date").get<std::string>();
        std::lock_guard<std::mutex> lock(service.getMutex());
        try {
            Client* client = service.getClient(cnp);
            client->transferBetweenOwnAccounts(fromIban, toIban, amount, dateStr);
            service.persist();
            setJson(res, nlohmann::json{{"status", "completed"}}, 200);
        } catch (const NotFoundError& e) {
            setError(res, 404, "NOT_FOUND", e.what());
        }
    });

    server.Post(R"(/clients/([0-9]+)/loans)", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string cnp = req.matches[1];
        nlohmann::json body;
        if (!parseJsonBody(req, body, res)) return;
        const double amount = body.at("amount").get<double>();
        const int months = body.at("months").get<int>();
        const std::string dateStr = body.at("date").get<std::string>();
        const std::string rawTargetIban = body.at("targetIban").get<std::string>();
        const std::string targetIban = normalizeIban(rawTargetIban);
        std::lock_guard<std::mutex> lock(service.getMutex());
        try {
            Client* client = service.getClient(cnp);
            const LoanRequestResult result = client->requestLoan(amount, months, dateStr, targetIban);
            service.persist();
            nlohmann::json payload{{"approved", result.approved}, {"reason", result.reason}};
            if (result.loan.has_value()) {
                payload["loan"] = loanToJson(*result.loan);
            }
            setJson(res, payload, 200);
        } catch (const BankException& e) {
            setError(res, 400, "VALIDATION_ERROR", e.what());
        }
    });
}

void HttpServer::start() {
    httplib::Server server;
    registerRoutes(server);
    server.listen("0.0.0.0", port);
}
