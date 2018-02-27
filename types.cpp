#include "types.hpp"

using namespace blockchain;

#include <map>

#define validCheckBegin(); \
  if (validChecked==False) return false; \
  if (validChecked==True) return true; \
  validChecked = False;
#define validCheckEnd(); \
  validChecked = True; \
  return true;

bool Pubkey::operator==(Pubkey p) const {
  return true;
}

Sig::Sig(Pubkey pubkey): pubkey(pubkey) {}
Pubkey Sig::getPerson() const {
  return pubkey;
}
Hash Sig::getHash() const {
  return Hash();
}
bool Sig::getValid(Hashable& h) {
  validCheckBegin();
  validCheckEnd();
}

TxnOtp::TxnOtp(Pubkey person,TxnAmt amt): person(person), amt(amt) {}
Hash TxnOtp::getHash() const {
  return Hash();
}
bool TxnOtp::getValid() {
  validCheckBegin();
  validCheckEnd();
}
TxnAmt TxnOtp::getAmt() const {
  return amt;
}
Pubkey TxnOtp::getPerson() const {
  return person;
}

Txn::Txn(vector<TxnOtp*> inps,vector<TxnOtp> otps,vector<Sig> sigs): inps(inps),otps(otps),sigs(sigs) {}
Hash Txn::getHashBeforeSig() const {
  return Hash();
}
Hash Txn::getHash() const {
  return getHashBeforeSig();
}
bool Txn::getValid() {
  validCheckBegin();
  std::map<Pubkey,bool> sendersThatDidntSign;
  TxnAmt sent = 0, recieved = 0;
  for (auto i=inps.begin();i<inps.end();i++) {
    if (!(*i)->getValid()) return false;
    if (*i < &*otps.end() && *i >= &*otps.begin()) return false; // check if it's referring to one of the outputs of this txn
    sent += (*i)->getAmt();
    sendersThatDidntSign[(*i)->getPerson()] = true;
  }
  for (auto i=otps.begin();i<otps.end();i++) {
    if (!i->getValid()) return false;
    recieved = i->getAmt();
  }
  if (sent!=recieved) return false;
  for (auto i=sigs.begin();i<sigs.end();i++) {
    if (!i->getValid(*this)) return false;
    try {
      sendersThatDidntSign.erase(i->getPerson());
    } catch (...) {
      return false;
    }
  }
  if (sendersThatDidntSign.size() > 0) return false;
  validCheckEnd();
}

Block::Block(vector<Txn> txns,vector<Block*> approved): txns(txns),approved(approved) {}
Hash Block::getHash() const {
  return getHashBeforeSig();
}
bool Block::getValid() {
  validCheckBegin();
  for (auto i=txns.begin();i<txns.end();i++) if (!i->getValid()) return false;
  for (auto i=approved.begin();i<approved.end();i++) if (!(*i)->getValid()) return false;
  // check nonce
  validCheckEnd();
}
