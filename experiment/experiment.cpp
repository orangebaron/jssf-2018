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
  vector<PunRwdTxn> punRwdTxns;
  map<Pubkey,bool> pubkeysSigned;
  int numContractCalls = 0;

  int len = std::uniform_int_distribution<>(1,10)(gen);
  std::uniform_int_distribution<> randTxnType(0,2);
  std::uniform_int_distribution<> randGas(10,100);
  std::uniform_int_distribution<> randId(0,INT_MAX);
  for (int i = 0; i < len; i++) {
    Pubkey sig;
    bool sign = false;
    switch ((txnType)randTxnType(gen)){
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
        //contractCalls.push_back(
        //  /*Caller:*/ sig,
        //  /*Called:*/ m.randomContKey(),
        //  /*Args:  */ randIntVector(0,5),
        //  /*Amt:   */ inps.back()->getAmt(),
        //  /*MaxGas:*/ randGas(gen),
        //  /*ID:    */ randId(gen)
        //));
        numContractCalls++;
        if (chainType.approvalType==rwdPunishment) punRwdTxns.push_back(PunRwdTxn(*(contractCalls.end()-1),RunOtp()));
      }
      break;
    }
    if (sign&&!pubkeysSigned[sig]) {
      pubkeysSigned[sig] = true;
      sigs.push_back(Sig(sig));
    }
  }
  return Txn(inps,otps,contractCreations,contractCalls,sigs,punRwdTxns,numContractCalls,fake);
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
      auto id=t.id;

      threads.push_back(thread([miners,id,this]() {
        //std::cout<<"waiting for "<<id<<std::endl;
        std::chrono::time_point<std::chrono::system_clock> x = std::chrono::system_clock::now();
        for (unsigned int numAccepted = 0;numAccepted<miners.size();){
          numAccepted=0;
          //if (stop) return;
          std::this_thread::sleep_for(std::chrono::microseconds(10));
          for (auto m: miners) if (m->txnAcceptedYet(id)) numAccepted++;
        }
        auto y = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now()-x).count();
        //std::cout<<"#ACCEPTED "<<id<<" IN "<<y<<std::endl;
        numSuccessfulTxns++;
        totalTxnTime+=y;
      }));

      m.recieveTxn(t);
      for (auto miner:miners) if (miner!=&m) m.sendTxn(t,*miner); // do things with it
      std::this_thread::sleep_for(ms);
    }
  }));
}
User::~User() {
  stop = true;
  for (auto i=threads.begin();i<threads.end();i++) if (i->joinable()) i->join();
}

