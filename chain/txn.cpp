#include "txn.hpp"

using namespace blockchain;
#include "common_macros.hpp"

Txn::Txn(vector<const TxnOtp*> inps,vector<TxnOtp> otps,vector<ContractCreation> contractCreations,vector<ContractCall> contractCalls,vector<Sig> sigs):
  inps(inps),otps(otps),contractCreations(contractCreations),contractCalls(contractCalls),sigs(sigs) {}
Hash Txn::getHashBeforeSig() const {
  return Hash();
}
Hash Txn::getHash() const {
  return getHashBeforeSig();
}
bool Txn::getValid(const ExtraChainData& e, ValidsChecked& v) const {
  validCheckBegin();
  std::map<Pubkey,bool> sendersThatDidntSign;
  std::map<const TxnOtp*,bool> inputsUsed;
  TxnAmt sent = 0, recieved = 0;
  for (auto i: inps) {
    if (!i->getValid(e,v)) return false;
    if (i < &*otps.end() && i >= &*otps.begin()) return false; // check if it's referring to one of the outputs of this txn
    if (inputsUsed[i]) return false;
    try { if (e.spentOutputs.at(i)) return false; } catch(...) {}
    inputsUsed[i] = true;
    sent += i->getAmt();
    sendersThatDidntSign[i->getPerson()] = true;
  }
  for (auto i: otps) {
    if (!i.getValid(e,v)) return false;
    recieved += i.getAmt();
  }
  for (auto i: contractCreations) {
    if (!i.getValid(e,v)) return false;
    sendersThatDidntSign[i.getKey()] = true;
  }
  if (sent!=recieved) return false;
  for (auto i: sigs) {
    if (!i.getValid(*this)) return false;
    try {
      sendersThatDidntSign.at(i.getPerson());
      sendersThatDidntSign.erase(i.getPerson());
    } catch (...) {
      return false;
    }
  }
  if (sendersThatDidntSign.size() > 0) return false;
  validCheckEnd();
}
void Txn::apply(ExtraChainData& e) const {
  for (auto i: inps) e.spentOutputs[i] = this;
  for (auto i: contractCreations) i.apply(e);
  for (auto i: contractCalls) {
    e.contractMoney[i.getCalled()] += i.getAmt();
    e.contractOtps[this].push_back(e.contractCodes.find(i.getCalled())->second.run(e,i));
  }
  for (auto i: e.contractOtps[this]) {
    for (auto j: i.moneySpent) e.spentOutputs[&j] = this;
    for (auto j: i.storageChanged) j.apply(e);
  }
}
void Txn::unapply(ExtraChainData& e) const {
  for (auto i: inps) if (e.spentOutputs[i] == this) e.spentOutputs[i] = NULL;
  for (auto i: contractCreations) i.unapply(e);
  for (auto i: e.contractOtps[this]) {
    for (auto j: i.moneySpent) if (e.spentOutputs[&j] == this) e.spentOutputs[&j] = NULL;
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
const vector<TxnOtp>& Txn::getOtps() const {
  return otps;
}

Block::Block(vector<Txn> txns,vector<Block*> approved): txns(txns),approved(approved) {}
Hash Block::getHash() const {
  return getHashBeforeSig();
}
bool Block::getValid(const ExtraChainData& e, ValidsChecked& v) const {
  validCheckBegin();
  for (auto i: txns) if (!i.getValid(e,v)) return false;
  for (auto i: approved) if (!i->getValid(e,v)) return false;
  // check nonce
  validCheckEnd();
}
void Block::apply(ExtraChainData& e) const {
  for (auto i: txns) i.apply(e);
}
void Block::unapply(ExtraChainData& e) const {
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
