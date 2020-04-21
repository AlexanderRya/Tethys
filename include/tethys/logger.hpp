#ifndef TETHYS_LOGGER_HPP
#define TETHYS_LOGGER_HPP

#include <tethys/util.hpp>

namespace tethys::logger {
    template <typename ...Args>
    void info(Args&& ...args) {
        util::print(util::format(
            "[{}] [Logger] [Info]: ",
            util::timestamp()));

        ((util::print(args), ...), util::print("\n"));
    }

    template <typename ...Args>
    void warning(Args&& ...args) {
        util::print(util::format(
            "[{}] [Logger] [Warning]: ",
            util::timestamp()));

        ((util::print(args), ...), util::print("\n"));
    }

    template <typename ...Args>
    void error(Args&& ...args) {
        util::print(util::format(
            "[{}] [Logger] [Error]: ",
            util::timestamp()));

        ((util::print(args), ...), util::print("\n"));
    }
} // namespace tethys::logger

#endif //TETHYS_LOGGER_HPP
