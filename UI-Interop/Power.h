#pragma once

#include "Process.h"
#include "config.h"
#include <memory>
#include <vector>

namespace UIInterop
{
    using System::Collections::Generic::List;
    using System::String;

    public ref class Power {
    private:
        Config *_config;

    public:
        Power();
        ~Power();

        List<String^>^ getConfigWindows();
        List<Process^>^ getOpenProcesses();
        void addProcess(String^);
        void addProcess(Process^);
        void removeProcess(String^);
    };
}