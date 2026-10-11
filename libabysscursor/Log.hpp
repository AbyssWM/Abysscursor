#pragma once

#include <string>
#include <format>
#include <iostream>

#include <abysscursor/shared.h>

namespace Debug {
    inline bool quiet   = false;
    inline bool verbose = false;

    template <typename... Args>
    void log(eAbysscursorLogLevel level, PABYSSCURSORLOGFUNC fn, const std::string& fmt, Args&&... args) {
        if (!fn)
            return;

        const std::string LOG = std::vformat(fmt, std::make_format_args(args...));

        fn(level, (char*)LOG.c_str());
    }
};