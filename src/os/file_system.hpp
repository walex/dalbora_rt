#ifndef __os_file_system_hpp__
#define __os_file_system_hpp__

#include "platform.hpp"

std::filesystem::path file_system_get_current_executable_directory();
std::vector<char> file_system_file_to_bin(const char* const path);
std::filesystem::path file_system_path_join(const std::filesystem::path& p1, const std::filesystem::path& p2);
#endif