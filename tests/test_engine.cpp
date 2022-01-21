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

namespace taikey {

namespace fs = std::filesystem;

typedef std::pair<std::string, std::string> test_word;

static auto key(char c) { return static_cast<KeyCode>(c); }

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
    BOOST_TEST(display.buffer == u8"ka");
    BOOST_TEST(display.candidates.size() > 10);
}

BOOST_AUTO_TEST_CASE(t03_primary_candidate) {
    feedText("goasiannmihlongboai");
    BOOST_TEST(display.candidates.size() > 0);
    BOOST_TEST(display.candidates[0].text == u8"我省乜朗無愛");
}

BOOST_AUTO_TEST_CASE(t04_erasing) {
    feedText("a");
    auto ret = e->onKeyDown(KeyCode::BACK, display);
    BOOST_TEST((ret == RetVal::Cancelled));
    BOOST_TEST(display.buffer == u8"");
    ret = e->onKeyDown(KeyCode::BACK, display);
    BOOST_TEST((ret == RetVal::NotConsumed));
    e->onKeyDown(KeyCode::BACK, display);
    BOOST_TEST(display.buffer == u8"");
    ret = e->onKeyDown(key('a'), display);
    BOOST_TEST((ret == RetVal::Consumed));
    BOOST_TEST(display.buffer == u8"a");
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace TaiKey
