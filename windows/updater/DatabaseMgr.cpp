#include "DatabaseMgr.h"

#include <SQLiteCpp/SQLiteCpp.h>

namespace khiin::data {
namespace {

class DatabaseMgrImpl : DatabaseMgr {
  public:
    DatabaseMgrImpl() {
        m_db = std::make_unique<SQLite::Database>(":memory:", SQLite::OPEN_READWRITE);
    }

    void Load(std::string const &old_db, std::string const &new_db) {
        m_db->exec("ATTACH DATABASE \"" + old_db + "\" AS old");
        m_db->exec("ATTACH DATABASE \"" + new_db + "\" AS new");
    }

    void MigrateSchema() {
        auto from_schema = m_db->execAndGet("PRAGMA old.user_version").getInt();
        auto to_schema = m_db->execAndGet("PRAGMA new.user_version").getInt();

        if (from_schema == to_schema) {
            return;
        }

        // Do migration - nothing to do yet
    }

    void CopyData() {
        m_db->exec(R"(
            BEGIN TRANSACTION;
            DELETE FROM old.conversions;
            DELETE FROM old.frequency;
            DELETE FROM old.syllables;
            DELETE FROM old.symbols;
            DELETE FROM old.emoji;
            DELETE FROM old.version;
            INSERT INTO old.version SELECT * FROM new.version;
            INSERT INTO old.emoji SELECT * FROM new.emoji;
            INSERT INTO old.symbols SELECT * FROM new.symbols;
            INSERT INTO old.syllables SELECT * FROM new.syllables;
            INSERT INTO old.frequency SELECT * FROM new.frequency;
            INSERT INTO old.conversions SELECT * FROM new.conversions;
            COMMIT TRANSACTION;
        )");
    }

  private:
    std::unique_ptr<SQLite::Database> m_db;
};

} // namespace

int DatabaseMgr::Update(std::string const &old_db, std::string const &new_db) {
    auto updater = DatabaseMgrImpl();
    try {
        updater.Load(old_db, new_db);
    } catch (...) {
        return 1;
    }

    try {
        updater.MigrateSchema();
    } catch (...) {
        return 2;
    }

    try {
        updater.CopyData();
    } catch (...) {
        return 3;
    }

    return 0;
}

} // namespace khiin::data
