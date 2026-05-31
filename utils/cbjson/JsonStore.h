#ifndef OOP_JSONSTORE_H
#define OOP_JSONSTORE_H

#include <string>
#include "../../models/cbbank/Bank.h"

class JsonStore {
    std::string bankPath;
    std::string clientsPath;
    std::string accountsPath;
    std::string bondsPath;
    std::string loansPath;

public:
    JsonStore(std::string bankFile,
              std::string clientsFile,
              std::string accountsFile,
              std::string bondsFile,
              std::string loansFile);

    void saveAll(const Bank& bank) const;
};

#endif
