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
  enum GraphType { DAG,blocks };
  enum ApprovalType { allApprove,rwdPunishment };
  struct ChainType {
    GraphType graphType;
    ApprovalType approvalType;
    bool operator==(ChainType& c) { return c.graphType==graphType && c.approvalType==approvalType; }
  };
  const ChainType Expt1 = ChainType{blocks,allApprove};
  const ChainType Expt2 = ChainType{DAG,allApprove};
  const ChainType Expt3 = ChainType{blocks,rwdPunishment};
  const ChainType Expt4 = ChainType{DAG,rwdPunishment};
  class User {
    thread t;
    bool stop;
    FileWrapper& f;
    ExtraChainData& e;
    ChainType chainType;
    Txn randTxn(bool fake = false);
    size_t numUnspentOutputs();
    TxnOtp* randomUnspentOutput(vector<const TxnOtp*>& dontUse);
    Pubkey randomPubkey();
    Pubkey randomContKey();
    vector<unsigned int> randIntVector(size_t minSize,size_t maxSize);
  public:
    User(ExtraChainData&, FileWrapper&, int txnsPerSecond, ChainType, int fakesPerSecond = 0);
    ~User();
  };
  class Miner;
  typedef vector<Miner*> MinerList;
  class Miner {
    thread t;
    bool stop;
    FileWrapper& f;
    vector<Block> chain;
    vector<Block*> unapprovedBlocks;
    ChainType chainType;
    MinerList& miners;
    vector<Txn>* currentBlock; //TODO initialize and delete
    ExtraChainData currentState;
    ValidsChecked validsChecked;
    bool checkTxn(const Txn&);
  public:
    Miner(FileWrapper&, ChainType, MinerList&, bool fake=false);
    ~Miner();
    void recieveTxn(const Txn&);
    void recieveBlock(Block&);
    bool txnAcceptedYet(int id);
  };
}

#endif
