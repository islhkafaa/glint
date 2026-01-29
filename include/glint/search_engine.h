#pragma once

#include "glint/database.h"
#include <string>
#include <vector>

namespace glint {

struct SearchResult {
  std::string filePath;
  int score;
  std::string preview;

  SearchResult(const std::string &path, int s, const std::string& prev = "") : filePath(path), score(s), preview(prev) {}
};

class SearchEngine {
public:
  explicit SearchEngine(Database &db);

  std::vector<SearchResult> search(const std::string &query) const;

private:
  Database &db_;
};

} // namespace glint
