#include <Windows.h>
#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>
#include <exception>
#include "powerManager.h"

constexpr auto SERVICE_NAME = L"PowerManager";

SERVICE_STATUS m_ServiceStatus;
SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
HANDLE g_hTerminateEvt;
PowerManager manager;

void ServiceExecutionThread();
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void WINAPI ServiceCtrlHandler(DWORD Opcode);
void InstallService(const std::wstring& serviceExe);
void DeleteService();
void StartServiceThread();

constexpr auto COMMAND_INSTALL = L"install";
constexpr auto COMMAND_REMOVE = L"remove";
#ifdef _DEBUG
constexpr auto COMMAND_DEBUG = L"debug";
#endif

constexpr auto SERVICE_DISPLAY_NAME = L"Power Manager";
constexpr auto SERVICE_TEXT_DESCRIPTION = L"Service for controlling power manager application";

constexpr auto SERVICE_TYPE = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;

class ServiceInstallationException : public std::exception 
{
private:
    std::string _message;
public:
    ServiceInstallationException(const char* const message) : std::exception()
    {
        _message = message;
    }

    const char* what() const throw() { return _message.c_str(); }
};

class ServiceDeleteException : public std::exception
{
private:
    std::string _message;
public:
    ServiceDeleteException(const char* const message) : std::exception()
    {
        _message = message;
    }

    const char* what() const throw() { return _message.c_str(); }
};

class ServiceRuntimeException : public std::exception
{
private:
    std::string _message;
public:
    ServiceRuntimeException(const char* const message) : std::exception()
    {
        _message = message;
    }

    const char* what() const throw() { return _message.c_str(); }
};

int wmain(int argc, wchar_t* argv[])
{
    if (argc > 1)
    {
        std::wstring nameOfExe = argv[0];
        std::wstring command = argv[1];
        if (command == COMMAND_INSTALL)
        {
            try {
                InstallService(nameOfExe);
            }
            catch (ServiceInstallationException e) {
                std::cout << e.what() << std::endl;
                return -1;
            }
        }
        else if (command == COMMAND_REMOVE)
        {
            try {
                DeleteService();
            }
            catch (ServiceDeleteException e) {
                std::cout << e.what() << std::endl;
                return -1;
            }
        }

#ifdef _DEBUG
        else if (command == COMMAND_DEBUG)
        {
            StartServiceThread();
            while (true)
                std::this_thread::sleep_for(std::chrono::seconds(1));
        }
#endif
    }
    else
    {
        SERVICE_TABLE_ENTRY dispatchTable[] =
        {
            { const_cast<LPWSTR>(SERVICE_NAME), static_cast<LPSERVICE_MAIN_FUNCTION>(ServiceMain) },
            { NULL, NULL } // NULL terminator for array of structs
        };

        // - registr the EXE with the SCM
        // - give the SCM a pointer to a service main function to call
        //   when it wants to start the service. 
        StartServiceCtrlDispatcher(dispatchTable);
    }

    return 0;
}

void InstallService(const std::wstring &serviceExe)
{
    // Installing the service from inside the application is clean. 
    // The installion can be done using a separate application. 
    BOOL retVal = TRUE;	// RMD added
    std::wstring strDir;
    strDir.reserve(1024);

    GetCurrentDirectoryW(strDir.size(), &strDir[0]);
    strDir.append(L"\\");
    strDir.append(serviceExe);

    std::wstring filePath = std::wstring(strDir.begin() + 1, strDir.end());
    std::wcout << filePath << std::endl;
    auto closeServiceHandle = [](SC_HANDLE* p) { CloseServiceHandle(*p); delete p; };

    // get a handle to the Service Control Manger (SCM)
    auto schSCManager = std::shared_ptr<SC_HANDLE>(
        new SC_HANDLE(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)),
        closeServiceHandle
    );

    if (*(schSCManager.get()) == nullptr)
        throw ServiceInstallationException("Could not open Service Control Manager");

    auto schService = std::shared_ptr<SC_HANDLE>(new SC_HANDLE(CreateService(
        *schSCManager.get(),
        SERVICE_NAME,			// name of service 
        SERVICE_DISPLAY_NAME,	// service name to display
        SERVICE_ALL_ACCESS,		// desired access 
        SERVICE_TYPE, // service type 
        SERVICE_AUTO_START,	// start type /// try SERVICE_AUTO_START
        SERVICE_ERROR_NORMAL,	// error control type 
        filePath.c_str(),		// service's binary 
        NULL,  // no load ordering group 
        NULL,  // no tag identifier 
        NULL,  // no dependencies
        NULL,  // LocalSystem account
        NULL)), closeServiceHandle); // no password

    if (*(schService.get()) == nullptr)
        throw ServiceInstallationException("Create Service Error: " + GetLastError());

    // add the description of the service
    SERVICE_DESCRIPTION l_sDescription;
    l_sDescription.lpDescription = const_cast<wchar_t*>(SERVICE_TEXT_DESCRIPTION);

    if (ChangeServiceConfig2(*schService.get(), SERVICE_CONFIG_DESCRIPTION, &l_sDescription) == FALSE)
        throw ServiceInstallationException("ChangeServiceConfig2 Error: " + GetLastError());

    std::cout << "Service installed" << std::endl;
}

