#include "Window.h"
#include <Psapi.h>
#include <filesystem>
#include <Windows.h>


Window::Window(HWND hWnd, const std::wstring &title, const std::wstring& processName)
{
    this->_hwnd = hWnd;
    this->_title = title;
    this->_processName = processName;
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    std::vector<Window>* windows = reinterpret_cast<std::vector<Window>*>(lParam);

    if (IsWindowVisible(hWnd))
    {
        std::wstring title;
        title.resize(MAX_PATH);
        int size = GetWindowText(hWnd, &title[0], static_cast<int>(title.size()));
        title.resize(size);

        if (!title.empty() || size > 0)
        {
            title += L'\0';

            DWORD pid = 1;

            GetWindowThreadProcessId(hWnd, &pid);

            HANDLE hProcess = OpenProcess(
                PROCESS_QUERY_INFORMATION,
                FALSE,
                pid
            );

            std::wstring buffer;
            buffer.resize(1024);
            DWORD size = static_cast<DWORD>(buffer.size());
            QueryFullProcessImageName(hProcess, NULL, &buffer[0], &size);
            buffer.resize(size);

            std::filesystem::path path(buffer);
            std::wstring fullPath = path.filename().wstring();

            windows->push_back(Window(hWnd, title, fullPath));
        }
    }

    return true;
}

BOOL CALLBACK enumWindowStationProc(LPWSTR lpszWindowStation, LPARAM lParam)
{
    auto stations = reinterpret_cast<std::vector<std::wstring>*>(lParam);
    std::wstring title = lpszWindowStation;
    if (!title.find(L"Service"))
        stations->emplace_back(lpszWindowStation);
    return TRUE;
}


BOOL CALLBACK enumDesktopsProc(LPWSTR lpszWindowDesktop, LPARAM lparam)
{
    auto desktops = reinterpret_cast<std::vector<std::wstring>*>(lparam);
    std::wstring title = lpszWindowDesktop;
    if(title == L"Default")
        desktops->emplace_back(title);

    return TRUE;
}

bool Window::operator==(const std::wstring& title) const
{
    return this->_processName == title;
}

bool Window::operator==(const Window& other) const
{
    return this->_processName == other._processName;
}

void getWindows(std::vector<Window>& windows)
{

    auto stationDeleter = [](HWINSTA* p) {
        CloseWindowStation(*p);
        delete p;
    };

    auto hStation = std::unique_ptr<HWINSTA, decltype(stationDeleter)>(new HWINSTA(OpenWindowStationW(
        L"WinSta0",
        FALSE,
        GENERIC_READ
    )), stationDeleter);

    SetProcessWindowStation(*hStation.get());

    auto desktopDeleter = [](HDESK* p) {
        CloseDesktop(*p);
        delete p;
    };

    auto desk = std::unique_ptr<HDESK, decltype(desktopDeleter)>(
        new HDESK(
            OpenDesktop(
                L"Default",
                NULL,
                FALSE,
                GENERIC_READ
            )
        ), desktopDeleter
    );

    if (*desk.get() == NULL)
        return;

    EnumDesktopWindows(
        *desk.get(),
        &EnumWindowsProc,
        reinterpret_cast<LPARAM>(&windows)
    );
}

std::vector<Window> getWindows()
{
    std::vector<Window> windows;
    getWindows(windows);
    return windows;
}