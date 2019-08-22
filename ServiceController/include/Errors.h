#pragma once
#include <exception>
#include <string>

namespace ServiceController
{
    class ServiceStartupException : public std::exception {
    private:
        std::string _message;
    public:
        ServiceStartupException(const char* const message) : std::exception() {
            _message = message;
        };
        const char* what() const throw() { return _message.c_str(); }
    };

    class ServiceControlException : public std::exception {
    private:
        std::string _message;
    public:
        ServiceControlException(const char* const message) : std::exception() {
            _message = message;
        };
        const char* what() const throw() { return _message.c_str(); }
    };
}