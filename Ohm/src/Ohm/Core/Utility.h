#pragma once

#include "Ohm/Core/Log.h"

#define OHM_BIND_FN(fn) [this](auto&& ... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

template<typename T>
std::string TypeName()
{
    std::string TypeName = typeid(T).name();
#if defined(__clang__) || defined(__GNUG__)
    int status;
    char *demangled_name = abi::__cxa_demangle(TypeName.c_str(), NULL, NULL, &status);
    if(status == 0)
    {
        TypeName = demangled_name;
        std::free(demangled_name);
    }
#endif
    return TypeName;
}
