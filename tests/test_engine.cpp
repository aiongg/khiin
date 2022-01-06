// libtaikey_test.cpp : Source file for your target.
//

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/log/trivial.hpp>

#include <cstdlib>
#include <filesystem>
#include <vector>

#include <utf8cpp/utf8.h>
#include <utf8cpp/utf8/cpp17.h>

#include "engine.h"

namespace TaiKey {

namespace fs = std::filesystem;

typedef std::pair<std::string, std::string> test_word;

#ifdef _WIN32
auto toPWSTR(std::string str) {
    auto wstr = utf8::utf8to16(str);
    std::unique_ptr<wchar_t[]> pwstr(new wchar_t[wstr.size() + 1]);
    std::copy(wstr.cbegin(), wstr.cend(), pwstr.get());
    pwstr[wstr.size()] = '\0';
    return std::move(pwstr);
}
#endif

auto setTaikeyPath() {
    auto resDir = fs::current_path();
    auto str = TaiKeyPath + "=" + resDir.string();
#ifdef _WIN32
    auto path = toPWSTR(str);
    auto ret = ::_wputenv(path.get());
#else
    const char *cpath = str.c_str();
    char *path = const_cast<char *>(cpath);
    auto ret = ::putenv(path);
#endif
}

struct EngineFx {
    EngineFx() {
        setTaikeyPath();
        e = new Engine();
    }
    ~EngineFx() { delete e; }
    Engine *e = nullptr;

    void feedText(const char *keys) {
        for (auto c : std::string(keys)) {
            e->onKeyDown(static_cast<KeyCode>(c), display);
        }
    }

    ImeDisplayData display;
};

BOOST_FIXTURE_TEST_SUITE(TestEngine, EngineFx);

BOOST_AUTO_TEST_CASE(t01_load) { BOOST_CHECK_EQUAL(u8"", display.buffer); }

BOOST_AUTO_TEST_CASE(t02_simple) {
    feedText("ka");
    BOOST_CHECK_EQUAL(u8"ka", display.buffer);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace TaiKey
