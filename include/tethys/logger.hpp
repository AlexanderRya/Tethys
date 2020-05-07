#ifndef TETHYS_LOGGER_HPP
#define TETHYS_LOGGER_HPP

#include <tethys/util.hpp>

namespace tethys::logger {
    template <typename ...Args>
    void info(const std::string& format, Args&& ...args) {
        std::printf("%s\n",
            util::format(
                "[{}] [Logger] [Info]: " + format,
                util::timestamp(), args...).c_str());
    }

    template <typename ...Args>
    void warning(const std::string& format, Args&& ...args) {
        std::printf("%s\n",
            util::format(
                "[{}] [Logger] [Warning]: " + format,
                util::timestamp(), args...).c_str());
    }

    template <typename ...Args>
    void error(const std::string& format, Args&& ...args) {
        std::printf("%s\n",
            util::format(
                "[{}] [Logger] [Error]: " + format,
                util::timestamp(), args...).c_str());
    }
} // namespace tethys::logger

#endif //TETHYS_LOGGER_HPP
