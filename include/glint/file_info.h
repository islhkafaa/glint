#pragma once

#include <filesystem>
#include <string>

namespace glint {

struct FileInfo {
  std::filesystem::path path;
  std::uintmax_t size;
  std::filesystem::file_time_type lastModified;
  std::string extension;

  FileInfo(const std::filesystem::path &p)
      : path(p), size(0), extension(p.extension().string()) {
    if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
      size = std::filesystem::file_size(p);
      lastModified = std::filesystem::last_write_time(p);
    }
  }
};

} // namespace glint
