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

  void insertToken(const std::string &token, int fileId, int frequency);
  void
  insertTokens(const std::vector<std::tuple<std::string, int, int>> &tokens);

  size_t getFileCount() const;
  size_t getTokenCount() const;
  int getFileId(const std::string &path) const;
  std::string getFilePath(int fileId) const;
  std::vector<std::pair<int, int>> searchToken(const std::string &token) const;

  bool isFileModified(const std::string &path,
                      std::filesystem::file_time_type modTime) const;
  void deleteFileTokens(int fileId);
  void optimizeDatabase();
  bool hasFileTokens(int fileId) const;

private:
  void executeSQL(const char *sql);

  std::string dbPath_;
  sqlite3 *db_;
};

} // namespace glint
