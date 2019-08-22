#include "Service.h"
#include "Errors.h"


namespace ServiceController
{
    Service::Service(const std::wstring &serviceName)
    {
        _serviceName = serviceName;
    }

    Service::Service(const Service& other) noexcept
    {
        _serviceName = other._serviceName;
        _serviceHandle = other._serviceHandle;
        _SCM = other._SCM;

        _status = other._status;
        _pauseEnabled = other._pauseEnabled;
    }

    Service::Service(Service&& other) noexcept
    {
        _serviceName = other._serviceName;
        _serviceHandle = std::move(other._serviceHandle);
        _SCM = std::move(other._SCM);

        _status = other._status;
        _pauseEnabled = other._pauseEnabled;
    }

    Service& Service::operator=(const Service& other) noexcept
    {
        if (this == &other)
            return *this;

        _serviceName = other._serviceName;
        _serviceHandle = other._serviceHandle;
        _SCM = other._SCM;

        _status = other._status;
        _pauseEnabled = other._pauseEnabled;

        return *this;
    }

    Service& Service::operator=(Service&& other) noexcept
    {
        if (this == &other)
            return *this;

        _serviceName = other._serviceName;
        _serviceHandle = std::move(other._serviceHandle);
        _SCM = std::move(other._SCM);

        _status = other._status;
        _pauseEnabled = other._pauseEnabled;

        return *this;
    }

    void Service::openService()
    {
        auto closeServiceHandle = [](SC_HANDLE* p) { CloseServiceHandle(*p); delete p; };

        _SCM = std::shared_ptr<SC_HANDLE>(
            new SC_HANDLE(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)),
            closeServiceHandle
        );

        if (*_SCM.get() == nullptr)
            throw ServiceStartupException("Could not start Service Control Manager");

        _serviceHandle = std::shared_ptr<SC_HANDLE>(
            new SC_HANDLE(OpenService(*_SCM, _serviceName.c_str(), GENERIC_ALL)),
            closeServiceHandle
        );

        if (*_serviceHandle.get() == nullptr)
            throw ServiceStartupException("Could not obtain service handle");
    }

    void Service::queryStatus()
    {
        DWORD dwBytesNeeded;
        // check for error
        int err = QueryServiceStatusEx(
            *_serviceHandle,
            SC_STATUS_PROCESS_INFO,
            reinterpret_cast<LPBYTE>(&_status),
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded);

        if (!err)
            throw ServiceControlException("error querying service: " + GetLastError());

        // check if pause is enabled
        _pauseEnabled = _status.dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE;
    }

    void Service::startService() const
    {
        BOOL err = StartService(*_serviceHandle, NULL, NULL);
        if (!err)
            throw ServiceControlException("error starting service: " + GetLastError());
    }

    void Service::stopService() const
    {
        SERVICE_STATUS status;
        BOOL err = ControlService(*_serviceHandle, SERVICE_CONTROL_STOP, &status);
        if (!err)
            throw ServiceStartupException("error stopping service: " + GetLastError());
    }

    void Service::resumeService() const
    {
        if (_pauseEnabled)
        {
            SERVICE_STATUS status;
            BOOL err = ControlService(*_serviceHandle, SERVICE_CONTROL_CONTINUE, &status);
            if (!err)
                throw ServiceStartupException("error resuming service: " + GetLastError());
        }
    }

    void Service::pauseService() const
    {
        if (_pauseEnabled)
        {
            SERVICE_STATUS status;
            BOOL err = ControlService(*_serviceHandle, SERVICE_CONTROL_PAUSE, &status);
            if (!err)
                throw ServiceStartupException("error pausing service: " + GetLastError());
        }
    }
}