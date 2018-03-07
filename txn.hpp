#ifndef TXN_HPP
#define TXN_HPP

#include "code.hpp"

namespace blockchain {
  class Txn: public Hashable, public Validable, public Applyable {
    vector<const TxnOtp*> inps;
    vector<TxnOtp> otps;
    vector<ContractCreation> contractCreations;
    vector<ContractCall> contractCalls;
    vector<Sig> sigs;
  public:
    Txn(vector<const TxnOtp*>,vector<TxnOtp>,vector<ContractCreation>,vector<Sig>);
    virtual Hash getHash() const;
    virtual Hash getHashBeforeSig() const;
    virtual bool getValid(const ExtraChainData&,ValidsChecked&) const;
    virtual void apply(ExtraChainData&) const;
    virtual void unapply(ExtraChainData&) const;
    const vector<TxnOtp>& getOtps() const;
  };
  class Block: public Hashable, public Validable, public Applyable {
    vector<Txn> txns;
    vector<Block*> approved;
    int nonce;
  public:
    Block(vector<Txn>,vector<Block*>);
    virtual Hash getHash() const;
    virtual bool getValid(const ExtraChainData&,ValidsChecked&) const;
    virtual void apply(ExtraChainData&) const;
    virtual void unapply(ExtraChainData&) const;
    const vector<Txn>& getTxns() const;
  };
}

#endif
