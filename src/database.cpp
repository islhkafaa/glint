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

        CREATE TABLE IF NOT EXISTS tokens (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            token TEXT UNIQUE NOT NULL
        );
        CREATE INDEX IF NOT EXISTS idx_token ON tokens(token);

        CREATE TABLE IF NOT EXISTS token_files (
            token_id INTEGER NOT NULL,
            file_id INTEGER NOT NULL,
            frequency INTEGER NOT NULL,
            PRIMARY KEY (token_id, file_id),
            FOREIGN KEY (token_id) REFERENCES tokens(id),
            FOREIGN KEY (file_id) REFERENCES files(id)
        );
        CREATE INDEX IF NOT EXISTS idx_token_files_token ON token_files(token_id);
        CREATE INDEX IF NOT EXISTS idx_token_files_file ON token_files(file_id);
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

void Database::insertToken(const std::string &token, int fileId,
                           int frequency) {
  std::ostringstream sql;
  sql << "INSERT OR IGNORE INTO tokens (token) VALUES ('" << token << "');";
  executeSQL(sql.str().c_str());

  sql.str("");
  sql << "INSERT OR REPLACE INTO token_files (token_id, file_id, frequency) "
      << "VALUES ((SELECT id FROM tokens WHERE token = '" << token << "'), "
      << fileId << ", " << frequency << ");";
  executeSQL(sql.str().c_str());
}

void Database::insertTokens(
    const std::vector<std::tuple<std::string, int, int>> &tokens) {
  executeSQL("BEGIN TRANSACTION;");

  try {
    for (const auto &[token, fileId, frequency] : tokens) {
      insertToken(token, fileId, frequency);
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

size_t Database::getTokenCount() const {
  sqlite3_stmt *stmt = nullptr;
  const char *sql = "SELECT COUNT(*) FROM tokens;";

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

int Database::getFileId(const std::string &path) const {
  sqlite3_stmt *stmt = nullptr;
  const char *sql = "SELECT id FROM files WHERE path = ?;";

  int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return -1;
  }

  sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);

  int fileId = -1;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    fileId = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return fileId;
}

std::string Database::getFilePath(int fileId) const {
  sqlite3_stmt *stmt = nullptr;
  const char *sql = "SELECT path FROM files WHERE id = ?;";

  int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return "";
  }

  sqlite3_bind_int(stmt, 1, fileId);

  std::string path;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const char *pathStr =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    if (pathStr) {
      path = pathStr;
    }
  }

  sqlite3_finalize(stmt);
  return path;
}

std::vector<std::pair<int, int>>
Database::searchToken(const std::string &token) const {
  std::vector<std::pair<int, int>> results;
  sqlite3_stmt *stmt = nullptr;
  const char *sql = R"(
    SELECT tf.file_id, tf.frequency
    FROM token_files tf
    JOIN tokens t ON tf.token_id = t.id
    WHERE t.token = ?;
  )";

  int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return results;
  }

  sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int fileId = sqlite3_column_int(stmt, 0);
    int frequency = sqlite3_column_int(stmt, 1);
    results.emplace_back(fileId, frequency);
  }

  sqlite3_finalize(stmt);
  return results;
}

} // namespace glint
