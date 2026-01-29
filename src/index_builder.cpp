#include "glint/index_builder.h"
#include <map>
#include <tuple>

namespace glint {

IndexBuilder::IndexBuilder(Database &db) : db_(db) {}

void IndexBuilder::indexFile(const std::string &filePath,
                             const std::vector<std::string> &tokens) {
  int fileId = db_.getFileId(filePath);
  if (fileId == -1) {
    return;
  }

  std::map<std::string, int> tokenFrequency;
  for (const auto &token : tokens) {
    tokenFrequency[token]++;
  }

  std::vector<std::tuple<std::string, int, int>> tokenData;
  tokenData.reserve(tokenFrequency.size());

  for (const auto &[token, frequency] : tokenFrequency) {
    tokenData.emplace_back(token, fileId, frequency);
  }

  db_.insertTokens(tokenData);
}

} // namespace glint
