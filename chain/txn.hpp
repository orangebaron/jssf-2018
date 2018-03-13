#ifndef TXN_HPP
#define TXN_HPP

#include "code.hpp"

namespace blockchain {
  class Txn: public Hashable, public Validable, public Applyable, public WorkRequired {
    vector<const TxnOtp*> inps;
    vector<TxnOtp> otps;
    vector<ContractCreation> contractCreations;
    vector<ContractCall> contractCalls;
    vector<Sig> sigs;
  public:
    Txn(vector<const TxnOtp*>,vector<TxnOtp>,vector<ContractCreation>,vector<ContractCall>,vector<Sig>);
    virtual Hash getHash() const;
    virtual Hash getHashBeforeSig() const;
    virtual bool getValid(const ExtraChainData&,ValidsChecked&) const;
    virtual void apply(ExtraChainData&) const;
    virtual void unapply(ExtraChainData&) const;
    virtual WorkType getWork(WorkCalculated&) const;
    const vector<TxnOtp>& getOtps() const;
  };
  class Block: public Hashable, public Validable, public Applyable, public WorkRequired {
    vector<Txn> txns;
    vector<Block*> approved;
    ExtraChainData extraDataAfterBlock;
    //int nonce; TODO: are we gonna do legit hashes?
  public:
    Block(vector<Txn>,vector<Block*>);
    virtual Hash getHash() const;
    virtual bool getValid(const ExtraChainData&,ValidsChecked&) const;
    virtual void apply(ExtraChainData&) const;
    virtual void unapply(ExtraChainData&) const;
    virtual WorkType getWork(WorkCalculated&) const;
    WorkType getSumWork(WorkCalculated&) const;
    const vector<Txn>& getTxns() const;
    const ExtraChainData& getExtraDataAfterBlock();
    vector<Block*> getApproved();
  };
}

#endif
