#include "SQL.h"

#include "utils/utils.h"

namespace khiin::engine {
namespace {
using namespace SQLite;
using namespace khiin::engine::utils;

std::string sql_binder(size_t n, std::string sep) {
    std::string res;

    if (n == 0) {
        return res;
    }

    res += sep;

    for (size_t i = 0; i < n - 1; i++) {
        res += ", " + sep;
    }

    return res;
}

std::string qmarks(size_t n) {
    return sql_binder(n, "?");
}

std::string qmark_pairs(size_t n) {
    return sql_binder(n, "(?, ?)");
}

std::string gram_pair(size_t n) {
    return sql_binder(n, "(?, 1)");
}

std::string gram_triple(size_t n) {
    return sql_binder(n, "(?, ?, 1)");
}

} // namespace

Statement SQL::SelectInputsByFreq(DbHandle &db) {
    return Statement(db, "SELECT * FROM frequency ORDER BY id");
}

Statement SQL::SelectSyllables(DbHandle &db) {
    return Statement(db, "SELECT input FROM syllables");
}

SQLite::Statement SQL::SelectSymbols(DbHandle &db) {
    return Statement(db, "SELECT * FROM symbols");
}

SQLite::Statement SQL::SelectEmojis(DbHandle &db) {
    return Statement(db, "SELECT * FROM emojis ORDER BY category ASC, id ASC");
}

SQLite::Statement SQL::SelectConversions(DbHandle &db, int input_id) {
    static constexpr auto sql = R"(
        SELECT
            c.id,
            f.input,
            c.output,
            c.category,
            c.annotation,
            c.weight
        FROM conversions AS c
        INNER JOIN frequency AS f
        ON c.input_id = f.id
        WHERE c.input_id=?
        ORDER BY c.id
    )";

    auto ret = Statement(db, sql);
    ret.bind(1, input_id);
    return ret;
}

SQLite::Statement SQL::SelectConversions(DbHandle &db, std::vector<std::string *> const &inputs) {
    static constexpr auto sql = R"(
        SELECT
            c.id,
            f.input,
            c.output,
            c.category,
            c.annotation
        FROM conversions AS c
        INNER JOIN frequency AS f
        ON c.input_id = f.id
        WHERE f.input IN (%s)
        ORDER BY c.id
    )";

    auto ret = Statement(db, format(sql, qmarks(inputs.size())));
    auto i = 1;
    for (auto *input : inputs) {
        ret.bind(i, *input);
        ++i;
    }
    return ret;
}

SQLite::Statement SQL::SelectBestUnigram(DbHandle &db, std::vector<std::string *> const &grams) {
    static constexpr auto sql = R"(
        SELECT gram, n
        FROM unigram_freq
        WHERE gram in (%s)
        ORDER BY n DESC LIMIT 1
    )";

    auto ret = Statement(db, format(sql, qmarks(grams.size())));
    auto i = 1;
    for (auto *gram : grams) {
        ret.bind(i, *gram);
        ++i;
    }
    return ret;
}

SQLite::Statement SQL::SelectBestBigram(DbHandle &db, std::string const &lgram,
                                        std::vector<std::string *> const &rgrams) {
    static constexpr auto sql = R"(
        SELECT *
        FROM bigram_freq
        WHERE lgram = ?
        AND rgram in (%s)
        ORDER BY n DESC LIMIT 1
    )";

    auto ret = Statement(db, format(sql, qmarks(rgrams.size())));
    ret.bind(1, lgram);
    auto i = 2;
    for (auto *gram : rgrams) {
        ret.bind(i, *gram);
        ++i;
    }
    return ret;
}

SQLite::Statement SQL::SelectUnigrams(DbHandle &db, std::vector<std::string *> const &grams) {
    static constexpr auto sql = R"(
        SELECT gram, n
        FROM unigram_freq
        WHERE gram in (%s)
    )";

    auto ret = Statement(db, format(sql, qmarks(grams.size())));
    auto i = 1;
    for (auto *gram : grams) {
        ret.bind(i, *gram);
        ++i;
    }
    return ret;
}

