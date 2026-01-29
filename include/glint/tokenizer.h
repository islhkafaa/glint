#pragma once

#include <string>
#include <vector>

namespace glint {

class Tokenizer {
public:
  static constexpr size_t MIN_WORD_LENGTH = 3;

  static std::vector<std::string> tokenize(const std::string &text);

private:
  static std::string normalize(const std::string &word);
  static bool isValidToken(const std::string &token);
};

} // namespace glint
