#ifndef GENERATE_TXNS_HPP
#define GENERATE_TXNS_HPP

#include <string>
using std::string;
#include <vector>
using std::vector;

namespace blockchain {
  struct FileData {
    char *data;
    size_t len;
  };
  class FileWrapper {
    int file;
    char *endPtr;
    vector<FileData> idDataMap;
  public:
    FileWrapper(string filename);
    ~FileWrapper();
    size_t write(FileData);
    FileData read(size_t id);
  };
  class User {
    User(FileWrapper&, int txnsPerSecond, int fakesPerSecond);
    ~User();
  };
  class Miner {
    Miner(FileWrapper&,bool fake=false);
    ~Miner();
    bool blockAcceptedYet(int id);
  };
}

#endif
