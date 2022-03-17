#include "Database.h"

#include <mutex>
#include <regex>
#include <thread>
#include <unordered_set>

#include "SQL.h"

namespace khiin::engine {
namespace {

class DatabaseImpl : public Database {
  public:
    DatabaseImpl(SQLite::Database *handle) {
        db_handle = std::unique_ptr<SQLite::Database>(handle);
    }

  private:
    virtual void AllWordsByFreq(std::vector<InputByFreq> &output) override {
        auto q = SQLite::Statement(*db_handle, SQL::SELECT_AllInputsByFreq);
        while (q.executeStep()) {
            output.push_back(InputByFreq{q.getColumn("id").getInt(), q.getColumn("input").getString()});
        }
    }

    virtual void LoadSyllables(std::vector<std::string> &syllables) override {
        syllables.clear();
        syllables.reserve(1500);
        auto query = SQLite::Statement(*db_handle, SQL::SELECT_Syllables);

        while (query.executeStep()) {
            syllables.push_back(query.getColumn("input").getString());
        }
    }

    virtual void ClearNGramsData() override {
        auto q1 = SQLite::Statement(*db_handle, R"(DELETE FROM "unigram_freq")");
        q1.exec();
        auto q2 = SQLite::Statement(*db_handle, R"(DELETE FROM "bigram_freq")");
        q2.exec();
    }

    virtual void RecordUnigrams(std::vector<std::string> const &grams) override {
        if (grams.empty()) {
            return;
        }
        auto size = grams.size();
        auto query = SQLite::Statement(*db_handle, SQL::UPSERT_Unigrams(size));
        auto i = 1;
        for (auto const &gram : grams) {
            query.bind(i, gram);
            query.bind(i + size, gram);
            ++i;
        }
        query.exec();
    }

    virtual void RecordBigrams(std::vector<Bigram> const &grams) override {
        if (grams.empty()) {
            return;
        }

        auto size = grams.size();
        auto query = SQLite::Statement(*db_handle, SQL::UPSERT_Bigrams(size));
        auto i = 1;
        for (auto const &gram : grams) {
            query.bind(i, gram.first);
            query.bind(i + size * 2, gram.first);
            ++i;
            query.bind(i, gram.second);
            query.bind(i + size * 2, gram.second);
            ++i;
        }
        query.exec();        
    }

    virtual int UnigramCount(std::string const& gram) override {
        auto query = SQLite::Statement(*db_handle, R"(SELECT "n" FROM "unigram_freq" WHERE "gram" = ?)");
        query.bind(1, gram);

        if (query.executeStep()) {
            return query.getColumn("n").getInt();
        }

        return 0;
    }

    virtual int BigramCount(Bigram const& gram) override {
        auto query = SQLite::Statement(*db_handle, R"(SELECT "n" FROM "bigram_freq" WHERE "lgram" = ? AND "rgram" = ?)");
        query.bind(1, gram.first);
        query.bind(2, gram.second);

        if (query.executeStep()) {
            return query.getColumn("n").getInt();
        }

        return 0;
    }

    virtual TaiToken *HighestUnigramCount(std::vector<TaiToken *> const &grams) override {
        if (grams.empty()) {
            return nullptr;
        }

        auto query = SQLite::Statement(*db_handle, SQL::SELECT_BestUnigram(grams.size()));
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

    virtual TaiToken *HighestBigramCount(std::string const &lgram, std::vector<TaiToken *> const &rgrams) override {
        if (lgram.empty() || rgrams.empty()) {
            return nullptr;
        }

        auto query = SQLite::Statement(*db_handle, SQL::SELECT_BestBigram(rgrams.size()));
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

    virtual void ConversionsByInputId(int input_id, std::vector<TaiToken> &conversions) override {
        auto q = SQLite::Statement(*db_handle, SQL::SELECT_ConversionsByInputId_One);
        q.bind(1, input_id);

        while (q.executeStep()) {
            auto token = TaiToken();
            token.id = q.getColumn(SQL::conv_id).getInt();
            token.input_id = input_id;
            token.input = q.getColumn(SQL::freq_input).getString();
            token.output = q.getColumn(SQL::conv_output).getString();
            token.annotation = q.getColumn(SQL::conv_annotation).getString();
            token.category = q.getColumn(SQL::conv_category).getInt();
            token.weight = q.getColumn(SQL::conv_weight).getInt();
            conversions.push_back(std::move(token));
        }
    }

    virtual void LoadPunctuation(std::vector<Punctuation> &output) override {
        auto query = SQLite::Statement(*db_handle, "SELECT * FROM symbols");
        while (query.executeStep()) {
            output.push_back(Punctuation{query.getColumn("id").getInt(), query.getColumn("input").getString(),
                                         query.getColumn("output").getString(),
                                         query.getColumn("annotation").getString()});
        }
    }

    virtual std::vector<Emoji> GetEmojis() override {
        auto ret = std::vector<Emoji>();
        auto query = SQLite::Statement(*db_handle, "SELECT * FROM emojis ORDER BY category ASC, id ASC");
        while (query.executeStep()) {
            ret.push_back(Emoji{query.getColumn("category").getInt(), query.getColumn("emoji").getString()});
        }
        return ret;
    }

    std::unique_ptr<SQLite::Database> db_handle;
};

} // namespace

Database *Database::TestDb() {
    auto handle = new SQLite::Database(":memory:", SQLite::OPEN_READWRITE);
    handle->exec(SQL::CREATE_DummyDatabase());
    return new DatabaseImpl(handle);
}

Database *Database::Connect(std::string const &db_filename) {
    try {
        auto handle = new SQLite::Database(db_filename, SQLite::OPEN_READWRITE);
        return new DatabaseImpl(handle);
    } catch (...) {
        return TestDb();
    }
}

} // namespace khiin::engine