SQLite::Statement SQL::SelectBigrams(DbHandle &db, std::string const &lgram, std::vector<std::string *> const &rgrams) {
    static constexpr auto sql = R"(
        SELECT rgram, n
        FROM bigram_freq
        WHERE lgram = ?
        AND rgram in (%s)
    )";

    auto ret = Statement(db, format(sql, qmarks(rgrams.size())));
    ret.bind(1, lgram);
    auto i = 2;
    for (auto *gram : rgrams) {
        ret.bind(i, *gram);
        ++i;
    }
    return ret;
}

SQLite::Statement SQL::SelectUnigramCount(DbHandle &db, std::string const &gram) {
    auto ret = Statement(db, "SELECT n FROM unigram_freq WHERE gram = ?");
    ret.bind(1, gram);
    return ret;
}

Statement SQL::SelectUnigramCounts(DbHandle &db, std::vector<std::string> const &grams) {
    static constexpr auto sql = R"(
        SELECT gram, n
        FROM unigram_freq
        WHERE gram in (%s)
    )";

    auto ret = Statement(db, format(sql, qmarks(grams.size())));
    auto i = 1;
    for (auto const &gram : grams) {
        ret.bind(i, gram);
        ++i;
    }
    return ret;
}

SQLite::Statement SQL::SelectBigramCount(DbHandle &db, std::string const &lgram, std::string const &rgram) {
    auto ret = Statement(db, "SELECT n FROM bigram_freq WHERE lgram = ? AND rgram = ?");
    ret.bind(1, lgram);
    ret.bind(2, rgram);
    return ret;
}

SQLite::Statement SQL::SelectBigramCounts(DbHandle &db, std::string const &lgram,
                                          std::vector<std::string> const &rgrams) {
    static constexpr auto sql = R"(
        SELECT n, rgram
        FROM bigram_freq
        WHERE lgram = ?
        AND rgram in (%s)
    )";

    auto ret = Statement(db, format(sql, qmarks(rgrams.size())));
    ret.bind(1, lgram);
    auto i = 2;
    for (auto const &rgram : rgrams) {
        ret.bind(i, rgram);
        ++i;
    }
    return ret;
}

SQLite::Statement SQL::IncrementUnigrams(DbHandle &db, std::vector<std::string> const &grams) {
    static constexpr auto sql = R"(
        INSERT INTO unigram_freq (gram, n)
            VALUES %s
        ON CONFLICT DO UPDATE
            SET n = n + 1
        WHERE gram IN (%s)
    )";

    auto size = grams.size();
    auto ret = Statement(db, format(sql, gram_pair(size), qmarks(size)));

    auto i = 1;
    for (auto const &gram : grams) {
        ret.bind(i, gram);
        ret.bind(i + static_cast<int>(size), gram);
        ++i;
    }

    return ret;
}

SQLite::Statement SQL::IncrementBigrams(DbHandle &db, std::vector<std::pair<std::string, std::string>> const &bigrams) {
    static constexpr auto sql = R"(
        INSERT INTO bigram_freq (lgram, rgram, n)
            VALUES %s
        ON CONFLICT DO UPDATE
            SET n = n + 1
        WHERE (lgram, rgram) IN (VALUES %s )
    )";

    auto size = bigrams.size();
    auto ret = Statement(db, format(sql, gram_triple(size), qmark_pairs(size)));
    auto i = 1;
    for (auto const &bigram : bigrams) {
        ret.bind(i, bigram.first);
        ret.bind(i + static_cast<int>(size) * 2, bigram.first);
        ++i;
        ret.bind(i, bigram.second);
        ret.bind(i + static_cast<int>(size) * 2, bigram.second);
        ++i;
    }
    return ret;
}

