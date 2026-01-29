#include "glint/database.h"
#include <iostream>
#include <sqlite3.h>
#include <sstream>
#include <stdexcept>


namespace glint {

Database::Database(const std::string &dbPath) : dbPath_(dbPath), db_(nullptr) {
  int rc = sqlite3_open(dbPath_.c_str(), &db_);
  if (rc != SQLITE_OK) {
    std::string error = sqlite3_errmsg(db_);
    sqlite3_close(db_);
    throw std::runtime_error("Failed to open database: " + error);
  }
}

Database::~Database() {
  if (db_) {
    sqlite3_close(db_);
  }
}

void Database::executeSQL(const char *sql) {
  char *errMsg = nullptr;
  int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);

  if (rc != SQLITE_OK) {
    std::string error = errMsg ? errMsg : "Unknown error";
    sqlite3_free(errMsg);
    throw std::runtime_error("SQL error: " + error);
  }
}

void Database::initialize() {
  const char *createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT UNIQUE NOT NULL,
            size INTEGER NOT NULL,
            modified_time INTEGER NOT NULL,
            extension TEXT
        );
        CREATE INDEX IF NOT EXISTS idx_path ON files(path);
        CREATE INDEX IF NOT EXISTS idx_extension ON files(extension);
    )";

  executeSQL(createTableSQL);
}

void Database::insertFile(const FileInfo &file) {
  auto timePoint = file.lastModified.time_since_epoch().count();

  std::ostringstream sql;
  sql << "INSERT OR REPLACE INTO files (path, size, modified_time, extension) "
         "VALUES ("
      << "'" << file.path.string() << "', " << file.size << ", " << timePoint
      << ", "
      << "'" << file.extension << "');";

  executeSQL(sql.str().c_str());
}

void Database::insertFiles(const std::vector<FileInfo> &files) {
  executeSQL("BEGIN TRANSACTION;");

  try {
    for (const auto &file : files) {
      insertFile(file);
    }
    executeSQL("COMMIT;");
  } catch (...) {
    executeSQL("ROLLBACK;");
    throw;
  }
}

size_t Database::getFileCount() const {
  sqlite3_stmt *stmt = nullptr;
  const char *sql = "SELECT COUNT(*) FROM files;";

  int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return 0;
  }

  size_t count = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    count = sqlite3_column_int64(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return count;
}

} // namespace glint
