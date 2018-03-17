#include "txn.hpp"

using namespace blockchain;
#include "common_macros.hpp"
#include <iostream>

#define output(x) std::cout<<x<<std::endl;

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
  output("Checking txn with id "<<id);
  output("Checking txn HasID validity...");
  if (!HasID::getValid(this,e)) return false;
  std::map<Pubkey,bool> sendersThatDidntSign;
  std::map<const TxnOtp*,bool> inputsUsed;
  TxnAmt sent = 0, recieved = 0;
  output("Checking txn inputs validity...");
  for (auto i: inps) {
    output(i);
    if (!i->getValid(e,v)) return false;
    if (i < &*otps.end() && i >= &*otps.begin()) return false; // check if it's referring to one of the outputs of this txn
    if (inputsUsed[i]) return false;
    try { if (e.spentOutputs.at(i)!=this) return false; } catch(...) {}
    inputsUsed[i] = true;
    sent += i->getAmt();
    sendersThatDidntSign[i->getPerson()] = true;
  }
  output("Checking txn outputs validity...");
  for (auto i: otps) {
    if (!i.getValid(e,v)) return false;
    recieved += i.getAmt();
  }
  output("Checking txn contract creations validity...");
  for (auto i: contractCreations) {
    if (!i.getValid(e,v)) return false;
    sendersThatDidntSign[i.getKey()] = true;
  }
  output("Checking txn sigs validity...");
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
  output("Returning txn is valid");
  validCheckEnd();
}
void Txn::apply(ExtraChainData& e) const {
  HasID::apply(this,e);
  for (auto i: inps) e.spentOutputs[i] = this;
  for (auto i: otps) i.HasID::apply(this,e);
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
  HasID::unapply(this,e);
  for (auto i: inps) if (e.spentOutputs[i] == this) e.spentOutputs[i] = nullptr;
  for (auto i: otps) i.HasID::unapply(this,e);
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
  output("Checking block HasID validity...");
  if (!HasID::getValid(this,e)) return false;
  output("Checking block txns validity...");
  for (auto i: txns) if (!i.getValid(e,v)) return false;
  output("Checking block approveds validity...");
  for (auto i: approved) if (!i->getValid(e,v)) return false;
  output("Returning block is valid");
  // check nonce
  validCheckEnd();
}
void Block::apply(ExtraChainData& e) const {
  HasID::apply(this,e);
  for (auto i: txns) i.apply(e);
}
void Block::unapply(ExtraChainData& e) const {
  HasID::unapply(this,e);
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
