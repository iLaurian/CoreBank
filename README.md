# CoreBank

A C++23 banking backend simulation with an HTTP REST API. Manages the hierarchical relationship between a Bank, its Clients, their Bank Accounts, Transactions, Loans, and Bonds.

## Features

- **4 account types:** Personal, Savings, Retirement, Investment
- **Currencies:** USD, EUR, GBP, JPY, CHF with automatic conversion
- **Monthly fees** per account type, **interest accrual** on savings/retirement
- **Investment bonds** with annual coupons and maturity payouts
- **Loan system** with credit-score-based approval and monthly payments
- **JSON persistence** — data loaded from and saved to JSON files
- **Scheduler** for automated monthly fee/interest/bond/loan processing
- **REST API** via cpp-httplib

## Project Structure

```
CoreBank/
├── main.cpp                  # Entry point
├── CMakeLists.txt            # Build configuration
├── data/                     # JSON seed data
│   ├── bank.json
│   ├── clients.json
│   ├── accounts.json
│   ├── bonds.json
│   └── loans.json
├── models/                   # Domain model
│   ├── cbaccount/            # BankAccount & derivatives
│   ├── cbbank/               # Bank singleton
│   ├── cbclient/             # Client
│   ├── cbinstrument/         # Bond
│   ├── cbloan/               # Loan
│   └── cbtransaction/        # Transaction
├── server/                   # HTTP server & service layer
│   ├── HttpServer.h/cpp      # Route handlers
│   ├── BankService.h/cpp     # Business logic facade
│   ├── Scheduler.h/cpp       # Periodic jobs
│   └── ApiTypes.h            # AccountKind enum
├── utils/                    # Utilities
│   ├── cbbanking/            # InterestAccrual, ReportGenerator
│   ├── cbcurrency/           # CurrencyConverter
│   ├── cbdate/               # Date parsing & arithmetic
│   ├── cbexception/          # Custom exceptions
│   ├── cbjson/               # JsonLoader, JsonStore, JsonHelpers
│   ├── cblogger/             # Logger
│   └── cbvalidation/         # IBAN, date, amount validation
├── ext/                      # External headers (nlohmann/json, httplib)
└── postman/                  # Postman collection
```

## Build & Run

### Prerequisites
- CMake >= 3.26
- C++23 compiler (GCC, Clang, MSVC)

### Build
```bash
cmake -B build
cmake --build build
```

### Run
The executable expects the `data/` directory as a relative path. Run from the **project root**:

```bash
cd CoreBank/
./build/oop                    # Linux/macOS
.\build\Release\oop.exe        # Windows (MSVC)
.\cmake-build-debug\oop.exe    # Windows (CLion / MinGW)
```

> **Important:** The server reads/writes JSON files using relative paths like `data/accounts.json`. The working directory must be the project root for this to work. If you run from a different directory (e.g. `cmake-build-debug/`), the files will be read/written there instead.

### Defaults
- Port: **8080**
- Log file: `./logs/log-<timestamp>.txt`

## Seed Data

The server loads initial data from `data/*.json` at startup. Modify these files to change starting state:

| File               | Contains                    |
|--------------------|-----------------------------|
| `bank.json`        | Bank name & SWIFT code      |
| `clients.json`     | Client list (CNP, name, address, income) |
| `accounts.json`    | Accounts with type, IBAN, balance, currency, owner |
| `bonds.json`       | Investment bonds per account |
| `loans.json`       | Client loans (not yet loaded) |

## REST API

Import the Postman collection at `postman/CoreBank.postman_collection.json` for a complete list of all endpoints with example bodies.

All requests and responses use `Content-Type: application/json`.