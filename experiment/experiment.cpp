#ifndef EXPERIMENT_CPP
#define EXPERIMENT_CPP

#include "experiment.hpp"
#include <algorithm>
#include <chrono>
#include <random>
#include <climits>
#include <iostream>
using namespace blockchain;

#define networkWait() std::this_thread::sleep_for(std::chrono::milliseconds(10));

Pubkey User::randomPubkey() {
  std::random_device r;
  std::default_random_engine gen(r());
  return Pubkey(std::uniform_int_distribution<>(INT_MIN,INT_MAX)(gen));
}
vector<unsigned int> User::randIntVector(size_t minSize,size_t maxSize) {
  std::random_device r;
  std::default_random_engine gen(r());
  vector<unsigned int> v;
  std::uniform_int_distribution<> randInt(INT_MIN,INT_MAX);
  for (int i = std::uniform_int_distribution<>(minSize,maxSize)(gen);i>0;i--) {
    v.push_back(randInt(gen));
  }
  return v;
}
Txn User::randTxn(Miner& m, bool fake) {
  std::random_device r;
  std::default_random_engine gen(r());

  enum txnType:int { transfer,contCreate,contCall };

  vector<const TxnOtp*> inps;
  vector<TxnOtp> otps;
  vector<ContractCreation> contractCreations;
  vector<ContractCall> contractCalls;
  vector<Sig> sigs;
  map<Pubkey,bool> pubkeysSigned;

  int len = std::uniform_int_distribution<>(1,10)(gen);
  std::uniform_int_distribution<> randTxnType(1,3);
  std::uniform_int_distribution<> randGas(10,100);
  std::uniform_int_distribution<> randId(0,INT_MAX);
  for (int i = 0; i < len; i++) {
    Pubkey sig;
    bool sign = false;
    switch ((txnType)0){//randTxnType(gen)){
    case transfer:
      if (m.getNumUnspentOutputs() > inps.size()) {
        inps.push_back(m.randomUnspentOutput(inps)); //choose random input
        otps.push_back(TxnOtp(randomPubkey(),
          std::uniform_int_distribution<>(1, inps.back()->getAmt() )(gen) )); //output is random amount [1,input.amount]
        otps.push_back(TxnOtp(randomPubkey(),
          inps.back()->getAmt() - otps.back().getAmt() )); //amount is input.amount - last output.amount
        sig = inps.back()->getPerson();
        sign = true;
      }
      break;
    case contCreate:
      sig = randomPubkey();
      contractCreations.push_back(ContractCreation(
        CodeMemory(randIntVector(10,50)),
        sig
      ));
      sign = true;
      break;
    case contCall:
      if (m.getNumUnspentOutputs() > inps.size()) {
        inps.push_back(m.randomUnspentOutput(inps)); //choose random input
        sig = inps.back()->getPerson();
        sign = true;
        contractCalls.push_back(ContractCall(
          /*Caller:*/ sig,
          /*Called:*/ m.randomContKey(),
          /*Args:  */ randIntVector(0,5),
          /*Amt:   */ inps.back()->getAmt(),
          /*MaxGas:*/ randGas(gen),
          /*ID:    */ randId(gen)
        ));
      }
      break;
    }
    if (sign&&!pubkeysSigned[sig]) {
      pubkeysSigned[sig] = true;
      sigs.push_back(Sig(sig));
    }
  }
  return Txn(inps,otps,contractCreations,contractCalls,sigs);
}
User::User(MinerList& miners, int txnsPerSecond, ChainType chainType, int fakesPerSecond):
  chainType(chainType), miners(miners) {
  stop = false;
  threads.push_back(thread([this,miners,txnsPerSecond,fakesPerSecond]() {
    std::chrono::milliseconds ms(1000/(txnsPerSecond+fakesPerSecond));

    std::random_device r;
    std::default_random_engine gen(r());
    std::uniform_int_distribution<> minerNumGen(0,miners.size()-1);
    for (int counter=0; !stop; counter=(counter+1)%(txnsPerSecond+fakesPerSecond)) {
      size_t loc = minerNumGen(gen);
      Miner& m = *miners[loc];
      Txn t = randTxn(m,counter<txnsPerSecond);
      m.recieveTxn(t,loc,loc); // do things with it
      auto id=t.id;

      threads.push_back(thread([miners,id]() {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        int numAccepted = 0;
        for (auto m: miners) if (m->txnAcceptedYet(id)) numAccepted++;
      }));

      std::this_thread::sleep_for(ms);
    }
  }));
}
User::~User() {
  stop = true;
  for (auto i=threads.begin();i<threads.end();i++) if (i->joinable()) i->join();
}

