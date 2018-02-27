#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>
using std::vector;

namespace blockchain {
  enum ValidChecked { Unchecked,False,True };
  class Pubkey {
    int x = 0; // just for now
  public:
    Pubkey() {} // just for now
    Pubkey(int x): x(x) {} // just for now
    bool operator==(Pubkey) const;
    bool operator<(Pubkey) const;
  };
  class Hash {};
  class Validable {
  protected:
    ValidChecked validChecked = Unchecked;
  public:
    virtual bool getValid() = 0;
  };
  class Hashable {
  public:
    virtual Hash getHash() const = 0;
    virtual Hash getHashBeforeSig() const { return Hash(); } // only write this func if thing is signed
  };
  class Sig: public Hashable {
    Pubkey pubkey;
    // more data here
    ValidChecked validChecked = Unchecked;
  public:
    Sig(Pubkey);
    Pubkey getPerson() const;
    virtual bool getValid(Hashable&);
    virtual Hash getHash() const;
  };
  typedef unsigned int TxnAmt;
  class TxnOtp: public Hashable, public Validable {
    Pubkey person;
    TxnAmt amt;
  public:
    TxnOtp(Pubkey,TxnAmt,bool isOrigin=false);
    TxnAmt getAmt() const;
    Pubkey getPerson() const;
    virtual Hash getHash() const;
    virtual bool getValid();
  };
  class Txn: public Hashable, public Validable {
    vector<TxnOtp*> inps;
    vector<TxnOtp> otps;
    vector<Sig> sigs;
  public:
    Txn(vector<TxnOtp*>,vector<TxnOtp>,vector<Sig>);
    virtual Hash getHash() const;
    virtual Hash getHashBeforeSig() const;
    virtual bool getValid();
  };
  class Block: public Hashable, public Validable {
    vector<Txn> txns;
    vector<Block*> approved;
    int nonce;
  public:
    Block(vector<Txn>,vector<Block*>);
    virtual Hash getHash() const;
    virtual bool getValid();
  };
}

#endif
