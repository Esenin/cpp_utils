#ifndef EXTERNALSORT_ENVIRONMENT_H
#define EXTERNALSORT_ENVIRONMENT_H

#include <inttypes.h>
#include <string>
#include <iostream>

#ifndef NDEBUG
  #define DEBUG(x) do { std::cerr << "DBG: " << x << "\n"; } while (false)
  #define DEBUG2(x) do { std::cerr << "DBG: " << __func__ << ":" << #x << ": " << x << "\n"; } while (false)
  #define WARNING(x) do { std::cerr << "WARN: " << __FILE__ << " > " << __func__ << ": " << x << "\n"; } while (false)
#else
  #define DEBUG(x) do {} while (false)
  #define DEBUG2(x) do {} while (false)
  #define WARNING(x) do {} while (false)
#endif
#define ERROR(x) do { std::cerr << "ERR: " << __FILE__ << " > " << __func__ << ": " << x << "\n"; \
                      ErrorExit("Error exit"); } while (false)

/// @namespace environment contains functions to get information from external environment
/// and other interact with operating system
namespace environment {

/// @brief Checks condition and if it fails(equals to false) exit program with error code
/// @param cond condition to check
/// @param msg message to strerr
void Assert(bool cond, std::string msg);

/// @brief Exit program with error status
/// @param reason message to stderr
void ErrorExit(const std::string &reason);

/// @brief Tells operating system to bound virtual memory for the program via SETRLIMIT
/// Should fail if you try to set more bytes than RLIM_MAX
/// @param bytes virtual memory limit in bytes
/// @returns true if success
bool TrySetMemoryLimit(uint64_t bytes);

/// @brief uses STAT to get file size
/// @param filepath full or relative path to a file
int64_t FileSize(const char *filepath);

} // environment

#endif //EXTERNALSORT_ENVIRONMENT_H
