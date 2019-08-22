#pragma once

#include <string>

namespace UIInterop
{
    using System::String;
    using System::Collections::Generic::List;

    public ref class Process {
    public:
        property String^ Title;
        property String^ ProcessName;

        Process(wchar_t*, wchar_t*);
        explicit Process(const std::wstring&, const std::wstring&);
    };
}