#include "glint/crawler.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

void printVersion() {
  std::cout << "Glint v0.1.0\n";
  std::cout << "Local Search Engine\n";
}

void printHelp() {
  std::cout << "Usage: glint [options]\n\n";
  std::cout << "Options:\n";
  std::cout << "  --help              Show this help message\n";
  std::cout << "  --version           Show version information\n";
  std::cout << "  --crawl <path>      Crawl directory and index files\n";
}

void crawlDirectory(const std::string &path) {
  std::cout << "Crawling directory: " << path << "\n\n";

  glint::DirectoryCrawler crawler(path);

  size_t fileCount = 0;
  std::uintmax_t totalSize = 0;

  crawler.setProgressCallback([&](const glint::FileInfo &info) {
    fileCount++;
    totalSize += info.size;
    if (fileCount % 100 == 0) {
      std::cout << "\rProcessed: " << fileCount << " files" << std::flush;
    }
  });

  auto results = crawler.crawl();

  std::cout << "\r\nCrawl complete!\n";
  std::cout << "Files found: " << results.size() << "\n";
  std::cout << "Total size: " << std::fixed << std::setprecision(2)
            << (totalSize / 1024.0 / 1024.0) << " MB\n";
}

int main(int argc, char *argv[]) {
  std::vector<std::string> args(argv + 1, argv + argc);

  if (args.empty()) {
    std::cout << "Glint initialized. Use --help for usage information.\n";
    return 0;
  }

  for (size_t i = 0; i < args.size(); ++i) {
    const auto &arg = args[i];

    if (arg == "--help" || arg == "-h") {
      printHelp();
      return 0;
    }
    if (arg == "--version" || arg == "-v") {
      printVersion();
      return 0;
    }
    if (arg == "--crawl") {
      if (i + 1 < args.size()) {
        crawlDirectory(args[i + 1]);
        return 0;
      } else {
        std::cerr << "Error: --crawl requires a directory path\n";
        return 1;
      }
    }
  }

  std::cerr << "Unknown option. Use --help for usage information.\n";
  return 1;
}
