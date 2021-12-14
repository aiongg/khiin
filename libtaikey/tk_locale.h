#pragma once

#include <iostream>
#include <mutex>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <boost/locale.hpp>

namespace TaiKey {

static std::once_flag locale_init_flag;

static void init_locale() {
    std::call_once(locale_init_flag, []() {
        std::cout << "called" << std::endl;
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
#endif
        boost::locale::generator gen;
        std::locale loc = gen("");
        std::locale::global(loc);
        std::wcout.imbue(loc);
    });
}

} // namespace TaiKey