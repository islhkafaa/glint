#pragma once

#include "file_info.h"
#include <filesystem>
#include <functional>
#include <set>
#include <vector>

namespace glint {

class DirectoryCrawler {
public:
  using ProgressCallback = std::function<void(const FileInfo &)>;

  explicit DirectoryCrawler(const std::filesystem::path &rootPath);

  void setFileExtensions(const std::set<std::string> &extensions);
  void setProgressCallback(ProgressCallback callback);

  std::vector<FileInfo> crawl();

private:
  bool shouldProcessFile(const std::filesystem::path &path) const;
  void crawlRecursive(const std::filesystem::path &dir,
                      std::vector<FileInfo> &results);

  std::filesystem::path rootPath_;
  std::set<std::string> allowedExtensions_;
  ProgressCallback progressCallback_;
  size_t filesProcessed_;
};

} // namespace glint
