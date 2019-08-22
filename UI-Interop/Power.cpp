#include "Power.h"
#include "Window.h"

#include <string>
#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>

namespace UIInterop
{
    Power::Power()
    {
        _config = new Config();
    }

    Power::~Power()
    {
        delete _config;
        _config = nullptr;
    }

    List<String^>^ Power::getConfigWindows()
    {
        List<String^>^ list = gcnew List<String^>();

        _config->read();
        std::vector<std::wstring> windows = _config->getWindows();

        for (const auto& p : windows)
            list->Add(gcnew String(p.c_str()));

        return list;
    }

    List<Process^>^ Power::getOpenProcesses()
    {
        List<Process^>^ list = gcnew List<Process^>();

        std::vector<Window> processes = getWindows();

        for (const auto& p : processes)
            list->Add(gcnew Process(p.getTitle().c_str(), p.getProcessName().c_str()));

        return list;
    }

    void Power::addProcess(String^ processName)
    {
        std::wstring result = msclr::interop::marshal_as<std::wstring>(processName);
        _config->addWindow(result);
        _config->write();
    }

    void Power::addProcess(Process^ process)
    {
        std::wstring result = msclr::interop::marshal_as<std::wstring>(process->ProcessName);
        _config->addWindow(result);
        _config->write();
    }

    void Power::removeProcess(String^ processName)
    {
        std::wstring result = msclr::interop::marshal_as<std::wstring>(processName);
        _config->removeWindow(result);
        _config->write();
    }
}