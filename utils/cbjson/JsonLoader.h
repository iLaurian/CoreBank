#ifndef OOP_JSONLOADER_H
#define OOP_JSONLOADER_H

#include <string>

class Bank;

namespace JsonLoader {
    Bank& loadBankData(const std::string& bankPath,
                       const std::string& clientsPath,
                       const std::string& accountsPath,
                       const std::string& bondsPath,
                       const std::string& loansPath);
}

#endif
