cmake_minimum_required(VERSION 3.26)

# NOTE: update executable name in .github/workflows/cmake.yml:25 when changing executable name in this file
# for now, the project name is used as the executable name
set(MAIN_PROJECT_NAME "oop")
set(MAIN_EXECUTABLE_NAME "${MAIN_PROJECT_NAME}")


project(${MAIN_PROJECT_NAME})
# set(CMAKE_PROJECT_VERSION_MAJOR 0)
# set(CMAKE_PROJECT_VERSION_MINOR 0)
# set(CMAKE_PROJECT_VERSION_PATCH 1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/Options.cmake)
include(cmake/CompilerFlags.cmake)
include(cmake/CopyHelper.cmake)

###############################################################################

# external dependencies with FetchContent
# include(FetchContent)
#
# set(FETCHCONTENT_QUIET OFF)
# set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
#
# NOTE: Also update env vars used for caching in
#         - .github/actions/configure-cmake/action.yml
#         - .github/workflows/cmake.yml
# FetchContent_Declare(
#         SomeLib
#         GIT_REPOSITORY https://github.com/<SomeUser>/<SomeLib>.git
#         GIT_TAG        <some_git_hash> # <which tag/branch @ <some_date>>
#         GIT_SHALLOW    1  # works only with branches or tags, not with arbitrary commit hashes
# )
#
# FetchContent_MakeAvailable(SomeLib)

###############################################################################

# external dependencies with find_package

# find_package(Threads REQUIRED)

###############################################################################

# NOTE: update executable name in .github/workflows/cmake.yml:25 when changing name here
add_executable(${MAIN_EXECUTABLE_NAME}
    main.cpp
        server/ApiTypes.h
        server/BankService.h
        server/BankService.cpp
        server/Scheduler.h
        server/Scheduler.cpp
        server/HttpServer.h
        server/HttpServer.cpp
        models/cbtransaction/Transaction.h
        models/cbtransaction/Transaction.cpp
        models/cbaccount/BankAccount.cpp
        models/cbaccount/BankAccount.h
        models/cbaccount/PersonalAccount.h
        models/cbaccount/PersonalAccount.cpp
        models/cbaccount/SavingsAccount.h
        models/cbaccount/SavingsAccount.cpp
        models/cbaccount/RetirementAccount.h
        models/cbaccount/RetirementAccount.cpp
        models/cbaccount/AccountFactory.h
        models/cbaccount/AccountFactory.cpp
        models/cbloan/Loan.h
        models/cbloan/Loan.cpp
        models/cbinstrument/Bond.h
        utils/cbdate/DateUtils.h
        utils/cbdate/DateUtils.cpp
        utils/cbbanking/InterestAccrual.h
        utils/cbbanking/InterestAccrual.cpp
        utils/cbvalidation/Validator.h
        utils/cbvalidation/Validator.cpp
        utils/cbexception/BankExceptions.h
        utils/cbexception/BankExceptions.cpp
        utils/cblogger/Logger.h
        utils/cblogger/Logger.cpp
        utils/cbjson/JsonLoader.h
        utils/cbjson/JsonLoader.cpp
        utils/cbjson/JsonHelpers.h
        utils/cbjson/JsonStore.h
        utils/cbjson/JsonStore.cpp
        models/cbclient/Client.h
        models/cbclient/Client.cpp
        models/cbaccount/InvestmentAccount.h
        models/cbaccount/InvestmentAccount.cpp
        utils/cbcurrency/CurrencyConverter.h
        utils/cbcurrency/CurrencyConverter.cpp
        utils/cbbanking/ReportGenerator.h
        models/cbbank/Bank.h
        models/cbbank/Bank.cpp
)

# NOTE: Add all defined targets (e.g. executables, libraries, etc. )
# NOTE: RUN_SANITIZERS is optional, if it's not present it will default to true
set_compiler_flags(RUN_SANITIZERS TRUE TARGET_NAMES ${MAIN_EXECUTABLE_NAME})
# set_compiler_flags(TARGET_NAMES ${MAIN_EXECUTABLE_NAME} ${FOO} ${BAR})
# where ${FOO} and ${BAR} represent additional executables or libraries
# you want to compile with the set compiler flags

###############################################################################

# use SYSTEM so cppcheck and clang-tidy do not report warnings from these directories
target_include_directories(${MAIN_EXECUTABLE_NAME} SYSTEM PRIVATE ext/include)
# Windows sockets for cpp-httplib
if(WIN32)
    target_link_libraries(${MAIN_EXECUTABLE_NAME} ws2_32)
endif()
# target_include_directories(${MAIN_EXECUTABLE_NAME} SYSTEM PRIVATE ${<SomeLib>_SOURCE_DIR}/include)
# target_link_directories(${MAIN_EXECUTABLE_NAME} PRIVATE ${<SomeLib>_BINARY_DIR}/lib)
# target_link_libraries(${MAIN_EXECUTABLE_NAME} <SomeLib>)

###############################################################################

# copy binaries to "bin" folder; these are uploaded as artifacts on each release
# DESTINATION_DIR is set as "bin" in cmake/Options.cmake:6
install(TARGETS ${MAIN_EXECUTABLE_NAME} DESTINATION ${DESTINATION_DIR})
if(APPLE)
    install(FILES launcher.command DESTINATION ${DESTINATION_DIR})
endif()

copy_files(FILES tastatura.txt DIRECTORY data COPY_TO_DESTINATION TARGET_NAME ${MAIN_EXECUTABLE_NAME})
# copy_files(FILES tastatura.txt config.json DIRECTORY images sounds COPY_TO_DESTINATION TARGET_NAME ${MAIN_EXECUTABLE_NAME})
# copy_files(DIRECTORY images sounds COPY_TO_DESTINATION TARGET_NAME ${MAIN_EXECUTABLE_NAME})
