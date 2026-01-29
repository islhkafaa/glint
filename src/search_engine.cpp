#include "glint/search_engine.h"
#include "glint/tokenizer.h"
#include <algorithm>
#include <map>


namespace glint {

SearchEngine::SearchEngine(Database &db) : db_(db) {}

std::vector<SearchResult> SearchEngine::search(const std::string &query) const {
  auto queryTokens = Tokenizer::tokenize(query);

  if (queryTokens.empty()) {
    return {};
  }

  std::map<int, int> fileScores;

  for (const auto &token : queryTokens) {
    auto results = db_.searchToken(token);
    for (const auto &[fileId, frequency] : results) {
      fileScores[fileId] += frequency;
    }
  }

  std::vector<SearchResult> searchResults;
  searchResults.reserve(fileScores.size());

  for (const auto &[fileId, score] : fileScores) {
    std::string filePath = db_.getFilePath(fileId);
    if (!filePath.empty()) {
      searchResults.emplace_back(filePath, score);
    }
  }

  std::sort(searchResults.begin(), searchResults.end(),
            [](const SearchResult &a, const SearchResult &b) {
              return a.score > b.score;
            });

  return searchResults;
}

} // namespace glint
