#pragma once

#include "Window.h"
#include <string>
#include <vector>

#ifdef POWER_MANAGER_EXPORTS
#define POWER_MANAGER_API __declspec(dllexport)
#else
#define POWER_MANAGER_API __declspec(dllimport)
#endif

class Config {
private:
    std::vector<std::wstring> _windows;

    PWSTR createPath() const;
    std::wstring getPath() const;

public:

    POWER_MANAGER_API const std::vector<std::wstring> getWindows() const { return _windows; }

    POWER_MANAGER_API const void addWindow(const Window& window);
    POWER_MANAGER_API const void addWindow(const std::wstring& title);

    POWER_MANAGER_API const void removeWindow(const std::wstring& title);

    POWER_MANAGER_API const void read();
    POWER_MANAGER_API const void write() const;
};