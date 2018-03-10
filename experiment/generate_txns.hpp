#ifndef GENERATE_TXNS_HPP
#define GENERATE_TXNS_HPP

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <thread>
using std::thread;
#include "../chain/txn.hpp"

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
    thread t;
    bool stop;
    FileWrapper& f;
    Txn randTxn(bool fake = false);
    size_t numUnspentOutputs();
    TxnOtp* randomUnspentOutput(vector<const TxnOtp*>& dontUse);
    Pubkey randomPubkey();
    ContractCreation randomContCreation();
    ContractCall randomContCall();
  public:
    User(FileWrapper&, int txnsPerSecond, int fakesPerSecond = 0);
    ~User();
  };
  class Miner {
    thread t;
    bool stop;
    FileWrapper& f;
  public:
    Miner(FileWrapper&,bool fake=false);
    ~Miner();
    bool blockAcceptedYet(int id);
  };
}

#endif
