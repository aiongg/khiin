#include "data/Database.h"

#include <mutex>
#include <regex>
#include <thread>
#include <unordered_set>

#include "SQL.h"

namespace khiin::engine {
namespace {
using namespace db_tables;

constexpr size_t kReservedSyllables = 1500;

class DatabaseImpl : public Database {
  public:
    DatabaseImpl(std::unique_ptr<SQLite::Database> &&handle) : db_handle(std::move(handle)) {}

  private:
    void AllWordsByFreq(std::vector<InputByFreq> &output) override {
        auto query = SQL::SelectInputsByFreq(*db_handle);
        while (query.executeStep()) {
            auto tmp = InputByFreq();
            tmp.id = query.getColumn(frequencies::id).getInt();
            tmp.input = query.getColumn(frequencies::input).getString();
            output.push_back(std::move(tmp));
        }
    }

    void LoadSyllables(std::vector<std::string> &syllables) override {
        syllables.clear();
        syllables.reserve(kReservedSyllables);
        auto query = SQL::SelectSyllables(*db_handle);
        while (query.executeStep()) {
            syllables.push_back(query.getColumn(syllables::input).getString());
        }
    }

    void ClearNGramsData() override {
        SQL::DeleteBigrams(*db_handle).exec();
        SQL::DeleteUnigrams(*db_handle).exec();
    }

    void RecordUnigrams(std::vector<std::string> const &grams) override {
        if (grams.empty()) {
            return;
        }

        SQL::IncrementUnigrams(*db_handle, grams).exec();
    }

    void RecordBigrams(std::vector<Bigram> const &grams) override {
        if (grams.empty()) {
            return;
        }

        SQL::IncrementBigrams(*db_handle, grams).exec();
    }

    int UnigramCount(std::string const &gram) override {
        auto ret = 0;

        if (auto query = SQL::SelectUnigramCount(*db_handle, gram); query.executeStep()) {
            ret = query.getColumn(unigram_freq::count).getInt();
        }

        return ret;
    }

    int BigramCount(Bigram const &gram) override {
        auto ret = 0;

        if (auto query = SQL::SelectBigramCount(*db_handle, gram.first, gram.second); query.executeStep()) {
            ret = query.getColumn(bigram_freq::count).getInt();
        }

        return ret;
    }

    std::vector<Gram> UnigramCounts(std::vector<std::string> const &grams) override {
        auto ret = std::vector<Gram>();
        auto query = SQL::SelectUnigramCounts(*db_handle, grams);

        while (query.executeStep()) {
            Gram gram{};
            gram.value = query.getColumn(unigram_freq::gram).getString();
            gram.count = query.getColumn(unigram_freq::count).getInt();
            ret.push_back(std::move(gram));
        }

        return ret;
    }

    std::vector<Gram> BigramCounts(std::string const &lgram, std::vector<std::string> const &rgrams) override {
        auto ret = std::vector<Gram>();
        auto query = SQL::SelectBigramCounts(*db_handle, lgram, rgrams);

        while (query.executeStep()) {
            Gram gram{};
            gram.value = query.getColumn(bigram_freq::rgram).getString();
            gram.count = query.getColumn(bigram_freq::count).getInt();
            ret.push_back(std::move(gram));
        }

        return ret;
    }

    TaiToken *HighestUnigramCount(std::vector<TaiToken *> const &tokens) override {
        TaiToken *ret = nullptr;

        if (tokens.empty()) {
            return ret;
        }

        auto grams = std::vector<std::string *>();
        for (auto const &token : tokens) {
            grams.push_back(&token->output);
        }

        auto query = SQL::SelectBestUnigram(*db_handle, grams);

        if (query.executeStep()) {
            auto result = query.getColumn(unigram_freq::gram).getString();
            for (auto const &token : tokens) {
                if (result == token->output) {
                    ret = token;
                    break;
                }
            }
        }

        return ret;
    }

    TaiToken *HighestBigramCount(std::string const &lgram, std::vector<TaiToken *> const &rgram_tokens) override {
        TaiToken *ret = nullptr;

        if (lgram.empty() || rgram_tokens.empty()) {
            return ret;
        }

        auto rgrams = std::vector<std::string *>();
        for (auto const &token : rgram_tokens) {
            rgrams.push_back(&token->output);
        }

        auto query = SQL::SelectBestBigram(*db_handle, lgram, rgrams);
        if (query.executeStep()) {
            auto result = query.getColumn(bigram_freq::rgram).getString();
            for (auto const &token : rgram_tokens) {
                if (result == token->output) {
                    ret = token;
                    break;
                }
            }
        }

        return ret;
    }

    void ConversionsByInputId(int input_id, std::vector<TaiToken> &conversions) override {
        auto query = SQL::SelectConversions(*db_handle, input_id);

        while (query.executeStep()) {
            auto token = TaiToken();
            token.id = query.getColumn(conversions::id).getInt();
            token.input_id = input_id;
            token.input = query.getColumn(frequencies::input).getString();
            token.output = query.getColumn(conversions::output).getString();
            token.annotation = query.getColumn(conversions::annotation).getString();
            token.category = query.getColumn(conversions::category).getInt();
            token.weight = query.getColumn(conversions::weight).getInt();
            conversions.push_back(std::move(token));
        }
    }

    void LoadPunctuation(std::vector<Punctuation> &output) override {
        auto query = SQL::SelectSymbols(*db_handle);
        while (query.executeStep()) {
            auto tmp = Punctuation();
            tmp.id = query.getColumn(symbols::id).getInt();
            tmp.input = query.getColumn(symbols::input).getString();
            tmp.output = query.getColumn(symbols::output).getString();
            tmp.annotation = query.getColumn(symbols::annotation).getString();
            output.push_back(std::move(tmp));
        }
    }

    std::vector<Emoji> GetEmojis() override {
        auto ret = std::vector<Emoji>();
        auto query = SQL::SelectEmojis(*db_handle);
        while (query.executeStep()) {
            auto tmp = Emoji();
            tmp.category = query.getColumn(emojis::category).getInt();
            tmp.value = query.getColumn(emojis::emoji).getString();
            ret.push_back(std::move(tmp));
        }
        return ret;
    }

    std::unique_ptr<SQLite::Database> db_handle;
};

} // namespace

Database::~Database() = default;

std::unique_ptr<Database> Database::TestDb() {
    auto handle = std::make_unique<SQLite::Database>(":memory:", SQLite::OPEN_READWRITE);
    SQL::CreateDummyDb(*handle).exec();
    return std::make_unique<DatabaseImpl>(std::move(handle));
}

std::unique_ptr<Database> Database::Connect(std::string const &db_filename) {
    try {
        auto handle = std::make_unique<SQLite::Database>(db_filename, SQLite::OPEN_READWRITE);
        return std::make_unique<DatabaseImpl>(std::move(handle));
    } catch (...) {
        return TestDb();
    }
}

} // namespace khiin::engine
