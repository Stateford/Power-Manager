#include "powerManager.h"

#include <thread>
#include <iostream>
#include <powrprof.h>

PowerManager::PowerManager() :
    _current(nullptr, LocalFree),
    _previous(nullptr, LocalFree)
{
    _config = std::make_shared<Config>(Config());
    _config->read();

    getCurrentScheme();
}

PowerManager::PowerManager(const PowerManager& other) noexcept :
    _current(nullptr, LocalFree),
    _previous(nullptr, LocalFree)
{
    _current = MAKE_SMART_GUID(new GUID(*other._current));
    _previous = MAKE_SMART_GUID(new GUID(*other._previous));
    
    _config = other._config;
    _running = other._running.load();
}

PowerManager::PowerManager(PowerManager&& other) noexcept :
    _current(nullptr, LocalFree),
    _previous(nullptr, LocalFree)
{
    _current = std::move(other._current);
    _previous = std::move(other._previous);

    _config = std::move(other._config);
    _running = other._running.load();
}

PowerManager& PowerManager::operator=(const PowerManager& other) noexcept
{
    if (this == &other)
        return *this;

    _current = MAKE_SMART_GUID(new GUID(*other._current));
    _previous = MAKE_SMART_GUID(new GUID(*other._previous));

    _config = other._config;
    _running = other._running.load();

    return *this;
}

PowerManager& PowerManager::operator=(PowerManager&& other) noexcept
{
    if (this == &other)
        return *this;

    _current = std::move(other._current);
    _previous = std::move(other._previous);

    _config = std::move(other._config);
    _running = other._running.load();

    return *this;
}

void PowerManager::getCurrentScheme()
{
    _previous = std::move(_current);
    GUID* current;
    PowerGetActiveScheme(NULL, &current);
    _current = std::move(MAKE_SMART_GUID(current));
}

void PowerManager::restorePreviousScheme()
{
    PowerSetActiveScheme(NULL, _previous.get());

    _current = std::move(_previous);
    _previous = MAKE_SMART_GUID(nullptr);
}

void PowerManager::setBalanced()
{
    _previous = std::move(_current);
    PowerSetActiveScheme(NULL, POWER_BALANCED);
    _current = MAKE_SMART_GUID(POWER_PERFORMANCE);
}

void PowerManager::setPerformance()
{
    _previous = std::move(_current);
    PowerSetActiveScheme(NULL, POWER_PERFORMANCE);
    _current = MAKE_SMART_GUID(POWER_PERFORMANCE);
}

bool PowerManager::checkWindows()
{
    std::vector<Window> windows = getWindows();
    for (const auto &window : windows)
    {
        for (const auto& windowString : _config->getWindows())
        {
            if (window == windowString)
                return true;
        }
    }
    return false;
}

void PowerManager::start()
{
    _running.store(true);
    _config->read();
    while (_running.load())
    {
        if (!_paused.load())
        {
            std::wstring foo = checkWindows() ? L"TRUE" : L"FALSE";
            std::wcout << foo << std::endl;

            bool result = checkWindows();

            if (result && !_isChanged)
            {
                setPerformance();
                _isChanged = true;
            }
            else if (!result && _isChanged)
            {
                restorePreviousScheme();
                _isChanged = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
}

void PowerManager::stop()
{
    _running.store(false);
}

void PowerManager::resume()
{
    _paused.store(false);
}

void PowerManager::pause()
{
    _paused.store(true);
}