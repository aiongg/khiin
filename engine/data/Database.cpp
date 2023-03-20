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
    explicit DatabaseImpl(std::unique_ptr<SQLite::Database> &&handle) : db_handle(std::move(handle)) {}

  private:
    std::string CurrentConnection() override {
        if (db_handle) {
            return db_handle->getFilename();
        }
        return std::string();
    }

    void AllWordsByFreq(std::vector<std::string>& output, InputType inputType) override {
        auto query = SQL::SelectAllKeySequences(*db_handle, inputType);
        while (query.executeStep()) {
            output.push_back(query.getColumn("key_sequence").getString());
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

    void AddUnigramData(std::vector<std::string *> const &grams, std::vector<TaiToken> &tokens) {
        auto query = SQL::SelectUnigrams(*db_handle, grams);

        while (query.executeStep()) {
            auto gram = query.getColumn(unigram_freq::gram).getString();
            auto found = std::find_if(tokens.begin(), tokens.end(), [&gram](TaiToken const &token) {
                return gram == token.output;
            });
            if (found != tokens.end()) {
                found->unigram_count = query.getColumn(unigram_freq::count).getInt();
            }
        }
    }

    void AddBigramData(std::string const &lgram, std::vector<std::string *> const &rgrams,
                       std::vector<TaiToken> &tokens) {
        auto query = SQL::SelectBigrams(*db_handle, lgram, rgrams);
        while (query.executeStep()) {
            auto gram = query.getColumn(bigram_freq::rgram).getString();
            auto found = std::find_if(tokens.begin(), tokens.end(), [&gram](TaiToken const &token) {
                return gram == token.output;
            });
            if (found != tokens.end()) {
                found->bigram_count = query.getColumn(bigram_freq::count).getInt();
            }
        }
    }

    void AddNGramsData(std::optional<std::string> const &lgram, std::vector<TaiToken> &tokens) override {
        if (tokens.empty()) {
            return;
        }

        auto rgrams = std::vector<std::string *>{};
        for (auto it = tokens.begin(); it != tokens.end(); ++it) {
            rgrams.push_back(&it->output);
        }

        AddUnigramData(rgrams, tokens);
        if (lgram) {
            AddBigramData(lgram.value(), rgrams, tokens);
        }
    }

    void LoadConversions(std::vector<std::string> &inputs, InputType inputType,
                         std::vector<TaiToken> &outputs) override {
        auto query = SQL::SelectConversions(*db_handle, inputs, inputType);

        while (query.executeStep()) {
            auto token = TaiToken();
            token.input_id = query.getColumn(conversions::input_id).getInt();
            token.input = query.getColumn(frequencies::input).getString();
            token.output = query.getColumn(conversions::output).getString();
            token.annotation = query.getColumn(conversions::annotation).getString();
            token.category = query.getColumn(conversions::category).getInt();
            token.weight = query.getColumn(conversions::weight).getInt();
            outputs.emplace_back(token);
        }
    }

    //void ConversionsByInputId(int input_id, std::vector<TaiToken> &conversions) override {
    //    auto query = SQL::SelectConversions(*db_handle, input_id);

    //    while (query.executeStep()) {
    //        auto token = TaiToken();
    //        token.id = query.getColumn(conversions::id).getInt();
    //        token.input_id = input_id;
    //        token.input = query.getColumn(frequencies::input).getString();
    //        token.output = query.getColumn(conversions::output).getString();
    //        token.annotation = query.getColumn(conversions::annotation).getString();
    //        token.category = query.getColumn(conversions::category).getInt();
    //        token.weight = query.getColumn(conversions::weight).getInt();
    //        conversions.push_back(std::move(token));
    //    }
    //}

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
    auto query = SQL::CreateDummyDb(*handle);
    // query.exec();
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
