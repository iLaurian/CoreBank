#ifndef OOP_HTTPSERVER_H
#define OOP_HTTPSERVER_H

#include <exception>
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "BankService.h"

class HttpServer {
    BankService& service;
    int port;

    static void setJson(httplib::Response& res, const nlohmann::json& payload, int status = 200);
    static void setError(httplib::Response& res, int status, const std::string& code, const std::string& message);

    void registerRoutes(httplib::Server& server);

public:
    HttpServer(BankService& serviceRef, int listenPort);
    void start();
};

#endif
