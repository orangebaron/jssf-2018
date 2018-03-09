#ifndef GENERATE_TXNS_HPP
#define GENERATE_TXNS_HPP

#include <string>
using std::string;

namespace blockchain {
  class FileWrapper {
    FileWrapper(string filename);
    ~FileWrapper();
    struct FileData {
      char *data;
      size_t len;
    };
    int writeToFile(FileData data);
    char* readFromFile(int id);
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
