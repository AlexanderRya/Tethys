#include <tethys/util.hpp>

#include <cmath>
#include <ctime>
#include <chrono>
#include <string>
#include <cstring>

namespace tethys::util {
#if _WIN32
    HMODULE load_module(LPCSTR name) {
        return LoadLibraryA(name);
    }

    auto load_symbol(HMODULE handle, LPCSTR symbol) -> void(*)() {
        return reinterpret_cast<void(*)()>(GetProcAddress(handle, symbol));
    }

    void close_module(HMODULE handle) {
        FreeLibrary(handle);
    }
#elif __linux__
    void* load_module(const char* name) {
        return dlopen(name, RTLD_LAZY | RTLD_LOCAL);
    }

    auto load_symbol(void* handle, const char* symbol) -> void(*)() {
        return reinterpret_cast<void(*)()>(dlsym(handle, symbol));
    }

    void close_module(void* handle) {
        dlclose(handle);
    }
#endif

    std::string timestamp() {
        namespace ch = std::chrono;

        std::time_t time = ch::duration_cast<ch::seconds>(ch::system_clock::now().time_since_epoch()).count();

        std::string buf(128, '\0');
        buf.resize(std::strftime(buf.data(), buf.size(), "%Y-%m-%d %X", std::localtime(&time)));

        return buf;
    }

    void print(const std::string& val) {
#if defined(__linux__)
        write(1, val.c_str(), val.size());
#else
        auto stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        WriteFile(stdout_handle, val.c_str(), val.size(), nullptr, nullptr);
#endif
    }

    void print(const char* str) {
        print(std::string(str));
    }

    void print(const void* addr) {
        std::string str(std::snprintf(nullptr, 0, "%p", addr), '\0');
        std::snprintf(str.data(), str.size(), "%p", addr);

        print(str);
    }
} // namespace tethys::util