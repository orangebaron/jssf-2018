#ifndef TYPES_CPP
#define TYPES_CPP

#include "types.hpp"
#include "code.hpp"
#include <iostream>
#include <cstdlib>

using namespace blockchain;
#include <map>
#include "common_macros.hpp"

bool Pubkey::operator==(Pubkey p) const {
  return x==p.x;
}
bool Pubkey::operator<(Pubkey p) const {
  return x<p.x;
}

HasID::HasID(): id(
  ((int)rand()<<(8*3)) |
  ((int)rand()<<(8*2)) |
  ((int)rand()<<(8*1)) |
  (int)rand()
) {}
bool HasID::getValid(const ExtraChainData& e) const {
  return e.IDs.find(this)==e.IDs.end();
}
void HasID::apply(ExtraChainData& e) const {
  e.IDs.emplace(this,id);
  e.IDsReverse.emplace(id,this);
}
void HasID::unapply(ExtraChainData& e) const {
  e.IDs.erase(this);
  e.IDsReverse.erase(id);
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

TxnOtp::TxnOtp(Pubkey person,TxnAmt amt,ValidsChecked* v): HasID(), person(person), amt(amt) {
  if (v != nullptr) (*v)[this] = true;
}
Hash TxnOtp::getHash() const {
  return Hash();
}
bool TxnOtp::getValid(const ExtraChainData& e, ValidsChecked& v) const {
  validCheckBegin();
  if (!((HasID*)this)->getValid(e)) return false;
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
  ((HasID*)this)->apply(e);
}
void StorageChange::unapply(ExtraChainData& e) const {
  e.storage[person][location] = prevValue;
  ((HasID*)this)->unapply(e);
}
WorkType StorageChange::getWork(WorkCalculated&) const { return prevValue==0 ? 20 : 10; }

ExtraChainData ExtraChainData::merge(ExtraChainData& e) { //CHECK ALL OF THIS
  ExtraChainData a = {
    map<const TxnOtp*,const Txn*>(spentOutputs),
    map<Pubkey,map<unsigned int,unsigned int>>(storage),
    map<Pubkey,CodeMemory>(contractCodes),
    map<Pubkey,TxnAmt>(contractMoney),
    map<Pubkey,map<unsigned int,const ContractCall*>>(),
    map<const Txn*,vector<RunOtp>>(contractOtps)
  };
  for (auto i: contractMaxIds) a.contractMaxIds[i.first] = map<unsigned int,const ContractCall*>(i.second);
  for (auto i: e.spentOutputs) {
    if (a.spentOutputs.find(i.first)==a.spentOutputs.end()) a.spentOutputs[i.first] = i.second;
    else if (a.spentOutputs[i.first]!=i.second) throw nullptr;
  }
  for (auto i: e.contractMaxIds) {
    bool copyElems = true;
    if (a.contractMaxIds.find(i.first)!=a.contractMaxIds.end()) {
      if (a.contractMaxIds[i.first].size()<i.second.size()) {
        if (i.second[a.contractMaxIds[i.first].size()-1]!=(*(--a.contractMaxIds[i.first].end())).second) throw nullptr; //CHECK THIS
      } else {
        if (a.contractMaxIds[i.first][i.second.size()-1]!=(*(--i.second.end())).second) throw nullptr; //CHECK THIS
        copyElems = false;
      }
    } else {
      a.contractCodes.emplace(i.first,CodeMemory(e.contractCodes[i.first]));
    }
    if (copyElems) {
      a.storage[i.first]=e.storage[i.first];
      a.contractMoney[i.first]=e.contractMoney[i.first];
      a.contractMaxIds[i.first]=map<unsigned int,const ContractCall*>(i.second);
    }
  }
  for (auto i: e.contractOtps)
    if (a.contractOtps.find(i.first)==a.contractOtps.end()) a.contractOtps[i.first] = vector<RunOtp>(i.second);
  return a;
}

#endif
