#pragma once

#include "file_info.h"
#include <string>
#include <vector>

struct sqlite3;

namespace glint {

class Database {
public:
  explicit Database(const std::string &dbPath);
  ~Database();

  Database(const Database &) = delete;
  Database &operator=(const Database &) = delete;

  void initialize();
  void insertFile(const FileInfo &file);
  void insertFiles(const std::vector<FileInfo> &files);

  size_t getFileCount() const;

private:
  void executeSQL(const char *sql);

  std::string dbPath_;
  sqlite3 *db_;
};

} // namespace glint