void DeleteService()
{
    SERVICE_STATUS status;	// for QueryServiceStatus 
    BOOL retVal;

    auto closeServiceHandle = [](SC_HANDLE* p) { CloseServiceHandle(*p); delete p; };
    // get a handle to the Service Control Manger (SCM)
    auto schSCManager = std::shared_ptr<SC_HANDLE>(
        new SC_HANDLE(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)),
        closeServiceHandle
    );

    if (*(schSCManager.get()) == nullptr)
        throw ServiceDeleteException("OpenSCManager failed " + GetLastError());

    // get a handle to your service 
    auto hService = std::shared_ptr<SC_HANDLE>(
        new SC_HANDLE(OpenService(*schSCManager.get(), SERVICE_NAME, SERVICE_ALL_ACCESS)),
        closeServiceHandle
    );

    if (*(hService.get()) == nullptr)
        throw ServiceDeleteException("OpenService failed " + GetLastError());

    // Stop the service if neccessary
    retVal = QueryServiceStatus(*hService.get(), &status);
    if (!retVal)
        throw ServiceDeleteException("QueryServiceStatus failed " + GetLastError());

    if (status.dwCurrentState != SERVICE_STOPPED)
    {
        std::cout << "Stopping Service...\n";
        retVal = ControlService(*hService.get(),
            SERVICE_CONTROL_STOP,
            &status
        );

        if (!retVal)
            throw ServiceDeleteException("ControlService Error " + GetLastError());

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Remove the service
    retVal = DeleteService(*hService.get());
    if (!retVal)
        throw ServiceDeleteException("DeleteService Error " + GetLastError());

    std::cout << "Service removed\n";
}

void ServiceExecutionThread()
{
    manager.start();
}// end ServiceExecutionThread

void StartServiceThread()
{
    std::thread thread1([] {
        ServiceExecutionThread();
    });

    thread1.detach();
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{   
    memset(&m_ServiceStatus, 0, sizeof(m_ServiceStatus));

    m_ServiceStatus.dwServiceType = SERVICE_TYPE;
    m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;

    m_ServiceStatusHandle = RegisterServiceCtrlHandler(
        SERVICE_NAME,
        ServiceCtrlHandler
    );

    if (m_ServiceStatusHandle == NULL) {
        OutputDebugStringW(L"Service Failed to start");
        return;
    }

    m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;

    if (!SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus))
        OutputDebugStringW(L"Service Failed to start");

    StartServiceThread();

    g_hTerminateEvt = CreateEvent(0, FALSE, FALSE, 0);

    if (g_hTerminateEvt == NULL)
        return;

    WaitForSingleObject(g_hTerminateEvt, INFINITE);

    return;
}

void WINAPI ServiceCtrlHandler(DWORD Opcode)
{
    switch (Opcode)
    {
    case SERVICE_CONTROL_PAUSE:
        manager.pause();

        m_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
        SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);

        break;
    case SERVICE_CONTROL_CONTINUE:
        manager.resume();
        m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);

        break;
    case SERVICE_CONTROL_STOP:
        manager.stop();

        m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);

        SetEvent(g_hTerminateEvt);

        break;
    case SERVICE_CONTROL_INTERROGATE:
        //SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
        break;
    }
}