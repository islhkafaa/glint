#include "glint/search_engine.h"
#include "glint/text_extractor.h"
#include "glint/tokenizer.h"
#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>

namespace glint {

SearchEngine::SearchEngine(Database &db) : db_(db) {}

std::string generatePreview(const std::string &text,
                            const std::vector<std::string> &queryTokens) {
  if (text.empty() || queryTokens.empty()) {
    return "";
  }

  std::string lowerText = text;
  std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(),
                 ::tolower);

  size_t firstMatch = std::string::npos;
  for (const auto &token : queryTokens) {
    size_t pos = lowerText.find(token);
    if (pos != std::string::npos &&
        (firstMatch == std::string::npos || pos < firstMatch)) {
      firstMatch = pos;
    }
  }

  if (firstMatch == std::string::npos) {
    return text.substr(0, std::min(size_t(150), text.length())) + "...";
  }

  const size_t contextSize = 75;
  size_t start = (firstMatch > contextSize) ? firstMatch - contextSize : 0;
  size_t end = std::min(firstMatch + contextSize, text.length());

  while (start > 0 && !std::isspace(text[start]))
    start--;
  while (end < text.length() && !std::isspace(text[end]))
    end++;

  std::string preview = text.substr(start, end - start);

  if (start > 0)
    preview = "..." + preview;
  if (end < text.length())
    preview += "...";

  return preview;
}

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
      std::string text = TextExtractor::extractText(filePath);
      std::string preview = generatePreview(text, queryTokens);

      searchResults.emplace_back(filePath, score, preview);
    }
  }

  std::sort(searchResults.begin(), searchResults.end(),
            [](const SearchResult &a, const SearchResult &b) {
              return a.score > b.score;
            });

  return searchResults;
}

} // namespace glint
