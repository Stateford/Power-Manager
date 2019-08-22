#pragma once

#ifdef SERVICE_MANAGER_EXPORTS
#define SERVICE_MANAGER_API __declspec(dllexport)
#else
#define SERVICE_MANAGER_API __declspec(dllimport)
#endif

#include <string>
#include <memory>
#include <Windows.h>

namespace ServiceController
{
    class Service {
    private:
        std::wstring _serviceName;

        std::shared_ptr<SC_HANDLE> _SCM;
        std::shared_ptr<SC_HANDLE> _serviceHandle;

        SERVICE_STATUS_PROCESS _status;

        bool _pauseEnabled;

    public:
        SERVICE_MANAGER_API Service(const std::wstring&);
        SERVICE_MANAGER_API Service(const Service&) noexcept;
        SERVICE_MANAGER_API Service(Service&&) noexcept;

        SERVICE_MANAGER_API Service& operator=(const Service&) noexcept;
        SERVICE_MANAGER_API Service& operator=(Service&&) noexcept;

        SERVICE_MANAGER_API void openService();
        SERVICE_MANAGER_API void queryStatus();

        SERVICE_MANAGER_API DWORD getStatus() const { return _status.dwCurrentState; };

        SERVICE_MANAGER_API void startService() const;
        SERVICE_MANAGER_API void stopService() const;
        SERVICE_MANAGER_API void resumeService() const;
        SERVICE_MANAGER_API void pauseService() const;
    };
}