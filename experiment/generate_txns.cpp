#ifndef GENERATE_TXNS_CPP
#define GENERATE_TXNS_CPP

#include "generate_txns.hpp"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <random>
using namespace blockchain;

FileWrapper::FileWrapper(string filename) {
  file = open(filename.c_str(),O_RDWR);
  endPtr = (char*)mmap(NULL,1/*size*/,PROT_READ|PROT_WRITE,MAP_SHARED,file,0);
}
FileWrapper::~FileWrapper() {
  close(file);
}
size_t FileWrapper::write(FileData data) {
  FileData d { endPtr,data.len };
  endPtr += data.len;
  std::copy(data.data,data.data+data.len,d.data);
  idDataMap.push_back(d);
  return idDataMap.size()-1;
}
FileData FileWrapper::read(size_t id) {
  return idDataMap.at(id);
}

TxnOtp* User::randomUnspentOutput(vector<const TxnOtp*>& dontUse) { return NULL; } //TODO
Txn User::randTxn(bool fake) {
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
  for (int _ = 0; _ < len; _++) {
    switch ((txnType)randTxnType(gen)){
    case transfer:
      if (numUnspentOutputs() > inps.size()) {
        inps.push_back(randomUnspentOutput(inps)); //choose random input
        otps.push_back(TxnOtp(randomPubkey(),
          std::uniform_int_distribution<>(1, inps.back()->getAmt() )(gen) )); //output is random amount [1,input.amount]
        otps.push_back(TxnOtp(randomPubkey(),
          inps.back()->getAmt() - otps.back().getAmt() )); //amount is input.amount - last output.amount
        if (!pubkeysSigned[inps.back()->getPerson()]) {
          pubkeysSigned[inps.back()->getPerson()] = true;
          sigs.push_back(Sig(inps.back()->getPerson()));
        }
      }
      break;
    case contCreate:
      contractCreations.push_back(randomContCreation());
      break;
    case contCall:
      if (numUnspentOutputs() > inps.size()) {
        inps.push_back(randomUnspentOutput(inps)); //choose random input
        contractCalls.push_back(ContractCall(
          /*Caller:*/ inps.back()->getPerson(),
          /*Called:*/ randomContKey(),
          /*Args:  */ randIntVector(0,5),
          /*Amt:   */ inps.back()->getAmt(),
          /*MaxGas:*/ randGas(gen)
        ));
        if (!pubkeysSigned[inps.back()->getPerson()]) {
          pubkeysSigned[inps.back()->getPerson()] = true;
          sigs.push_back(Sig(inps.back()->getPerson()));
        }
      }
      break;
    }
  }
  return Txn(inps,otps,contractCreations,contractCalls,sigs);
}
User::User(ExtraChainData& e, FileWrapper& f, int txnsPerSecond, int fakesPerSecond): f(f), e(e) {
  t = thread([](User* user,int txnsPerSecond,int fakesPerSecond) {
    std::chrono::milliseconds ms(1000/(txnsPerSecond+fakesPerSecond));
    for (int counter=0; !user->stop; counter=(counter+1)%(txnsPerSecond+fakesPerSecond)) {
      Txn t = user->randTxn(counter<txnsPerSecond); // do things with it
      std::this_thread::sleep_for(ms);
    }
  },this,txnsPerSecond,fakesPerSecond);
}
User::~User() {
  stop = true;
  t.join();
}

Miner::Miner(FileWrapper& f, bool fake): f(f) {
  t = thread([](Miner* miner,bool fake) {
    while (!miner->stop) {}
  },this,fake);
}
Miner::~Miner() {
  stop = true;
  t.join();
}

#endif
