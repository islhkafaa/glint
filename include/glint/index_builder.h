#pragma once

#include "glint/database.h"
#include <map>
#include <string>
#include <vector>


namespace glint {

class IndexBuilder {
public:
  explicit IndexBuilder(Database &db);

  void indexFile(const std::string &filePath,
                 const std::vector<std::string> &tokens);

private:
  Database &db_;
};

} // namespace glint
