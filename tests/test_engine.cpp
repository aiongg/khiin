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
        words.push_back(test_word(u8"a", u8"a"));
        words.push_back(test_word(u8"as", u8"á"));
        words.push_back(test_word(u8"af", u8"à"));
        words.push_back(test_word(u8"uif", u8"ùi"));
        words.push_back(test_word(u8"uis", u8"úi"));
        words.push_back(test_word(u8"iuj", u8"iū"));
        words.push_back(test_word(u8"boal", u8"bôa"));
        words.push_back(test_word(u8"oanl", u8"oân"));
        words.push_back(test_word(u8"aw", u8"ă"));
        words.push_back(test_word(u8"khoannf", u8"khòaⁿ"));
    }
    ~EngineFx() { delete e; }
    Engine *e = nullptr;

    void feedKeys(const char *keys) {
        e->reset();
        for (auto c : std::string(keys)) {
            e->onKeyDown(c);
        }
    }

    std::string getBuffer() { return e->getBuffer(); }
    void reset() { e->reset(); }

    std::vector<test_word> words;
};

BOOST_FIXTURE_TEST_SUITE(TestEngine, EngineFx);

BOOST_AUTO_TEST_CASE(t01_load) { BOOST_CHECK_EQUAL(u8"", getBuffer()); }

BOOST_AUTO_TEST_CASE(Engine_Failure, *boost::unit_test::expected_failures(1)) {
    feedKeys(u8"a");
    BOOST_CHECK_EQUAL(u8"b", e->getBuffer());
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace TaiKey
