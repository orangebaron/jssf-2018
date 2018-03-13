#ifndef TYPES_CPP
#define TYPES_CPP

#include "types.hpp"
#include <iostream>

using namespace blockchain;
#include <map>
#include "common_macros.hpp"

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
WorkType Sig::getWork(WorkCalculated&) const { return 2; }

TxnOtp::TxnOtp(Pubkey person,TxnAmt amt,ValidsChecked* v): person(person), amt(amt) {
  if (v != nullptr) (*v)[this] = true;
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
WorkType TxnOtp::getWork(WorkCalculated&) const { return 4; }

StorageChange::StorageChange(Pubkey person,unsigned int location,unsigned int value,unsigned int prevValue):
  person(person), location(location), value(value), prevValue(prevValue) {}
StorageChange::StorageChange(Pubkey person,unsigned int location,unsigned int value,const ExtraChainData& e):
  person(person), location(location), value(value) {
  try { prevValue = e.storage.at(person).at(location); } catch (...) { prevValue = 0; }
}
Pubkey StorageChange::getPerson() const { return person; }
unsigned int StorageChange::getLocation() const { return location; }
unsigned int StorageChange::getValue() const { return value; }
unsigned int StorageChange::getPrevValue() const { return prevValue; }
void StorageChange::apply(ExtraChainData& e) const {
  e.storage[person][location] = value;
}
void StorageChange::unapply(ExtraChainData& e) const {
  e.storage[person][location] = prevValue;
}
WorkType StorageChange::getWork(WorkCalculated&) const { return prevValue==0 ? 20 : 10; }

#endif