Miner::Miner(ChainType chainType, MinerList& miners, bool fake):
  chainType(chainType),miners(miners) {
  stop = false;
  TxnOtp* otp = new TxnOtp(Pubkey(),10000,&v);
  currentState.IDs[otp->id] = otp;
  otp->id=0;
  chain.push_back(Block({Txn(
    {otp},
    {TxnOtp(Pubkey(1),10000)},
    {},
    {},
    {Sig(Pubkey())},
    {}
  )},{}));
  chain[0].id=1;
  ((vector<Txn>&)chain[0].getTxns())[0].id=2;
  ((vector<TxnOtp>&)chain[0].getTxns()[0].getOtps())[0].id=3;
  unspentOutputs.push_back((TxnOtp*)&chain[0].getTxns()[0].getOtps()[0]);
  if (!chain[0].getValid(currentState,v)) std::cout<<"FIRST THING IS INVALID";
  chain[0].apply(currentState);
  if (chainType.graphType==blocks) {
    threads.push_back(thread([this,miners,fake]() {
      thread* t = &threads[threads.size()-1];
      std::random_device r;
      std::default_random_engine gen(r());
      std::uniform_int_distribution<> minedNumGen(INT_MIN,INT_MAX);
      while (!stop) {
        if (minedNumGen(gen)%(1000) == 0) { //10 second block time
          Block b(currentBlock,{&*(chain.end()-1)});
          recieveBlock(b);
          for (auto m: miners) if (m!=this) sendBlock(b,*m);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10*miners.size()));
      }
      deletableThreads.push_back(t);
    }));
  }
}
Miner::~Miner() {
  stop = true;
  for (auto i=threads.begin();i<threads.end();i++) if (i->joinable()) i->join();
}
void Miner::sendTxn(const Txn& txn0,Miner& miner) {
  std::cout<<"BAD"<<std::endl;
  vector<const TxnOtp*> inps(txn0.getInps());
  for (size_t j = 0;j<inps.size();j++)
    inps[j] = (TxnOtp*)miner.currentState.IDs[inps[j]->id];
  Txn& txn = *new Txn(
    inps,
    txn0.getOtps(),
    txn0.getContractCreations(),
    txn0.getContractCalls(),
    txn0.getSigs(),
    txn0.getPunRwdTxns(),
    txn0.numContractCalls,
    txn0.fake
  );
  txn.id = txn0.id;
  std::cout<<"about to send txn "<<txn.id<<std::endl;
  miner.recieveTxn(txn0);
}
void Miner::recieveTxn(const Txn& txn) {
  /*threads.push_back(*///thread([txn,this]() {
    thread* t = &threads[threads.size()-1];
    networkWait();
    /*if (txn.isFake()) return;
    std::this_thread::sleep_for(std::chrono::milliseconds(txn.getOtps().size()*2));
    std::this_thread::sleep_for(std::chrono::milliseconds(txn.getContractCreations().size()*5));
    if (chainType.approvalType==allApprove)
      std::this_thread::sleep_for(std::chrono::milliseconds(txn.getContractCalls().size()*10));
    std::this_thread::sleep_for(std::chrono::milliseconds(txn.getPunRwdTxns().size()*2));
    *///if (!txn.getValid(currentState,v)) return;
    //std::cout<<"miner accepted txn "<<txn.id<<std::endl;
    if (chainType.graphType == blocks) {
      currentBlock.push_back(txn);
    } else {
      std::random_device r;
      std::default_random_engine gen(r());
      Block b({txn},{
        &chain[std::uniform_int_distribution<>(0,chain.size()-1)(gen)],
        &chain[std::uniform_int_distribution<>(0,chain.size()-1)(gen)]
      });
      recieveBlock(b);
    }
    deletableThreads.push_back(t);
  //}).detach();//);
}
void Miner::sendBlock(Block& b,Miner& miner) {
  std::cout<<"BAD"<<std::endl;
  vector<Txn> txns(b.getTxns());
  vector<Block*> approved(b.getApproved());
  for (auto i = b.getTxns().begin(); i<b.getTxns().end(); i++) {
    auto& t = *i;
    vector<const TxnOtp*> inps(t.getInps());
    for (size_t j = 0;j<inps.size();j++)
      inps[j] = (TxnOtp*)miner.currentState.IDs[inps[j]->id];
    txns.push_back(Txn(
      inps,
      t.getOtps(),
      t.getContractCreations(),
      t.getContractCalls(),
      t.getSigs(),
      t.getPunRwdTxns(),
      t.numContractCalls,
      t.fake
    ));
  }
  if (approved.size()>0) for (size_t i = 0;i<approved.size();i++)
    approved[i] = (Block*)miner.currentState.IDs[approved[i]->id];

  Block c(txns,approved);
  c.id = b.id;
  miner.recieveBlock(b);
}
void Miner::recieveBlock(Block& c) {
  /*threads.push_back(*///thread([this](Block& c) {
    //thread* t = &threads[threads.size()-1];
    networkWait();
    //std::cout<<"--------------------------"<<std::endl;
    //std::cout<<"Checking block..."<<std::endl;
    //if (!c.getValid(currentState,v)) return;
    chain.push_back(c);
    Block& b = chain[chain.size()-1];
    //std::cout<<"Block accepted"<<std::endl;
    for (auto i=unapprovedBlocks.begin();i<unapprovedBlocks.end();i++)
      for (auto j: b.getApproved())
        if (*i==j) { unapprovedBlocks.erase(i); i--; }
    c.apply(currentState);
    for (auto t: b.getTxns()) {
      for (auto i: t.getInps()) for (auto j=unspentOutputs.begin();j<unspentOutputs.end();j++) if (i==*j) unspentOutputs.erase(j);
      for (auto i: t.getOtps()) unspentOutputs.push_back(&i);
    }
    //deletableThreads.push_back(t);
  //},std::ref(c)).detach();//);
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
    currentState.IDs.at(id);
    return true;
  } catch (...) {
    return false;
  }
}
size_t Miner::getNumUnspentOutputs() {
for (auto i=unspentOutputs.begin();i<unspentOutputs.end();i++)
  for (auto j=i+1;j<unspentOutputs.end();j++)
    if (i==j) {unspentOutputs.erase(i); return getNumUnspentOutputs();}
  return unspentOutputs.size();
}
TxnOtp* Miner::randomUnspentOutput(vector<const TxnOtp*>& dontUse) {
  std::random_device r;
  std::default_random_engine gen(r());
  //if (dontUse.size()>unspentOutputs.size()/4) {
    vector<TxnOtp*> canUse;
    for (auto i: unspentOutputs) { bool use = true; for (auto j: dontUse) if (i==j) use = false; if (use) canUse.push_back(i); }
    if (canUse.size()==0) {
      //bool duplicates = false;
      //for (auto i=unspentOutputs.begin();i<unspentOutputs.end();i++)
      //for (auto j=i+1;j<unspentOutputs.end();j++) if (i==j) duplicates = true;
      //std::cout<<"BADD"<<unspentOutputs.size()<<" "<<dontUse.size()<<
      //(duplicates?"DUPLICATES":"NO DUPL")<<std::endl;
      /*std::cout<<"UNSPENT:"<<std::endl;
      for (auto mm: unspentOutputs)std::cout<<mm<<std::endl;
      std::cout<<"DONTUSE:"<<std::endl;
      for (auto mm: dontUse)std::cout<<mm<<std::endl;*/
      return unspentOutputs[0];
    }
    return canUse[std::uniform_int_distribution<>(0,canUse.size()-1)(gen)];
  /*} else {
    std::uniform_int_distribution<> otpSelector(0,unspentOutputs.size()-1);
    for (;;) {
      TxnOtp* x = unspentOutputs[otpSelector(gen)];
      bool isIn = false;
      for (auto i: dontUse) if (i==x) isIn = true;
      std::cout<<"looping?"<<std::endl;
      if (!isIn) return x;
    }
  //}*/
}

#endif
