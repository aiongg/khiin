#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Dictionary.h"
#include "Splitter.h"
#include "TestEnv.h"

namespace khiin::engine {
namespace {

std::string join(std::vector<std::string> &parts, std::string delim) {
    auto ret = std::string();
    auto it = parts.cbegin();
    while (it != parts.cend() - 1) {
        ret += *it;
        ret += delim;
        ++it;
    }
    ret += parts.back();
    return ret;
}

static std::vector<std::string> words = //
    {"goa2", "goa", "m7chai",     "mchai",     "joache",    "joa7che7", "lang5", "lang", "ham5", "ham",
     "u7",   "u",   "kangkhoan2", "kangkhoan", "e",         "seng",     "tiong", "li2",  "li",   "ho2",
     "ho",   "la",  "toa7",       "toa",       "toa7lang5", "toalang",  "to",    "a",    "ng"};

TEST(SplitterTest, SplitSentence) {
    auto splitter = Splitter(words);

    std::vector<std::string> res;
    splitter.Split("goamchaiujoachelanghamgoaukangkhoanesengtiong", res);
    std::string joined = join(res, " ");
    EXPECT_EQ(joined, "goa mchai u joache lang ham goa u kangkhoan e seng tiong");
}

TEST(SplitterTest, split_sentence_fail) {
    auto splitter = Splitter(words);

    std::vector<std::string> res;
    splitter.Split("goamchaiblarg", res);
    std::string joined = join(res, " ");
    EXPECT_EQ(joined, "goa mchai b la r g");
}

TEST(SplitterTest, split_sentence_digits) {
    auto splitter = Splitter(words);
    std::vector<std::string> res;
    splitter.Split("goam7chai", res);
    std::string joined = join(res, " ");
    EXPECT_EQ(joined, "goa m7chai");
}

TEST(SplitterTest, SplitSentenceDigitsLong) {
    auto splitter = Splitter(words);
    std::vector<std::string> res;
    splitter.Split("goa2mchaiu7joa7che7lang5ham5goa2ukangkhoan2esengtiong", res);
    auto joined = join(res, " ");
    EXPECT_EQ(joined, "goa2 mchai u7 joa7che7 lang5 ham5 goa2 u kangkhoan2 e seng tiong");
}

TEST(SplitterTest, CanSplitTest) {
    auto splitter = Splitter(words);
    EXPECT_TRUE(splitter.CanSplit("liho"));
    EXPECT_TRUE(splitter.CanSplit("goamchaiujoachelanghamgoaukangkhoanesengtiong"));
    EXPECT_TRUE(splitter.CanSplit("li2ho2"));
    EXPECT_FALSE(splitter.CanSplit("ppp"));
    EXPECT_FALSE(splitter.CanSplit("q"));
    EXPECT_FALSE(splitter.CanSplit("h"));
    EXPECT_FALSE(splitter.CanSplit("3"));
    EXPECT_FALSE(splitter.CanSplit("."));
    EXPECT_FALSE(splitter.CanSplit("-"));
}

TEST(SplitterTest, MaxSplitSize) {
    auto splitter = Splitter(words);
    EXPECT_EQ(splitter.MaxSplitSize(""), 0);
    EXPECT_EQ(splitter.MaxSplitSize("liho"), 4);
    EXPECT_EQ(splitter.MaxSplitSize("lihox"), 4);
    EXPECT_EQ(splitter.MaxSplitSize("lihoxyzasdf"), 4);
    EXPECT_EQ(splitter.MaxSplitSize("lihxyz"), 2);
    EXPECT_EQ(splitter.MaxSplitSize("mchaijoachelang"), 15);
    EXPECT_EQ(splitter.MaxSplitSize("xyz"), 0);
}

TEST(SplitterTest, SplitIndices) {
    auto splitter = Splitter(words);
    EXPECT_EQ(splitter.MaxSplitSize("toalang"), 7);
}

TEST(SplitterTest, DISABLED_UsingActualEngine) {
    auto splitter = TestEnv::engine()->dictionary()->word_splitter();
    std::vector<std::string> res;

    splitter->Split("goutuitiunnkinkukasiokthekiongechuliauchitesiauliankesisimchongb"
                    "apihlaikoesineiesithekuibinlongsibaksaikapphinnkouchebengbengsit"
                    "ikoesinchinchengsiutiohchintoaethongkhou",
                    res);
    std::string joined = join(res, " ");
    EXPECT_EQ(joined,
              "gou tui tiunn kin ku ka siok thekiong e chu liau chite siaulian kesi sim chong ba pih lai koe "
              "sin e i e sithe kui bin longsi baksai kap phinn kou che beng beng si ti koe sin chincheng siu tioh chin "
              "toae thong khou");
}

} // namespace
} // namespace khiin::engine