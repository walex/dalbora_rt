#ifndef __os_strings_hpp__
#define __os_strings_hpp__

#include "platform.hpp"

inline std::wstring to_wstring_ascii(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}

template <typename T>
std::string get_type_name() {
#ifdef __clang__
    std::string_view name = __PRETTY_FUNCTION__;
    auto start = name.find("T = ") + 4;
    auto end = name.find("]", start);
    return std::string(name.substr(start, end - start));
#elif defined(__GNUC__)
    std::string_view name = __PRETTY_FUNCTION__;
    auto start = name.find("T = ") + 4;
    auto end = name.find(";", start);
    return std::string(name.substr(start, end - start));
#elif defined(_MSC_VER)
    std::string_view name = __FUNCSIG__;
    auto start = name.find("get_type_name<") + 14;
    auto end = name.find(">(void)");
    return std::string(name.substr(start, end - start));
#endif
}
#endif
