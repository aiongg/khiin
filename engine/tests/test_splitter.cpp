#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include "BufferManager.h"
#include "Splitter.h"
#include "Trie.h"

namespace khiin::engine {
namespace {

namespace fs = std::filesystem;

class SplitterFx : public ::testing::Test {
  protected:
    void SetUp() override {
        std::vector<std::string> syls;

        if (!fs::exists("syllables.dat")) {
            ss = new Splitter(syls);
            return;
        }

        std::ifstream fileHandler("syllables.dat");
        std::string syl;

        while (std::getline(fileHandler, syl)) {
            syls.push_back(syl);
        }

        ss = new Splitter(syls);
    }
    ~SplitterFx() {
        delete ss;
    }

    Splitter *ss = nullptr;
};

TEST_F(SplitterFx, loads) {
    EXPECT_TRUE(true);
}

TEST_F(SplitterFx, split_sentence) {
    std::vector<std::string> res;
    ss->split("goamchaiujoachelanghamgoaukangkhoanesengtiong", res);
    std::string joined = boost::algorithm::join(res, " ");
    EXPECT_EQ(joined, "goa m chai u joa che lang ham goa u kang khoan e seng tiong");

    ss->split("goutuitiunnkinkukasiokthekiongechuliauchitesiauliankesisimchongb"
              "apihlaikoesineiesithekuibinlongsibaksaikapphinnkouchebengbengsit"
              "ikoesinchinchengsiutiohchintoaethongkhou",
              res);
    joined = boost::algorithm::join(res, " ");
    // BOOST_LOG_TRIVIAL(debug) << joined;
    EXPECT_EQ(joined, "gou tui tiunn kin ku ka siok the kiong e chu liau chit e siau lian ke "
                      "si sim chong ba pih lai koe sin e i e si the kui bin long si bak sai "
                      "kap phinn kou che beng beng si ti koe sin chin cheng siu tioh chin "
                      "toa e thong khou");
}

TEST_F(SplitterFx, split_sentence_fail) {
    std::vector<std::string> res;
    ss->split("goamchaiblarg", res);
    std::string joined = boost::algorithm::join(res, " ");
    EXPECT_EQ(joined, "goa m chai b la r g");
}

TEST_F(SplitterFx, split_sentence_digits) {
    std::vector<std::string> res;
    ss->split("goam7chai", res);
    std::string joined = boost::algorithm::join(res, " ");
    EXPECT_EQ(joined, "goa m7 chai");

    ss->split("goa2mchaiu7joa7chelang5ham5goa2ukangkhoan2esengtiong", res);
    joined = boost::algorithm::join(res, " ");
    EXPECT_EQ(joined, "goa2 m chai u7 joa7 che lang5 ham5 goa2 u kang khoan2 e seng tiong");
}

TEST_F(SplitterFx, test_can_split) {
    EXPECT_TRUE(ss->canSplit("liho"));
    EXPECT_TRUE(ss->canSplit("goamchaiujoachelanghamgoaukangkhoanesengtiong"));
    EXPECT_TRUE(ss->canSplit("li2ho2"));
    EXPECT_TRUE(!ss->canSplit("ppp"));
}

} // namespace
} // namespace khiin::engine