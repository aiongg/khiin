#include "pch.h"

#include "DatabaseMgr.h"

#include <filesystem>

#include <SQLiteCpp/SQLiteCpp.h>

#include "tip/Config.h"
#include "tip/Utils.h"

namespace khiin::data {
namespace {
namespace fs = std::filesystem;
using namespace khiin::win32;

class DatabaseMgrImpl : DatabaseMgr {
  public:
    DatabaseMgrImpl() {
        m_db = std::make_unique<SQLite::Database>(":memory:", SQLite::OPEN_READWRITE);
    }

    int DoUpdate(fs::path const& existing_db, fs::path const& update_db) {
        try {
            Load(existing_db.string(), update_db.string());
        } catch (...) {
            return 1;
        }

        m_db->exec("BEGIN");

        try {
            MigrateSchema();
        } catch (...) {
            m_db->exec("ROLLBACK");
            return 2;
        }

        try {
            CopyData();
        } catch (...) {
            m_db->exec("ROLLBACK");
            return 3;
        }

        m_db->exec("COMMIT");
        return 0;
    }

    void Load(std::string const &target_db, std::string const &source_db) {
        m_db->exec("ATTACH DATABASE \"" + target_db + "\" AS old");
        m_db->exec("ATTACH DATABASE \"" + source_db + "\" AS new");
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
        )");
    }

  private:
    std::unique_ptr<SQLite::Database> m_db;
};

} // namespace

int DatabaseMgr::ApplyUpdateToExisting(std::string const &existing_file, std::string const &update_file) {
    auto ret = -1;
    auto existing = fs::path(existing_file);
    auto update = fs::path(update_file);

    if (fs::exists(existing) && fs::exists(update)) {
        auto updater = DatabaseMgrImpl();
        ret = updater.DoUpdate(existing, update);
    } else if (fs::exists(update)) {
        fs::copy(update, existing);
        ret = 0;
    }

    if (ret == 0) {
        auto abspath = fs::absolute(existing);
        Config::SetDatabaseFile(Utils::Widen(abspath.string()));
    }

    return ret;
}

} // namespace khiin::data
