#ifndef TETHYS_UTIL_HPP
#define TETHYS_UTIL_HPP

#include <tethys/types.hpp>

#include <string>
#include <sstream>
#include <utility>

#if _WIN32
    #include <Windows.h>
    #include <libloaderapi.h>
#elif __linux__
    #include <dlfcn.h>
    #include <unistd.h>
#endif

#include <type_traits>

namespace tethys::util {
    struct Funct {
        Funct(void(*f)()) : f(f) {}

        template <typename Ty>
        Ty as() const {
            return reinterpret_cast<Ty>(f);
        }
    private:
        void(*f)();
    };
#if _WIN32
    constexpr inline const char* vulkan_module = "vulkan-1.dll";
    HMODULE load_module(LPCSTR);
    Funct load_symbol(HMODULE, LPCSTR);
    void close_module(HMODULE);
#elif __linux__
    constexpr inline const char* vulkan_module = "libvulkan.so";
    void* load_module(const char*);
    Funct load_symbol(void* handle, const char* symbol);
    void close_module(void*);
#endif
    template <typename ...Args>
    [[nodiscard]] std::string format(const std::string& str, Args&& ...args) {
        /* Check if argument count and format specifiers match */ {
            u64 start = 0;
            u64 count = 0;

            while ((start = str.find("{}", start)) != std::string::npos) {
                start += 2;
                ++count;
            }

            if (count != sizeof...(args)) {
                throw std::runtime_error("Format specifiers and argument count mismatch");
            }
        }

        auto internal_fmt = [](const std::string& istr, std::stringstream& oss, size_t& start_idx, const auto& var) {
            size_t index = istr.find("{}", start_idx);

            if (index == std::string::npos) {
                return;
            }

            oss << istr.substr(start_idx, index - start_idx) << var;
            start_idx = index + 2;
        };

        std::stringstream oss{};
        size_t start_idx = 0;

        (internal_fmt(str, oss, start_idx, std::forward<Args>(args)), ...);

        return oss << str.substr(start_idx, str.length()), oss.str();
    }

    [[nodiscard]] std::string timestamp();
} // namespace tethys::util

#endif //TETHYS_UTIL_HPP
