#ifndef LOGGING
#define LOGGING

#include <string>
#include <iostream>

#define LOG_ERROR(msg) logger_error(msg, __FILE__, __LINE__)
#define CODE_ERROR(msg, file, line) logger_code_error(msg, file, line, __FILE__, __LINE__)

void logger_error(std::string msg, const char* file, int line)
{
    std::cout << msg << " <<COMPILER:[" << file << ":" << line << "]>>" << std::endl;
    exit(1);
}

void logger_code_error(std::string msg, std::string codeFile, unsigned int codeLine, const char* file, int line)
{
    std::cout << msg << " [" << codeFile << ":" << codeLine << "] <<COMPILER:[" << file << ":" << line << "]>>" << std::endl;
    exit(1);
}

#endif