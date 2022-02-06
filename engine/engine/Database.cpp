#include "Database.h"

#include <mutex>
#include <regex>
#include <thread>
#include <unordered_set>

#include "SQL.h"

namespace khiin::engine {
namespace {

void AllWordsByFreqImpl(SQLite::Database &db, std::vector<DictionaryRow> &output) {
    output.clear();
    output.reserve(17000);
    auto q = SQLite::Statement(db, "select * from dictionary order by chhan_id asc");
    while (q.executeStep()) {
        output.push_back(DictionaryRow{q.getColumn("id").getInt(), q.getColumn("chhan_id").getInt(),
                                       q.getColumn("input").getString(), q.getColumn("output").getString(),
                                       q.getColumn("weight").getInt(), q.getColumn("category").getInt(),
                                       q.getColumn("annotation").getString()});
    }
}

void LoadSyllablesImpl(SQLite::Database &db, std::vector<std::string> &syllables) {
    syllables.clear();
    syllables.reserve(1500);
    auto query = SQLite::Statement(db, SQL::SELECT_Syllables);

    while (query.executeStep()) {
        syllables.push_back(query.getColumn("syl").getString());
    }
}

DictionaryRow *HighestUnigramCountImpl(SQLite::Database &db, std::vector<DictionaryRow *> const &grams) {
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

DictionaryRow *HighestBigramCountImpl(SQLite::Database &db, std::string const &lgram,
                                      std::vector<DictionaryRow *> const &rgrams) {
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

    virtual void AllWordsByFreq(std::vector<DictionaryRow> &output) override {
        AllWordsByFreqImpl(*db_handle, output);
    }

    virtual void LoadSyllables(std::vector<std::string> &syllables) override {
        LoadSyllablesImpl(*db_handle, syllables);
    }

    virtual DictionaryRow *HighestUnigramCount(std::vector<DictionaryRow *> const &grams) override {
        return HighestUnigramCountImpl(*db_handle, grams);
    }

    virtual DictionaryRow *HighestBigramCount(std::string const &lgram,
                                              std::vector<DictionaryRow *> const &rgrams) override {
        return HighestBigramCountImpl(*db_handle, lgram, rgrams);
    }

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
    auto handle = new SQLite::Database(db_filename, SQLite::OPEN_READWRITE);
    return new DatabaseImpl(handle);
}

} // namespace khiin::engine
