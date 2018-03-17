#ifndef EXPERIMENT_HPP
#define EXPERIMENT_HPP

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <thread>
using std::thread;
#include <atomic>
#include "../chain/txn.hpp"

namespace blockchain {
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
  class Miner;
  typedef vector<Miner*> MinerList;
  class User {
    vector<thread> threads;
    ChainType chainType;
    MinerList& miners;
    vector<long long> ids;
    vector<std::chrono::time_point<std::chrono::system_clock>> blockTimes; //TODO: was planning on getting this done hours ago but whoops
    ValidsChecked v;
    Txn randTxn(Miner&,bool fake = false);
    Pubkey randomPubkey();
    vector<unsigned int> randIntVector(size_t minSize,size_t maxSize);
  public:
    int numSuccessfulTxns = 0;
    long long totalTxnTime = 0;
    std::atomic_bool stop;
    User(MinerList&,int txnsPerSecond, ChainType, int fakesPerSecond = 0);
    ~User();
  };
  class Miner {
    vector<thread> threads;
    std::atomic_bool stop;
    vector<Block> chain;
    vector<Block*> unapprovedBlocks;
    ChainType chainType;
    MinerList& miners;
    vector<Txn> currentBlock;
    vector<TxnOtp*> unspentOutputs;
    ValidsChecked v;
    bool checkTxn(const Txn&);
  public:
    vector<thread*> deletableThreads;
    ExtraChainData currentState;
    Miner(ChainType, MinerList&, bool fake=false);
    ~Miner();
    void recieveTxn(const Txn&);
    void sendTxn(const Txn&,Miner&);
    void recieveBlock(Block&);
    void sendBlock(Block&,Miner&);
    bool txnAcceptedYet(long long id);
    Pubkey randomContKey();
    size_t getNumUnspentOutputs();
    TxnOtp* randomUnspentOutput(vector<const TxnOtp*>& dontUse);
  };
}

#endif
