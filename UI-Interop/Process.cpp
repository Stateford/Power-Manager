#include "Process.h"

namespace UIInterop
{
    Process::Process(wchar_t *title, wchar_t *processName)
    {
        Title = gcnew String(title);
        ProcessName = gcnew String(processName);
    }

    Process::Process(const std::wstring &title, const std::wstring &processName)
    {
        Title = gcnew String(title.c_str());
        ProcessName = gcnew String(processName.c_str());
    }
}