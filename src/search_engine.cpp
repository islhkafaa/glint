#include "glint/search_engine.h"
#include "glint/text_extractor.h"
#include "glint/tokenizer.h"
#include <algorithm>
#include <cctype>
#include <map>
#include <set>
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

bool containsPhrase(const std::string &text, const std::string &phrase) {
  std::string lowerText = text;
  std::string lowerPhrase = phrase;
  std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(),
                 ::tolower);
  std::transform(lowerPhrase.begin(), lowerPhrase.end(), lowerPhrase.begin(),
                 ::tolower);
  return lowerText.find(lowerPhrase) != std::string::npos;
}

std::vector<SearchResult> SearchEngine::search(const std::string &query) const {
  return search(query, "");
}

std::vector<SearchResult>
SearchEngine::search(const std::string &query,
                     const std::string &fileTypeFilter) const {
  std::vector<std::string> phrases;
  std::string remainingQuery = query;
  size_t quotePos = 0;

  while ((quotePos = remainingQuery.find('"')) != std::string::npos) {
    size_t endQuote = remainingQuery.find('"', quotePos + 1);
    if (endQuote != std::string::npos) {
      std::string phrase =
          remainingQuery.substr(quotePos + 1, endQuote - quotePos - 1);
      phrases.push_back(phrase);
      remainingQuery.erase(quotePos, endQuote - quotePos + 1);
    } else {
      break;
    }
  }

  std::vector<std::string> andTerms;
  std::vector<std::string> orTerms;
  std::vector<std::string> notTerms;

  std::istringstream iss(remainingQuery);
  std::string word;
  std::string lastOp = "OR";

  while (iss >> word) {
    if (word == "AND" || word == "OR" || word == "NOT") {
      lastOp = word;
    } else {
      if (lastOp == "AND") {
        andTerms.push_back(word);
      } else if (lastOp == "NOT") {
        notTerms.push_back(word);
      } else {
        orTerms.push_back(word);
      }
      lastOp = "OR";
    }
  }

  std::vector<std::string> andTokens;
  std::vector<std::string> orTokens;
  std::vector<std::string> notTokens;

  for (const auto &term : andTerms) {
    auto tokens = Tokenizer::tokenize(term);
    andTokens.insert(andTokens.end(), tokens.begin(), tokens.end());
  }
  for (const auto &term : orTerms) {
    auto tokens = Tokenizer::tokenize(term);
    orTokens.insert(orTokens.end(), tokens.begin(), tokens.end());
  }
  for (const auto &term : notTerms) {
    auto tokens = Tokenizer::tokenize(term);
    notTokens.insert(notTokens.end(), tokens.begin(), tokens.end());
  }

  std::vector<std::string> allQueryTokens = orTokens;
  allQueryTokens.insert(allQueryTokens.end(), andTokens.begin(),
                        andTokens.end());

  if (allQueryTokens.empty() && phrases.empty()) {
    return {};
  }

  std::map<int, int> fileScores;
  std::set<int> andFileSet;
  std::set<int> notFileSet;

  for (const auto &token : orTokens) {
    auto results = db_.searchToken(token);
    for (const auto &[fileId, frequency] : results) {
      fileScores[fileId] += frequency;
    }
  }

  if (!andTokens.empty()) {
    bool first = true;
    for (const auto &token : andTokens) {
      auto results = db_.searchToken(token);
      std::set<int> currentSet;
      for (const auto &[fileId, frequency] : results) {
        currentSet.insert(fileId);
        fileScores[fileId] += frequency;
      }

      if (first) {
        andFileSet = currentSet;
        first = false;
      } else {
        std::set<int> intersection;
        std::set_intersection(
            andFileSet.begin(), andFileSet.end(), currentSet.begin(),
            currentSet.end(),
            std::inserter(intersection, intersection.begin()));
        andFileSet = intersection;
      }
    }
  }

  for (const auto &token : notTokens) {
    auto results = db_.searchToken(token);
    for (const auto &[fileId, frequency] : results) {
      notFileSet.insert(fileId);
    }
  }

  std::vector<SearchResult> searchResults;

  for (const auto &[fileId, score] : fileScores) {
    if (notFileSet.count(fileId) > 0) {
      continue;
    }

    if (!andTokens.empty() && andFileSet.count(fileId) == 0) {
      continue;
    }

    std::string filePath = db_.getFilePath(fileId);
    if (filePath.empty()) {
      continue;
    }

    if (!fileTypeFilter.empty()) {
      size_t dotPos = filePath.find_last_of('.');
      if (dotPos == std::string::npos) {
        continue;
      }
      std::string ext = filePath.substr(dotPos + 1);
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
      if (ext != fileTypeFilter) {
        continue;
      }
    }

    bool phraseMatch = true;
    if (!phrases.empty()) {
      std::string text = TextExtractor::extractText(filePath);
      for (const auto &phrase : phrases) {
        if (!containsPhrase(text, phrase)) {
          phraseMatch = false;
          break;
        }
      }
    }

    if (!phraseMatch) {
      continue;
    }

    std::string text = TextExtractor::extractText(filePath);
    std::string preview = generatePreview(text, allQueryTokens);

    searchResults.emplace_back(filePath, score, preview);
  }

  std::sort(searchResults.begin(), searchResults.end(),
            [](const SearchResult &a, const SearchResult &b) {
              return a.score > b.score;
            });

  return searchResults;
}

} // namespace glint
