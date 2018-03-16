#include "txn.hpp"

using namespace blockchain;
#include "common_macros.hpp"
#include <iostream>

Txn::Txn(vector<const TxnOtp*> inps,vector<TxnOtp> otps,vector<ContractCreation> contractCreations,vector<ContractCall> contractCalls,vector<Sig> sigs):
  HasID(), inps(inps),otps(otps),contractCreations(contractCreations),contractCalls(contractCalls),sigs(sigs) {}
Hash Txn::getHashBeforeSig() const {
  return Hash();
}
Hash Txn::getHash() const {
  return getHashBeforeSig();
}
bool Txn::getValid(const ExtraChainData& e, ValidsChecked& v) const {
  validCheckBegin();
  std::cout<<"Checking txn with id "<<id<<std::endl;
  std::cout<<"Checking txn HasID validity..."<<std::endl;
  if (!HasID::getValid(e)) return false;
  std::map<Pubkey,bool> sendersThatDidntSign;
  std::map<const TxnOtp*,bool> inputsUsed;
  TxnAmt sent = 0, recieved = 0;
  std::cout<<"Checking txn inputs validity..."<<std::endl;
  for (auto i: inps) {
    std::cout<<"A"<<std::endl;
    if (!i->getValid(e,v)) return false;
      std::cout<<"B"<<std::endl;
    if (i < &*otps.end() && i >= &*otps.begin()) return false; // check if it's referring to one of the outputs of this txn
      std::cout<<"C"<<std::endl;
    if (inputsUsed[i]) return false;
      std::cout<<"D"<<std::endl;
    try { if (e.spentOutputs.at(i)!=this) return false; } catch(...) {}
      std::cout<<"E"<<std::endl;
    inputsUsed[i] = true;
    sent += i->getAmt();
    sendersThatDidntSign[i->getPerson()] = true;
  }
  std::cout<<"Checking txn outputs validity..."<<std::endl;
  for (auto i: otps) {
    if (!i.getValid(e,v)) return false;
    recieved += i.getAmt();
  }
  std::cout<<"Checking txn contract creations validity..."<<std::endl;
  for (auto i: contractCreations) {
    if (!i.getValid(e,v)) return false;
    sendersThatDidntSign[i.getKey()] = true;
  }
  std::cout<<"Checking txn sigs validity..."<<std::endl;
  if (sent!=recieved) return false;
  for (auto i: sigs) {
    try {
      sendersThatDidntSign.at(i.getPerson());
      sendersThatDidntSign.erase(i.getPerson());
    } catch (...) {
      return false;
    }
  }
  if (sendersThatDidntSign.size() > 0) return false;
  std::cout<<"Returning txn is valid"<<std::endl;
  validCheckEnd();
}
void Txn::apply(ExtraChainData& e) const {
  HasID::apply(e);
  for (auto i: inps) e.spentOutputs[i] = this;
  for (auto i: otps) i.HasID::apply(e);
  for (auto i: contractCreations) i.apply(e);
  for (auto i: contractCalls) {
    e.contractMoney[i.getCalled()] += i.getAmt();
    e.contractOtps[this].push_back(e.contractCodes.find(i.getCalled())->second.run(e,i));
    e.contractMaxIds[i.getCalled()][i.getId()] = &i;
  }
  for (auto i: e.contractOtps[this]) {
    for (auto j: i.moneySpent) e.spentOutputs[&j] = this;
    for (auto j: i.storageChanged) j.apply(e);
  }
}
void Txn::unapply(ExtraChainData& e) const {
  HasID::unapply(e);
  for (auto i: inps) if (e.spentOutputs[i] == this) e.spentOutputs[i] = nullptr;
  for (auto i: otps) i.HasID::unapply(e);
  for (auto i: contractCreations) i.unapply(e);
  for (auto i: contractCalls) e.contractMaxIds[i.getCalled()].erase(i.getId());
  for (auto i: e.contractOtps[this]) {
    for (auto j: i.moneySpent) if (e.spentOutputs[&j] == this) e.spentOutputs[&j] = nullptr;
    for (auto j: i.storageChanged) j.unapply(e);
  }
}
WorkType Txn::getWork(WorkCalculated& w) const {
  getWorkBegin();
  work += 3*inps.size();
  for (auto i: otps) work += i.getWork(w);
  for (auto i: contractCreations) work += i.getWork(w);
  for (auto i: contractCalls) work += i.getWork(w);
  for (auto i: sigs) work += i.getWork(w);
  return work;
}
const vector<const TxnOtp*>& Txn::getInps() const { return inps; }
const vector<TxnOtp>& Txn::getOtps() const { return otps; }
const vector<ContractCreation>& Txn::getContractCreations() const { return contractCreations; }
const vector<ContractCall>& Txn::getContractCalls() const { return contractCalls; }
const vector<Sig>& Txn::getSigs() const { return sigs; }

Block::Block(): HasID() {}
Block::Block(vector<Txn> txns,vector<Block*> approved): HasID(),txns(txns),approved(approved) {}
Hash Block::getHash() const {
  return getHashBeforeSig();
}
bool Block::getValid(const ExtraChainData& e, ValidsChecked& v) const {
  validCheckBegin();
  std::cout<<"Checking block HasID validity..."<<std::endl;
  if (!HasID::getValid(e)) return false;
  std::cout<<"Checking block txns validity..."<<std::endl;
  for (auto i: txns) if (!i.getValid(e,v)) return false;
  std::cout<<"Checking block approveds validity..."<<std::endl;
  for (auto i: approved) if (!i->getValid(e,v)) return false;
  std::cout<<"Returning block is valid"<<std::endl;
  // check nonce
  validCheckEnd();
}
void Block::apply(ExtraChainData& e) const {
  HasID::apply(e);
  for (auto i: txns) i.apply(e);
}
void Block::unapply(ExtraChainData& e) const {
  HasID::unapply(e);
  for (auto i: txns) i.unapply(e);
}
WorkType Block::getWork(WorkCalculated& w) const {
  getWorkBegin();
  for (auto i: txns) work += i.getWork(w);
  return work;
}
WorkType Block::getSumWork(WorkCalculated& w) const {
  WorkType work = this->getWork(w);
  for (auto i: approved) work += i->getWork(w);
  return work;
}
const vector<Txn>& Block::getTxns() const {
  return txns;
}
vector<Block*> Block::getApproved() {
  return vector<Block*>(approved);
}
