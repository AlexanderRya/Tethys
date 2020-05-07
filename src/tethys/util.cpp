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

    Funct load_symbol(HMODULE handle, LPCSTR symbol) {
        return reinterpret_cast<void(*)()>(GetProcAddress(handle, symbol));
    }

    void close_module(HMODULE handle) {
        FreeLibrary(handle);
    }
#elif __linux__
    void* load_module(const char* name) {
        return dlopen(name, RTLD_LAZY | RTLD_LOCAL);
    }

    Funct load_symbol(void* handle, const char* symbol) {
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
} // namespace tethys::util