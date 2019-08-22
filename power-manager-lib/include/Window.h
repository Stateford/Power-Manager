#pragma once

#ifdef POWER_MANAGER_EXPORTS
#define POWER_MANAGER_API __declspec(dllexport)
#else
#define POWER_MANAGER_API __declspec(dllimport)
#endif

#include <Windows.h>
#include <vector>
#include <string>


class Window {
private:
    DWORD _pid;
    HWND _hwnd;
    std::wstring _title;
    std::wstring _processName;
public:
    POWER_MANAGER_API Window(HWND, const std::wstring&, const std::wstring&);

    POWER_MANAGER_API std::wstring getTitle() const { return _title; }
    POWER_MANAGER_API std::wstring getProcessName() const { return _processName; }

    bool operator==(const Window&) const;
    bool operator==(const std::wstring&) const;
};

void getWindows(std::vector<Window>&);
POWER_MANAGER_API std::vector<Window> getWindows();
BOOL CALLBACK EnumWindowsProc(HWND, LPARAM);
BOOL CALLBACK enumWindowStationProc(LPWSTR, LPARAM);
BOOL CALLBACK enumDesktopsProc(LPWSTR, LPARAM);
