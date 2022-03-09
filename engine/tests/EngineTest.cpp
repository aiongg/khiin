#include <gtest/gtest.h>

#include "Engine.h"

#include <cstdlib>
#include <filesystem>
#include <vector>

#include <utf8cpp/utf8/cpp17.h>

#include "TestEnv.h"
#include "common.h"
#include "proto.h"

namespace khiin::engine {
namespace {

using namespace khiin::proto;
namespace fs = std::filesystem;

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
    auto str = kKhiinHome + "=" + resDir.string();
#ifdef _WIN32
    auto path = toPWSTR(str);
    auto ret = ::_wputenv(path.get());
#else
    const char *cpath = str.c_str();
    char *path = const_cast<char *>(cpath);
    auto ret = ::putenv(path);
#endif
}

class EngineTest : public ::testing::Test {
  protected:
    void SetUp() override {
        setTaikeyPath();
        engine = TestEnv::engine();
    }

    Engine *engine = nullptr;
    Command *command = nullptr;
    Request *req = nullptr;
    Response *res = nullptr;

    void feedText(const char *keys) {
        for (auto c : std::string(keys)) {
            command = new Command();
            req = command->mutable_request();
            res = command->mutable_response();

            req->set_type(CMD_SEND_KEY);
            req->mutable_key_event()->set_key_code(c);
            engine->SendCommand(command);
        }
    }
};

TEST_F(EngineTest, Loads) {
    EXPECT_TRUE(engine);
}

TEST_F(EngineTest, SimpleInput) {
    //feedText("ka");
    //EXPECT_GT(output->candidate_list().candidates().size(), 0);
}

TEST_F(EngineTest, PrimaryCandidate) {
    //feedText("goasiannmihlongboai");
    //EXPECT_GT(output->candidate_list().candidates().size(), 0);
    //EXPECT_EQ(output->candidate_list().candidates().at(0).value(), u8"我省乜朗無愛");
}

TEST_F(EngineTest, Erasing) {
    //feedText("a");
    // auto ret = e->onKeyDown(KeyCode::BACK, display);
    // BOOST_TEST((ret == RetVal::Cancelled));
    // BOOST_TEST(display.buffer == u8"");
    // ret = e->onKeyDown(KeyCode::BACK, display);
    // BOOST_TEST((ret == RetVal::NotConsumed));
    // e->onKeyDown(KeyCode::BACK, display);
    // BOOST_TEST(display.buffer == u8"");
    // ret = e->onKeyDown(key('a'), display);
    // BOOST_TEST((ret == RetVal::Consumed));
    // BOOST_TEST(display.buffer == u8"a");
}

} // namespace
} // namespace khiin::engine
