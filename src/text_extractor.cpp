#include "glint/text_extractor.h"
#include <fstream>
#include <set>
#include <sstream>


namespace glint {

bool TextExtractor::isTextFile(const std::filesystem::path &filePath) {
  static const std::set<std::string> textExtensions = {
      ".txt", ".md",   ".cpp", ".h",     ".hpp",  ".c",    ".cc",
      ".cxx", ".py",   ".js",  ".ts",    ".java", ".cs",   ".go",
      ".rs",  ".html", ".css", ".xml",   ".json", ".yaml", ".yml",
      ".sh",  ".bat",  ".ps1", ".cmake", ".ini",  ".cfg"};

  std::string ext = filePath.extension().string();
  return textExtensions.find(ext) != textExtensions.end();
}

std::string TextExtractor::extractText(const std::filesystem::path &filePath) {
  if (!std::filesystem::exists(filePath)) {
    return "";
  }

  if (!std::filesystem::is_regular_file(filePath)) {
    return "";
  }

  auto fileSize = std::filesystem::file_size(filePath);
  if (fileSize > MAX_FILE_SIZE || fileSize == 0) {
    return "";
  }

  if (!isTextFile(filePath)) {
    return "";
  }

  std::ifstream file(filePath, std::ios::binary);
  if (!file.is_open()) {
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

} // namespace glint
