#include "Database.h"

#include <mutex>
#include <regex>
#include <thread>
#include <unordered_set>

#include "SQL.h"

namespace khiin::engine {
namespace {

void AllWordsByFreqImpl(SQLite::Database &db, std::vector<InputByFreq> &output) {
    auto q = SQLite::Statement(db, SQL::SELECT_AllInputsByFreq);
    while (q.executeStep()) {
        output.push_back(InputByFreq{q.getColumn("id").getInt(), q.getColumn("input").getString()});
    }
}

void ConversionsByInputImpl(SQLite::Database &db, int input_id,
                            std::vector<TaiToken> &conversions) {

    auto q = SQLite::Statement(db, SQL::SELECT_ConversionsByInputId_One);
    q.bind(1, input_id);

    while (q.executeStep()) {
        auto token = TaiToken();
        token.id = q.getColumn(SQL::conv_id).getInt();
        token.input = q.getColumn(SQL::freq_input).getString();
        token.output = q.getColumn(SQL::conv_output).getString();
        token.annotation = q.getColumn(SQL::conv_annotation).getString();
        token.category = q.getColumn(SQL::conv_category).getInt();
        conversions.push_back(std::move(token));
    }
}

void LoadSyllablesImpl(SQLite::Database &db, std::vector<std::string> &syllables) {
    syllables.clear();
    syllables.reserve(1500);
    auto query = SQLite::Statement(db, SQL::SELECT_Syllables);

    while (query.executeStep()) {
        syllables.push_back(query.getColumn("input").getString());
    }
}

TaiToken *HighestUnigramCountImpl(SQLite::Database &db, std::vector<TaiToken *> const &grams) {
    if (grams.empty()) {
        return nullptr;
    }

    auto query = SQLite::Statement(db, SQL::SELECT_BestUnigram(grams.size()));
    auto i = 1;
    for (auto const &gram : grams) {
        query.bind(i, gram->output);
        ++i;
    }

    auto result = std::string();
    while (query.executeStep()) {
        result = query.getColumn("gram").getString();
        break;
    }

    for (auto const &gram : grams) {
        if (result == gram->output) {
            return gram;
        }
    }

    return nullptr;
}

TaiToken *HighestBigramCountImpl(SQLite::Database &db, std::string const &lgram,
                                 std::vector<TaiToken *> const &rgrams) {
    if (lgram.empty() || rgrams.empty()) {
        return nullptr;
    }

    auto query = SQLite::Statement(db, SQL::SELECT_BestBigram(rgrams.size()));
    query.bind(1, lgram);
    auto i = 2;
    for (auto const &gram : rgrams) {
        query.bind(i, gram->output);
        ++i;
    }

    auto result = std::string();
    while (query.executeStep()) {
        result = query.getColumn("rgram").getString();
        break;
    }

    for (auto const &gram : rgrams) {
        if (result == gram->output) {
            return gram;
        }
    }

    return nullptr;
}

class DatabaseImpl : public Database {
  public:
    DatabaseImpl(SQLite::Database *handle) {
        db_handle = std::unique_ptr<SQLite::Database>(handle);
    }

    virtual void AllWordsByFreq(std::vector<InputByFreq> &output) override {
        AllWordsByFreqImpl(*db_handle, output);
    }

    virtual void LoadSyllables(std::vector<std::string> &syllables) override {
        LoadSyllablesImpl(*db_handle, syllables);
    }

    virtual TaiToken *HighestUnigramCount(std::vector<TaiToken *> const &grams) override {
        return HighestUnigramCountImpl(*db_handle, grams);
    }

    virtual TaiToken *HighestBigramCount(std::string const &lgram, std::vector<TaiToken *> const &rgrams) override {
        return HighestBigramCountImpl(*db_handle, lgram, rgrams);
    }

    // virtual void ConversionsByInputId(std::vector<int> const &input_ids, std::vector<TaiToken> &conversions) override
    // {
    //    ConversionsByInputIdImpl(*db_handle, input_ids, conversions);
    //}

    virtual void ConversionsByInputId(int input_id, std::vector<TaiToken> &output) override {
        ConversionsByInputImpl(*db_handle, input_id, output);
    }

    //virtual void ConversionsByInput(std::vector<std::string> const &inputs, std::vector<TaiToken> &output) override {
    //    ConversionsByInputImpl(*db_handle, inputs, output);
    //}

  private:
    std::unique_ptr<SQLite::Database> db_handle;
};

} // namespace

Database *Database::TestDb() {
    auto handle = new SQLite::Database(":memory:", SQLite::OPEN_READWRITE);
    handle->exec(SQL::CREATE_DummyDatabase());
    return new DatabaseImpl(handle);
}

Database *Database::Connect(std::string db_filename) {
    try {
        auto handle = new SQLite::Database(db_filename, SQLite::OPEN_READWRITE);
        return new DatabaseImpl(handle);
    } catch (...) {
        return TestDb();
    }
}

} // namespace khiin::engine
