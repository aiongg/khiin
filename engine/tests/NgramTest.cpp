#include "input/CandidateFinder.h"
#include "data/Database.h"

#include "TestEnv.h"

namespace khiin::engine {

struct NgramTest : ::testing::Test, TestEnv {
  protected:
    void TearDown() override {
        engine()->database()->ClearNGramsData();
    }

    void RecordUnigrams(std::vector<std::string> grams) {
        engine()->database()->RecordUnigrams(grams);
    }
};

TEST_F(NgramTest, TestUnigram) {
    auto result = CandidateFinder::ContinuousSingleMatch(engine(), nullptr, "e5");
    EXPECT_EQ(result.Text(), "个");
    RecordUnigrams({"兮"});
    result = CandidateFinder::ContinuousSingleMatch(engine(), nullptr, "e5");
    EXPECT_EQ(result.Text(), "兮");
}

TEST_F(NgramTest, TestUnigramSort) {
    auto result = CandidateFinder::MultiMatch(engine(), nullptr, "e5");
    EXPECT_EQ(result[0].Text(), "个");
    EXPECT_EQ(result[1].Text(), "兮");
    EXPECT_EQ(result[2].Text(), "鞋");
    RecordUnigrams({"鞋"});
    result = CandidateFinder::MultiMatch(engine(), nullptr, "e5");
    EXPECT_EQ(result[0].Text(), "鞋");
    EXPECT_EQ(result[1].Text(), "个");
    EXPECT_EQ(result[2].Text(), "兮");
}

} // namespace khiin::engine
