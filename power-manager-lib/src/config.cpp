#include "config.h"

#include <Windows.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <shtypes.h>
#include <fstream>
#include <filesystem>
#include <memory>
#include <functional>

constexpr auto FILE_PATH = L"\\PowerManager\\settings.PMAN";

PWSTR Config::createPath() const
{
    PWSTR path;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);

    if (!SUCCEEDED(hr))
        return nullptr;

    return path;
}

std::wstring Config::getPath() const
{
    std::unique_ptr<WCHAR, decltype(&CoTaskMemFree)> path(createPath(), CoTaskMemFree);
    std::wstring outPath(&*path);
    outPath.append(FILE_PATH);
    return outPath;
}

const void Config::addWindow(const std::wstring& title)
{
    _windows.push_back(title);
}

const void Config::addWindow(const Window& window)
{
    _windows.push_back(window.getProcessName().c_str());
}

const void Config::removeWindow(const std::wstring& title)
{
    auto it = std::remove_if(_windows.begin(), _windows.end(), [=](const std::wstring& value) {
        return title == value;
    });
    bool any_change = it != _windows.end();
    _windows.erase(it, _windows.end());
}

void const Config::write() const
{
    std::wstring filePath = getPath();
    std::filesystem::path path(filePath);
    
    if (!std::filesystem::exists(path.parent_path()))
        std::filesystem::create_directory(path.parent_path());

    std::wofstream outFile;
    outFile.open(filePath.c_str());

    for (const auto& p : _windows)
    {
        std::wstring newlined = p;
        newlined += L"\n";
        outFile << newlined;
    }
}

void const Config::read()
{
    _windows.clear();
    _windows.resize(0);
    std::wstring path = getPath();
    
    bool result = std::filesystem::exists(path);
    if (result) 
    {
        std::wifstream inFile;
        inFile.open(path.c_str());

        while (!inFile.eof())
        {
            std::wstring processName;
            inFile >> processName;

            if (processName.empty())
                break;

            if (inFile.fail())
            {
                inFile.clear();
                continue;
            }
            _windows.push_back(processName);
        }
    }
}