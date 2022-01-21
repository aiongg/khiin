#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include "buffer_manager.h"
#include "splitter.h"
#include "trie.h"

using namespace taikey;

namespace fs = std::filesystem;

struct SplitterFx {
    SplitterFx() {
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
    ~SplitterFx() { delete ss; }

    Splitter *ss = nullptr;
};

BOOST_FIXTURE_TEST_SUITE(WordlistTest, SplitterFx);

BOOST_AUTO_TEST_CASE(loads) { BOOST_TEST(true); }

BOOST_AUTO_TEST_CASE(split_sentence) {
    std::vector<std::string> res;
    ss->split("goamchaiujoachelanghamgoaukangkhoanesengtiong", res);
    std::string joined = boost::algorithm::join(res, " ");
    BOOST_TEST(joined ==
               "goa m chai u joa che lang ham goa u kang khoan e seng tiong");

    ss->split("goutuitiunnkinkukasiokthekiongechuliauchitesiauliankesisimchongb"
              "apihlaikoesineiesithekuibinlongsibaksaikapphinnkouchebengbengsit"
              "ikoesinchinchengsiutiohchintoaethongkhou",
              res);
    joined = boost::algorithm::join(res, " ");
    // BOOST_LOG_TRIVIAL(debug) << joined;
    BOOST_TEST(
        joined ==
        "gou tui tiunn kin ku ka siok the kiong e chu liau chit e siau lian ke "
        "si sim chong ba pih lai koe sin e i e si the kui bin long si bak sai "
        "kap phinn kou che beng beng si ti koe sin chin cheng siu tioh chin "
        "toa e thong khou");
}

BOOST_AUTO_TEST_CASE(split_sentence_fail) {
    std::vector<std::string> res;
    ss->split("goamchaiblarg", res);
    std::string joined = boost::algorithm::join(res, " ");
    BOOST_TEST(joined == "goa m chai b la r g");
}

BOOST_AUTO_TEST_CASE(split_sentence_digits) {
    std::vector<std::string> res;
    ss->split("goam7chai", res);
    std::string joined = boost::algorithm::join(res, " ");
    BOOST_TEST(joined == "goa m7 chai");

    ss->split("goa2mchaiu7joa7chelang5ham5goa2ukangkhoan2esengtiong", res);
    joined = boost::algorithm::join(res, " ");
    BOOST_TEST(
        joined ==
        "goa2 m chai u7 joa7 che lang5 ham5 goa2 u kang khoan2 e seng tiong");
}

BOOST_AUTO_TEST_CASE(test_can_split) {
    BOOST_TEST(ss->canSplit("liho"));
    BOOST_TEST(ss->canSplit("goamchaiujoachelanghamgoaukangkhoanesengtiong"));
    BOOST_TEST(ss->canSplit("li2ho2"));
    BOOST_TEST(!ss->canSplit("ppp"));
}

BOOST_AUTO_TEST_SUITE_END();
