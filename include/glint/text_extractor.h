#pragma once

#include <filesystem>
#include <string>


namespace glint {

class TextExtractor {
public:
  static constexpr size_t MAX_FILE_SIZE = 10 * 1024 * 1024;

  static std::string extractText(const std::filesystem::path &filePath);

private:
  static bool isTextFile(const std::filesystem::path &filePath);
};

} // namespace glint