Miner::Miner(ChainType chainType, MinerList& miners, size_t listLoc, bool fake):
  chainType(chainType),miners(miners) {
  stop = false;
  TxnOtp* otp = new TxnOtp(Pubkey(),10000,&v);
  chain.push_back(Block({Txn(
    {otp},
    {TxnOtp(Pubkey(1),10000,&v)},
    {},
    {},
    {Sig(Pubkey())}
  )},{}));
  unspentOutputs.push_back((TxnOtp*)&chain[0].getTxns()[0].getOtps()[0]);
  chain[0].apply(currentState);
  threads.push_back(thread([this,fake,listLoc]() {
    std::random_device r;
    std::default_random_engine gen(r());
    std::uniform_int_distribution<> minedNumGen(INT_MIN,INT_MAX);
    while (!stop) {
      if (minedNumGen(gen)%(200) == 0) { //10 second block time w/ 60 miners
        Block b(currentBlock,{&*(chain.end()-1)});
        recieveBlock(b,listLoc,listLoc);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }));
}
Miner::~Miner() {
  stop = true;
  for (auto i=threads.begin();i<threads.end();i++) if (i->joinable()) i->join();
}
void Miner::recieveTxn(const Txn& txn0,const ExtraChainData& e,size_t listLoc,size_t startLoc) {
  if (miners.size()>1) {
    if ((listLoc+1)%miners.size()==startLoc) return;
  }
  vector<const TxnOtp*> inps(txn0.getInps());
  for (size_t j = 0;j<inps.size();j++)
    inps[j] = (TxnOtp*)currentState.IDsReverse[e.IDs.at(inps[j])];
  Txn& txn = *new Txn(
    inps,
    txn0.getOtps(),
    txn0.getContractCreations(),
    txn0.getContractCalls(),
    txn0.getSigs()
  );
  recieveTxn(txn,listLoc,startLoc);
}
void Miner::recieveTxn(const Txn& txn,size_t listLoc,size_t startLoc) {
  if (miners.size()>1) {
    listLoc++;
    listLoc %= miners.size();
    if (listLoc==startLoc) return;
  }
  threads.push_back(thread([txn,listLoc,startLoc,this]() {
    networkWait();
    if (!txn.getValid(currentState,v)) return;
    if (chainType.graphType == blocks) {
      currentBlock.push_back(txn);
      if (miners.size()>1)
        miners[listLoc]->recieveTxn(txn,currentState,listLoc,startLoc);
    } else {
      std::random_device r;
      std::default_random_engine gen(r());
      Block b({txn},{
        unapprovedBlocks[std::uniform_int_distribution<>(0,unapprovedBlocks.size())(gen)],
        &chain[std::uniform_int_distribution<>(0,chain.size())(gen)]
      });
      recieveBlock(b,listLoc,listLoc);
    }
  }));
}
void Miner::recieveBlock(Block& b,const ExtraChainData& e,size_t listLoc,size_t startLoc) {
  if (miners.size()>1) {
    if ((listLoc+1)%miners.size()==startLoc) return;
  }
  vector<Txn> txns(b.getTxns());
  vector<Block*> approved(b.getApproved());
  for (auto i = b.getTxns().begin(); i<b.getTxns().end(); i++) {
    auto& t = *i;
    vector<const TxnOtp*> inps(t.getInps());
    for (size_t j = 0;j<inps.size();j++)
      inps[j] = (TxnOtp*)currentState.IDsReverse[e.IDs.at(inps[j])];
    txns.push_back(Txn(
      inps,
      t.getOtps(),
      t.getContractCreations(),
      t.getContractCalls(),
      t.getSigs()
    ));
  }
  if (approved.size()>0) for (size_t i = 0;i<approved.size();i++)
    approved[i] = (Block*)currentState.IDsReverse[e.IDs.at(approved[i])];

  Block c(txns,approved);
  recieveBlock(c,listLoc,startLoc);
}
void Miner::recieveBlock(Block& c,size_t listLoc,size_t startLoc) {
  if (miners.size()>1) {
    listLoc++;
    listLoc %= miners.size();
    if (listLoc==startLoc) return;
  }
  if (!c.getValid(currentState,v)) return;
  for (auto i=unapprovedBlocks.begin();i<unapprovedBlocks.end();i++)
    for (auto j: c.getApproved())
      if (*i==j) { unapprovedBlocks.erase(i); i--; }
  c.apply(currentState);
  for (auto t: c.getTxns()) {
    for (auto i: t.getInps()) for (auto j=unspentOutputs.begin();j<unspentOutputs.end();j++) if (i==*j) unspentOutputs.erase(j);
    for (auto i: t.getOtps()) unspentOutputs.push_back(&i);
  }
  if (miners.size()>1)
    threads.push_back(thread([this,listLoc,startLoc](Block& c) {
      networkWait();
      miners[listLoc]->recieveBlock(c,currentState,listLoc,startLoc);
    },std::ref(c)));
}
Pubkey Miner::randomContKey() {
  std::random_device r;
  std::default_random_engine gen(r());
  Pubkey p = std::next(currentState.contractMoney.begin(),
    std::uniform_int_distribution<>(0,currentState.contractMoney.size())(gen))
    ->first;
  return p;
}
bool Miner::txnAcceptedYet(long long id) {
  try {
    currentState.IDsReverse.at(id);
    return true;
  } catch (...) {
    return false;
  }
}
size_t Miner::getNumUnspentOutputs() { return unspentOutputs.size(); }
TxnOtp* Miner::randomUnspentOutput(vector<const TxnOtp*>& dontUse) {
  std::random_device r;
  std::default_random_engine gen(r());
  if (dontUse.size()>unspentOutputs.size()/4) {
    vector<TxnOtp*> canUse;
    for (auto i: unspentOutputs) { bool use = true; for (auto j: dontUse) if (i==j) use = false; if (use) canUse.push_back(i); }
    return canUse[std::uniform_int_distribution<>(0,canUse.size())(gen)];
  } else {
    std::uniform_int_distribution<> otpSelector(0,unspentOutputs.size()-1);
    for (;;) {
      TxnOtp* x = unspentOutputs[otpSelector(gen)];
      bool isIn = false;
      for (auto i: dontUse) if (i==x) isIn = true;
      if (!isIn) return x;
    }
  }
}

#endif
