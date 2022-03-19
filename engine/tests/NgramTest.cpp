#include "engine/CandidateFinder.h"
#include "engine/Database.h"

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
    auto result = CandidateFinder::ContinuousBestMatch(engine(), nullptr, "e5");
    EXPECT_EQ(result.Text(), "个");
    RecordUnigrams({"兮"});
    result = CandidateFinder::ContinuousBestMatch(engine(), nullptr, "e5");
    EXPECT_EQ(result.Text(), "兮");
}

} // namespace khiin::engine
