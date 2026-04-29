#ifndef __os_strings_hpp__
#define __os_strings_hpp__

#include "platform.hpp"

inline std::wstring utf8_to_wstring(const std::string& input)
{
    std::wstring result;
    size_t i = 0;

    while (i < input.size())
    {
        uint32_t codepoint = 0;
        unsigned char c = static_cast<unsigned char>(input[i]);

        if (c <= 0x7F)
        {
            codepoint = c;
            i += 1;
        }
        else if ((c >> 5) == 0x6) // 110xxxxx
        {
            if (i + 1 >= input.size()) throw std::runtime_error("Invalid UTF-8");

            codepoint = ((c & 0x1F) << 6) |
                (static_cast<unsigned char>(input[i + 1]) & 0x3F);
            i += 2;
        }
        else if ((c >> 4) == 0xE) // 1110xxxx
        {
            if (i + 2 >= input.size()) throw std::runtime_error("Invalid UTF-8");

            codepoint = ((c & 0x0F) << 12) |
                ((static_cast<unsigned char>(input[i + 1]) & 0x3F) << 6) |
                (static_cast<unsigned char>(input[i + 2]) & 0x3F);
            i += 3;
        }
        else if ((c >> 3) == 0x1E) // 11110xxx
        {
            if (i + 3 >= input.size()) throw std::runtime_error("Invalid UTF-8");

            codepoint = ((c & 0x07) << 18) |
                ((static_cast<unsigned char>(input[i + 1]) & 0x3F) << 12) |
                ((static_cast<unsigned char>(input[i + 2]) & 0x3F) << 6) |
                (static_cast<unsigned char>(input[i + 3]) & 0x3F);
            i += 4;
        }
        else
        {
            throw std::runtime_error("Invalid UTF-8");
        }

#if WCHAR_MAX <= 0xFFFF
        // Windows → UTF-16
        if (codepoint <= 0xFFFF)
        {
            result.push_back(static_cast<wchar_t>(codepoint));
        }
        else
        {
            codepoint -= 0x10000;
            result.push_back(static_cast<wchar_t>((codepoint >> 10) + 0xD800));
            result.push_back(static_cast<wchar_t>((codepoint & 0x3FF) + 0xDC00));
        }
#else
        // Linux/macOS → UTF-32
        result.push_back(static_cast<wchar_t>(codepoint));
#endif
    }

    return result;
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
