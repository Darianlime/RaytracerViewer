#define NOMINMAX
#include <windows.h>
#include "Paths.h"

std::filesystem::path ExeDir() {
    wchar_t buf[4096];
    DWORD n = GetModuleFileNameW(nullptr, buf, 4096);
    return std::filesystem::path(std::wstring(buf, n)).parent_path();
}

std::filesystem::path AssetPath(const std::string& name) {
    return ExeDir() / "assets" / name;
}