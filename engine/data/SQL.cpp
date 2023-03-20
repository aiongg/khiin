#include "SQL.h"

#include "utils/utils.h"

namespace khiin::engine {
namespace {
using namespace SQLite;
using namespace khiin::engine::utils;

std::string sql_binder(size_t n, std::string &&sep) {
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

Statement SQL::SelectAllKeySequences(DbHandle &db, InputType inputType) {
    switch (inputType) {
    case InputType::Telex:
        return Statement(db, "SELECT DISTINCT input_id, key_sequence FROM lookup_telex");
    default:
        return Statement(db, "SELECT DISTINCT input_id, key_sequence FROM lookup_numeric");
    }
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

SQLite::Statement SQL::SelectConversions(DbHandle &db, std::vector<std::string> const &inputs, const InputType input_type) {
    static constexpr auto sql_numeric = R"(
        SELECT *
        FROM lookup_numeric
        WHERE key_sequence IN (%s)
    )";

    static constexpr auto sql_telex = R"(
        SELECT *
        FROM lookup_telex
        WHERE key_sequence IN (%s)
    )";

    auto ret = Statement(db, "");

    switch (input_type) {
    case InputType::Numeric:
        ret = Statement(db, format(sql_numeric, qmarks(inputs.size())));
        break;
    case InputType::Telex:
        ret = Statement(db, format(sql_telex, qmarks(inputs.size())));
        break;
    }

    auto i = 1;
    for (const auto &input : inputs) {
        ret.bind(i, input);
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

constexpr auto kCreateDummyDb = R"(
BEGIN TRANSACTION;
DROP TABLE IF EXISTS "version";
CREATE TABLE IF NOT EXISTS "version" (
	"key"	TEXT,
	"value"	INTEGER
);
DROP TABLE IF EXISTS "frequency";
CREATE TABLE IF NOT EXISTS "frequency" (
	"id"	INTEGER,
	"input"	TEXT NOT NULL,
	"freq"	INTEGER,
	"chhan_id"	INTEGER,
	PRIMARY KEY("id"),
	UNIQUE("input")
);
DROP TABLE IF EXISTS "conversions";
CREATE TABLE IF NOT EXISTS "conversions" (
	"id"	INTEGER,
	"input_id"	INTEGER,
	"output"	TEXT NOT NULL,
	"weight"	INTEGER,
	"category"	INTEGER,
	"annotation"	TEXT,
	PRIMARY KEY("id"),
	FOREIGN KEY("input_id") REFERENCES "frequency"("id"),
	UNIQUE("input_id","output")
);
DROP TABLE IF EXISTS "syllables";
CREATE TABLE IF NOT EXISTS "syllables" (
	"id"	INTEGER,
	"input"	TEXT NOT NULL,
	PRIMARY KEY("id"),
	UNIQUE("input")
);
DROP TABLE IF EXISTS "unigram_freq";
CREATE TABLE IF NOT EXISTS "unigram_freq" (
	"id"	INTEGER,
	"gram"	TEXT NOT NULL UNIQUE,
	"n"	INTEGER NOT NULL,
	PRIMARY KEY("id")
);
DROP TABLE IF EXISTS "bigram_freq";
CREATE TABLE IF NOT EXISTS "bigram_freq" (
	"id"	INTEGER,
	"lgram"	TEXT,
	"rgram"	TEXT,
	"n"	INTEGER NOT NULL,
	PRIMARY KEY("id"),
	UNIQUE("lgram","rgram")
);
DROP TABLE IF EXISTS "symbols";
CREATE TABLE IF NOT EXISTS "symbols" (
	"id"	INTEGER,
	"input"	TEXT NOT NULL,
	"output"	TEXT NOT NULL,
	"category"	INTEGER,
	"annotation"	TEXT,
	PRIMARY KEY("id")
);
DROP TABLE IF EXISTS "emoji";
CREATE TABLE IF NOT EXISTS "emoji" (
	"id"	INTEGER,
	"emoji"	TEXT NOT NULL,
	"short_name"	TEXT NOT NULL,
	"category"	INTEGER NOT NULL,
	"code"	TEXT NOT NULL,
	PRIMARY KEY("id")
);
INSERT INTO "frequency" VALUES (1,'ê',33807,32),
 (2,'góa',15113,36),
 (3,'lí',13744,44),
 (4,'bô',12353,40),
 (5,'i',12275,34),
 (6,'ū',12082,35),
 (7,'sī',10206,33),
 (8,'lâng',9492,7),
 (9,'lâi',9429,3),
 (10,'tio̍h',8871,10);
INSERT INTO "conversions" VALUES (97,4,'無',1000,NULL,NULL),
 (98,4,'bô',900,NULL,NULL),
 (460,2,'我',1000,NULL,NULL),
 (461,2,'góa',900,NULL,NULL),
 (593,5,'伊',1000,NULL,NULL),
 (594,5,'i',900,NULL,NULL),
 (1136,9,'來',1000,NULL,NULL),
 (1137,9,'lâi',900,NULL,NULL),
 (1140,8,'人',1000,NULL,NULL),
 (1141,8,'籠',1000,NULL,NULL),
 (1142,8,'lâng',900,NULL,NULL),
 (1167,3,'李',1000,NULL,NULL),
 (1168,3,'汝',1000,NULL,NULL),
 (1169,3,'Lí',900,NULL,NULL),
 (1170,3,'lí',900,NULL,NULL),
 (1714,7,'是',1000,NULL,NULL),
 (1715,7,'示',1000,NULL,NULL),
 (1716,7,'sī',900,NULL,NULL),
 (1813,10,'着',1000,NULL,NULL),
 (1814,10,'tio̍h',900,NULL,NULL),
 (2074,1,'个',1000,NULL,NULL),
 (2075,1,'兮',1000,NULL,NULL),
 (2076,1,'鞋',1000,NULL,NULL),
 (2077,1,'ê',900,NULL,NULL),
 (2148,6,'有',1000,NULL,NULL),
 (2149,6,'ū',900,NULL,NULL);
INSERT INTO "syllables" VALUES (1,'a'),
 (2,'ah'),
 (3,'ahⁿ'),
 (4,'ai'),
 (5,'aih'),
 (6,'aihⁿ'),
 (7,'aiⁿ'),
 (8,'ak'),
 (9,'am'),
 (10,'an');
INSERT INTO "symbols" VALUES (1,'!','!',0,NULL),
 (2,'!','！',1,NULL),
 (3,'"','"',0,NULL),
 (4,'"','＂',1,NULL),
 (5,'"','“”',0,NULL),
 (6,'"','‘’',0,NULL),
 (7,'#','#',0,NULL),
 (8,'#','＃',1,NULL),
 (9,'$','$',0,NULL),
 (10,'$','¢',0,NULL);
INSERT INTO "emoji" VALUES (1,'😀','grinning face',1,'U+1F600'),
 (2,'😃','grinning face with big eyes',1,'U+1F603'),
 (3,'😄','grinning face with smiling eyes',1,'U+1F604'),
 (4,'😁','beaming face with smiling eyes',1,'U+1F601'),
 (5,'😆','grinning squinting face',1,'U+1F606'),
 (6,'😅','grinning face with sweat',1,'U+1F605'),
 (7,'🤣','rolling on the floor laughing',1,'U+1F923'),
 (8,'😂','face with tears of joy',1,'U+1F602'),
 (9,'🙂','slightly smiling face',1,'U+1F642'),
 (10,'🙃','upside-down face',1,'U+1F643');
DROP INDEX IF EXISTS "unigram_freq_gram_idx";
CREATE INDEX IF NOT EXISTS "unigram_freq_gram_idx" ON "unigram_freq" (
	"gram"
);
DROP INDEX IF EXISTS "bigram_freq_gram_index";
CREATE INDEX IF NOT EXISTS "bigram_freq_gram_index" ON "bigram_freq" (
	"lgram",
	"rgram"
);
COMMIT;)";

int SQL::CreateDummyDb(DbHandle &db) {
    return db.exec(kCreateDummyDb);
}

} // namespace khiin::engine
