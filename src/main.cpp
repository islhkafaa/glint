#include "glint/crawler.h"
#include "glint/database.h"
#include "glint/text_extractor.h"
#include "glint/tokenizer.h"

#include <iomanip>
#include <iostream>
#include <set>
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
  std::cout << "  --db <path>         Database file path (default: glint.db)\n";
  std::cout << "  --verbose           Show detailed processing information\n";
}

void crawlDirectory(const std::string &path, const std::string &dbPath,
                    bool verbose) {
  std::cout << "Crawling directory: " << path << "\n";
  std::cout << "Database: " << dbPath << "\n\n";

  try {
    glint::Database db(dbPath);
    db.initialize();

    glint::DirectoryCrawler crawler(path);

    size_t fileCount = 0;
    std::uintmax_t totalSize = 0;
    size_t totalTokens = 0;
    std::set<std::string> uniqueTokens;

    crawler.setProgressCallback([&](const glint::FileInfo &info) {
      fileCount++;
      totalSize += info.size;

      std::string text = glint::TextExtractor::extractText(info.path);
      if (!text.empty()) {
        auto tokens = glint::Tokenizer::tokenize(text);
        totalTokens += tokens.size();

        for (const auto &token : tokens) {
          uniqueTokens.insert(token);
        }

        if (verbose && !tokens.empty()) {
          std::cout << "\n"
                    << info.path.filename().string() << ": " << tokens.size()
                    << " tokens\n";
        }
      }

      if (fileCount % 100 == 0) {
        std::cout << "\rProcessed: " << fileCount << " files" << std::flush;
      }
    });

    auto results = crawler.crawl();

    std::cout << "\r\nStoring files in database...\n";
    db.insertFiles(results);

    std::cout << "\nCrawl complete!\n";
    std::cout << "Files found: " << results.size() << "\n";
    std::cout << "Files in database: " << db.getFileCount() << "\n";
    std::cout << "Total size: " << std::fixed << std::setprecision(2)
              << (totalSize / 1024.0 / 1024.0) << " MB\n";
    std::cout << "Total tokens: " << totalTokens << "\n";
    std::cout << "Unique tokens: " << uniqueTokens.size() << "\n";
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
  }
}

int main(int argc, char *argv[]) {
  std::vector<std::string> args(argv + 1, argv + argc);

  if (args.empty()) {
    std::cout << "Glint initialized. Use --help for usage information.\n";
    return 0;
  }

  std::string crawlPath;
  std::string dbPath = "glint.db";
  bool verbose = false;

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
        crawlPath = args[i + 1];
        ++i;
      } else {
        std::cerr << "Error: --crawl requires a directory path\n";
        return 1;
      }
    }
    if (arg == "--db") {
      if (i + 1 < args.size()) {
        dbPath = args[i + 1];
        ++i;
      } else {
        std::cerr << "Error: --db requires a file path\n";
        return 1;
      }
    }
    if (arg == "--verbose") {
      verbose = true;
    }
  }

  if (!crawlPath.empty()) {
    crawlDirectory(crawlPath, dbPath, verbose);
    return 0;
  }

  std::cerr << "Unknown option. Use --help for usage information.\n";
  return 1;
}