SQLite::Statement SQL::DeleteUnigrams(DbHandle &db) {
    return Statement(db, "DELETE FROM unigram_freq");
}

SQLite::Statement SQL::DeleteBigrams(DbHandle &db) {
    return Statement(db, "DELETE FROM bigram_freq");
}

constexpr auto kCreateDummyDb = R"(BEGIN TRANSACTION;
    DROP TABLE IF EXISTS "version";
    DROP TABLE IF EXISTS "syllables";
    DROP TABLE IF EXISTS "unigram_freq";
    DROP TABLE IF EXISTS "bigram_freq";
    DROP TABLE IF EXISTS "conversions";
    DROP TABLE IF EXISTS "frequency";
    DROP TABLE IF EXISTS "punctuation";
    CREATE TABLE IF NOT EXISTS "version" ("key" TEXT, "value" INTEGER);
    CREATE TABLE "syllables" ("id" INTEGER PRIMARY KEY, "input" TEXT NOT NULL, UNIQUE("input"));
    CREATE TABLE IF NOT EXISTS "unigram_freq" ("id" INTEGER, "gram" TEXT NOT NULL UNIQUE, "n" INTEGER NOT NULL, PRIMARY KEY("id"));
    CREATE TABLE IF NOT EXISTS "bigram_freq" ("id" INTEGER, "lgram" TEXT, "rgram" TEXT, "n" INTEGER NOT NULL, PRIMARY KEY("id"), UNIQUE("lgram","rgram"));
    CREATE TABLE "frequency" ("id" INTEGER PRIMARY KEY, "input" TEXT NOT NULL, "freq" INTEGER, "chhan_id" INTEGER, UNIQUE("input"));
    CREATE TABLE "conversions" (
     "id"           INTEGER PRIMARY KEY,
     "input_id"     INTEGER,
     "output"       TEXT NOT NULL,
     "weight"       INTEGER,
     "category"     INTEGER,
     "annotation"   TEXT,
     UNIQUE("input_id","output"),
        FOREIGN KEY("input_id") REFERENCES "frequency"("id")
    );
    INSERT INTO "version" VALUES ('id',1), ('created_at',1639838788), ('last_updated',1639839434);
    INSERT INTO "syllables" VALUES (1,'e'), (2,'a'), (3,'si'), (4,'bo'), (5,'lang'), (6,'chit'), (7,'chiah'), (8,'m'), (9,'i'), (10,'khi');
    INSERT INTO "frequency" VALUES (1, "ê", 100, 1), (2, "bô", 50, 2), (3, "tio̍h", 20, 3);
    INSERT INTO "conversions" VALUES
     (1,1,'个',1000,1,NULL), (2,1,'兮',900,1,NULL), (3,1,'ê',800,1,NULL),
     (4,2,'無',1000,1,NULL), (5,2,'bô',900,1,NULL),
     (6,3,'着',1000,1,NULL), (7,3,'tio̍h',900,1,NULL);
    DROP INDEX IF EXISTS "unigram_freq_gram_idx";
    CREATE INDEX IF NOT EXISTS "unigram_freq_gram_idx" ON "unigram_freq" (
     "gram"
    );
    DROP INDEX IF EXISTS "bigram_freq_gram_index";
    CREATE INDEX IF NOT EXISTS "bigram_freq_gram_index" ON "bigram_freq" (
     "lgram",
     "rgram"
    );
    CREATE TABLE "punctuation" (
	    "id"           INTEGER PRIMARY KEY,
	    "input"        TEXT NOT NULL,
	    "output"       TEXT NOT NULL,
	    "annotation"   TEXT,
	    UNIQUE("input","output")
    );
    COMMIT;
    )";

SQLite::Statement SQL::CreateDummyDb(DbHandle &db) {
    return Statement(db, kCreateDummyDb);
}

} // namespace khiin::engine
