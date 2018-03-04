#ifndef TYPES_CPP
#define TYPES_CPP

#include "types.hpp"
#include <iostream>

using namespace blockchain;

#include <map>

#define validCheckBegin(); \
  try { if (v.at(this)) return true; else return false; } catch (...) {} \
  v[this] = false;
#define validCheckEnd(); \
  v[this] = true; \
  return true;

bool Pubkey::operator==(Pubkey p) const {
  return x==p.x;
}
bool Pubkey::operator<(Pubkey p) const {
  return x<p.x;
}

Sig::Sig(Pubkey pubkey): pubkey(pubkey) {}
Pubkey Sig::getPerson() const {
  return pubkey;
}
Hash Sig::getHash() const {
  return Hash();
}
bool Sig::getValid(const Hashable& h) const {
  return true;
}

TxnOtp::TxnOtp(Pubkey person,TxnAmt amt,ValidsChecked* v): person(person), amt(amt) {
  if (v != NULL) (*v)[this] = true;
}
Hash TxnOtp::getHash() const {
  return Hash();
}
bool TxnOtp::getValid(const ExtraChainData&, ValidsChecked& v) const {
  validCheckBegin();
  validCheckEnd();
}
TxnAmt TxnOtp::getAmt() const {
  return amt;
}
Pubkey TxnOtp::getPerson() const {
  return person;
}

Txn::Txn(vector<const TxnOtp*> inps,vector<TxnOtp> otps,vector<Sig> sigs): inps(inps),otps(otps),sigs(sigs) {}
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
}
void Txn::unapply(ExtraChainData& e) const {
  for (auto i: inps) if (e.spentOutputs[i] == this) e.spentOutputs[i] = NULL;
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
  for (auto i=txns.begin();i<txns.end();i++) if (!i->getValid(e,v)) return false;
  for (auto i=approved.begin();i<approved.end();i++) if (!(*i)->getValid(e,v)) return false;
  // check nonce
  validCheckEnd();
}
void Block::apply(ExtraChainData& e) const {
  for (auto i: txns) i.apply(e);
}
void Block::unapply(ExtraChainData& e) const {
  for (auto i: txns) i.unapply(e);
}
const vector<Txn>& Block::getTxns() const {
  return txns;
}

#endif
