#ifndef EXPERIMENT_CPP
#define EXPERIMENT_CPP

#include "experiment.hpp"
#include <algorithm>
#include <chrono>
#include <random>
#include <climits>
using namespace blockchain;

#define networkWait() std::this_thread::sleep_for(std::chrono::milliseconds(2));

TxnOtp* User::randomUnspentOutput(vector<const TxnOtp*>& dontUse) { return nullptr; } //TODO
Pubkey User::randomPubkey() {
  std::random_device r;
  std::default_random_engine gen(r());
  return Pubkey(std::uniform_int_distribution<>(INT_MIN,INT_MAX)(gen));
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
  for (int _ = 0; _ < len; _++) {
    Pubkey sig;
    switch ((txnType)randTxnType(gen)){
    case transfer:
      if (numUnspentOutputs() > inps.size()) {
        inps.push_back(randomUnspentOutput(inps)); //choose random input
        otps.push_back(TxnOtp(randomPubkey(),
          std::uniform_int_distribution<>(1, inps.back()->getAmt() )(gen) )); //output is random amount [1,input.amount]
        otps.push_back(TxnOtp(randomPubkey(),
          inps.back()->getAmt() - otps.back().getAmt() )); //amount is input.amount - last output.amount
        sig = inps.back()->getPerson();
        if (!pubkeysSigned[inps.back()->getPerson()]) {
          pubkeysSigned[inps.back()->getPerson()] = true;
          sigs.push_back(Sig(inps.back()->getPerson()));
        }
      }
      break;
    case contCreate:
      sig = randomPubkey();
      contractCreations.push_back(ContractCreation(
        CodeMemory(randIntVector(10,50)),
        sig
      ));
      break;
    case contCall:
      if (numUnspentOutputs() > inps.size()) {
        inps.push_back(randomUnspentOutput(inps)); //choose random input
        sig = inps.back()->getPerson();
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
    if (!pubkeysSigned[sig]) {
      pubkeysSigned[sig] = true;
      sigs.push_back(Sig(sig));
    }
  }
  return Txn(inps,otps,contractCreations,contractCalls,sigs);
}
User::User(MinerList& miners, int txnsPerSecond, ChainType chainType, int fakesPerSecond):
  chainType(chainType), miners(miners) {
  t = thread([this,miners,txnsPerSecond,fakesPerSecond]() {
    std::chrono::milliseconds ms(1000/(txnsPerSecond+fakesPerSecond));

    std::random_device r;
    std::default_random_engine gen(r());
    std::uniform_int_distribution<> minerNumGen(1,10);
    for (int counter=0; !stop; counter=(counter+1)%(txnsPerSecond+fakesPerSecond)) {
      Miner& m = *miners[minerNumGen(gen)];
      m.recieveTxn(randTxn(m,counter<txnsPerSecond)); // do things with it
      std::this_thread::sleep_for(ms);
    }
  });
}
User::~User() {
  stop = true;
  t.join();
}

Miner::Miner(ChainType chainType, MinerList& miners, bool fake):
  chainType(chainType),miners(miners) {
  t = thread([this,fake]() {
    std::random_device r;
    std::default_random_engine gen(r());
    std::uniform_int_distribution<> minedNumGen(INT_MIN,INT_MAX);
    while (!stop) {
      if (minedNumGen(gen)%/*difficulty*/65 == 0) {
        Block b(*currentBlock,{&*(chain.end()-1)});
        recieveBlock(b,currentState);
      }
    }
  });
}
Miner::~Miner() {
  stop = true;
  t.join();
}
void Miner::recieveTxn(const Txn& t) {
  thread([t,this]() {
    networkWait();
    if (!t.getValid(currentState,validsChecked)) return;
    if (chainType.graphType == blocks) {
      currentBlock->push_back(t);
      for (auto i: miners) if (i!=this) i->recieveTxn(t);
      t.apply(currentState);
    } else {
      std::random_device r;
      std::default_random_engine gen(r());
      Block b({t},{
        unapprovedBlocks[std::uniform_int_distribution<>(0,unapprovedBlocks.size())(gen)],
        &chain[std::uniform_int_distribution<>(0,chain.size())(gen)]
      });
      recieveBlock(b,currentState);
    }
  });
}
void Miner::recieveBlock(Block& b,const ExtraChainData& e) {
  if (!b.getValid(currentState,validsChecked)) return;
  vector<Txn> txns(b.getTxns());
  vector<Block*> approved(b.getApproved());
  for (auto i = b.getTxns().begin(); i!=b.getTxns().end(); i++) {
    auto& t = *i;
    vector<const TxnOtp*> inps(t.getInps());
    for (size_t j = 0;j<approved.size();j++)
      inps[j] = (TxnOtp*)currentState.IDsReverse[e.IDs.at(inps[j])];
    txns.push_back(Txn(
      inps,
      t.getOtps(),
      t.getContractCreations(),
      t.getContractCalls(),
      t.getSigs()
    ));
  }
  for (size_t i = 0;i<approved.size();i++)
    approved[i] = (Block*)currentState.IDsReverse[e.IDs.at(approved[i])];

  Block c(txns,approved);
  for (auto i=unapprovedBlocks.begin();i!=unapprovedBlocks.end();i++)
    for (auto j: c.getApproved())
      if (*i==j) { unapprovedBlocks.erase(i); i--; }
  b.apply(currentState);
  for (auto m: miners)
    thread([m,this](Block& c) {
      networkWait();
      m->recieveBlock(c,currentState);
    },c);
}
Pubkey Miner::randomContKey() {
  std::random_device r;
  std::default_random_engine gen(r());
  return (std::next(currentState.contractMoney.begin(),
    std::uniform_int_distribution<>(0,currentState.contractMoney.size())(gen))
    ->first
  );
}

#endif
