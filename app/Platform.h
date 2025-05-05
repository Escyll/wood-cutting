#pragma once

#include <string>
#include <filesystem>

std::string toLinuxStyle(const std::filesystem::path& p);

std::string readFile(const std::string& path);
