#pragma once
#include <filesystem>
#include <string>

std::filesystem::path ExeDir();
std::filesystem::path AssetPath(const std::string& name);