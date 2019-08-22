#pragma once

#ifdef POWER_MANAGER_EXPORTS
#define POWER_MANAGER_API __declspec(dllexport)
#else
#define POWER_MANAGER_API __declspec(dllimport)
#endif

#include "Window.h"
#include "config.h"
#include <atomic>
#include <memory>


constexpr GUID *POWER_PERFORMANCE = const_cast<GUID*>(&GUID_MIN_POWER_SAVINGS);
constexpr GUID *POWER_BALANCED = const_cast<GUID*>(&GUID_MAX_POWER_SAVINGS);

class PowerManager {
private:
    typedef std::unique_ptr<GUID, decltype(&LocalFree)> SMART_GUID;

    SMART_GUID MAKE_SMART_GUID(GUID *value)
    {
        return SMART_GUID(value, LocalFree);
    }

    bool _isChanged = false;

    std::shared_ptr<Config> _config;
    std::atomic<bool> _running = false;
    std::atomic<bool> _paused = false;

    SMART_GUID _previous;
    SMART_GUID _current;

    void getCurrentScheme();
    void restorePreviousScheme();

    void setBalanced();
    void setPerformance();

    bool checkWindows();
public:
    POWER_MANAGER_API PowerManager();
    POWER_MANAGER_API PowerManager(const PowerManager&) noexcept;
    POWER_MANAGER_API PowerManager(PowerManager&&) noexcept;

    POWER_MANAGER_API PowerManager& operator=(const PowerManager&) noexcept;
    POWER_MANAGER_API PowerManager& operator=(PowerManager&&) noexcept;

    POWER_MANAGER_API void start();
    POWER_MANAGER_API void stop();
    POWER_MANAGER_API void pause();
    POWER_MANAGER_API void resume();
};