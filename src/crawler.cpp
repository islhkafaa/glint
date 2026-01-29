#include "glint/crawler.h"

#include <iostream>

namespace glint {

DirectoryCrawler::DirectoryCrawler(const std::filesystem::path &rootPath)
    : rootPath_(rootPath), filesProcessed_(0) {}

void DirectoryCrawler::setFileExtensions(
    const std::set<std::string> &extensions) {
  allowedExtensions_ = extensions;
}

void DirectoryCrawler::setProgressCallback(ProgressCallback callback) {
  progressCallback_ = callback;
}

bool DirectoryCrawler::shouldProcessFile(
    const std::filesystem::path &path) const {
  if (!std::filesystem::is_regular_file(path)) {
    return false;
  }

  auto filename = path.filename().string();
  if (!filename.empty() && filename[0] == '.') {
    return false;
  }

  if (allowedExtensions_.empty()) {
    return true;
  }

  std::string ext = path.extension().string();
  return allowedExtensions_.find(ext) != allowedExtensions_.end();
}

void DirectoryCrawler::crawlRecursive(const std::filesystem::path &dir,
                                      std::vector<FileInfo> &results) {
  try {
    for (const auto &entry : std::filesystem::directory_iterator(dir)) {
      try {
        if (std::filesystem::is_directory(entry)) {
          auto dirname = entry.path().filename().string();
          if (!dirname.empty() && dirname[0] != '.') {
            crawlRecursive(entry.path(), results);
          }
        } else if (shouldProcessFile(entry.path())) {
          FileInfo info(entry.path());
          results.push_back(info);
          filesProcessed_++;

          if (progressCallback_) {
            progressCallback_(info);
          }
        }
      } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Error accessing: " << entry.path() << " - " << e.what()
                  << "\n";
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Error reading directory: " << dir << " - " << e.what()
              << "\n";
  }
}

std::vector<FileInfo> DirectoryCrawler::crawl() {
  std::vector<FileInfo> results;
  filesProcessed_ = 0;

  if (!std::filesystem::exists(rootPath_)) {
    std::cerr << "Path does not exist: " << rootPath_ << "\n";
    return results;
  }

  if (!std::filesystem::is_directory(rootPath_)) {
    std::cerr << "Path is not a directory: " << rootPath_ << "\n";
    return results;
  }

  crawlRecursive(rootPath_, results);
  return results;
}

} // namespace glint
