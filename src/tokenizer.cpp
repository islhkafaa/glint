#include "glint/tokenizer.h"
#include <cctype>
#include <sstream>

namespace glint {

std::string Tokenizer::normalize(const std::string &word) {
  std::string normalized;
  normalized.reserve(word.size());

  for (char c : word) {
    if (std::isalnum(static_cast<unsigned char>(c))) {
      normalized += std::tolower(static_cast<unsigned char>(c));
    }
  }

  return normalized;
}

bool Tokenizer::isValidToken(const std::string &token) {
  if (token.length() < MIN_WORD_LENGTH) {
    return false;
  }

  for (char c : token) {
    if (std::isalpha(static_cast<unsigned char>(c))) {
      return true;
    }
  }

  return false;
}

std::vector<std::string> Tokenizer::tokenize(const std::string &text) {
  std::vector<std::string> tokens;
  std::istringstream stream(text);
  std::string word;

  while (stream >> word) {
    std::string normalized = normalize(word);
    if (isValidToken(normalized)) {
      tokens.push_back(normalized);
    }
  }

  return tokens;
}

} // namespace glint
