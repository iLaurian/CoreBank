#ifndef OOP_APITYPES_H
#define OOP_APITYPES_H

#include <string>

enum class AccountKind {
    Personal,
    Savings,
    Retirement,
    Investment
};

inline std::string accountKindToString(const AccountKind kind) {
    switch (kind) {
        case AccountKind::Personal: return "personal";
        case AccountKind::Savings: return "savings";
        case AccountKind::Retirement: return "retirement";
        case AccountKind::Investment: return "investment";
        default: return "unknown";
    }
}

#endif
