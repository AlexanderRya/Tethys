#ifndef TETHYS_UTIL_HPP
#define TETHYS_UTIL_HPP

#include <tethys/Types.hpp>

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
#if _WIN32
    constexpr inline const char* vulkan_module = "vulkan-1.dll";
    HMODULE load_module(LPCSTR);
    auto load_symbol(HMODULE, LPCSTR) -> void(*)();
    void close_module(HMODULE);
#elif __linux__
    constexpr inline const char* vulkan_module = "libvulkan.so";
    void* load_module(const char*);
    auto load_symbol(void* handle, const char* symbol) -> void(*)();
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
    void print(const std::string&);
    void print(const char*);
    void print(const void* addr);

#if 1 // TODO: pls release C++20
    template <typename Ty, std::enable_if_t<std::is_arithmetic_v<Ty>>* = nullptr>
#else
    template <typename Ty>
    concept is_stringable = requires(Ty x ) {
        { std::to_string(x) };
    };
    template <typename Ty>
        requires is_stringable<Ty>
#endif
    void print(const Ty val) {
        print(std::to_string(val));
    }
} // namespace tethys::util

#endif //TETHYS_UTIL_HPP
