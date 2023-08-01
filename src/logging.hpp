#include <iostream>

#pragma once

// Logging levels
enum class LogLevel { DEBUG, INFO, WARNING, ERROR };
static LogLevel currentLogLevel = LogLevel::DEBUG;

// ANSI escape codes for colors
constexpr char ANSI_RESET[]  = "\x1B[0m";
constexpr char ANSI_RED[]    = "\x1B[31m";
constexpr char ANSI_YELLOW[] = "\x1B[33m";
constexpr char ANSI_GREEN[]  = "\x1B[32m";
constexpr char ANSI_CYAN[]   = "\x1B[36m";

// Macros for conditional logging with colors
#define LOG_DEBUG(msg)                                                                             \
    if(currentLogLevel <= LogLevel::DEBUG)                                                         \
        std::cout << ANSI_GREEN << "[DEBUG] " << msg << ANSI_RESET << std::endl;
#define LOG_INFO(msg)                                                                              \
    if(currentLogLevel <= LogLevel::INFO)                                                          \
        std::cout << ANSI_CYAN << "[INFO] " << msg << ANSI_RESET << std::endl;
#define LOG_WARNING(msg)                                                                           \
    if(currentLogLevel <= LogLevel::WARNING)                                                       \
        std::cout << ANSI_YELLOW << "[WARNING] " << msg << ANSI_RESET << std::endl;
#define LOG_ERROR(msg)                                                                             \
    if(currentLogLevel <= LogLevel::ERROR)                                                         \
        std::cerr << ANSI_RED << "[ERROR] " << msg << ANSI_RESET << std::endl;
