#include "types.hpp"
#include "types.cpp"
#include <iostream>
#include <string>
using namespace blockchain;

#define testNum 0
#define assert(a,b) \
if (argc==1 || std::stoi(argv[1])==__LINE__) \
if (!a) { std::cout<<"Test failed: "<<b<<", line "<<__LINE__<<std::endl; /*return 0;*/ }

// call with no arguments to run all tests or one argument (line no.) to do 1 test
int main(int argc, char* argv[]) {
  ExtraChainData e;
  ValidsChecked v;
  Pubkey p1, p2(1);
  TxnOtp origin = TxnOtp(p1,10,&v);
  assert(origin.getValid(e,v),"Origin");
  Txn t0({&origin},{},{});
  assert(! t0.getValid(e,v),"Txn with no output or signature");
  assert(! t0.getValid(e,v),"Re-checking validity (false)");
  assert(! Txn({&origin},{},{Sig(p1)}).getValid(e,v),"Txn with no output");
  assert(! Txn({&origin},{TxnOtp(p1,10)},{}).getValid(e,v),"Txn with no signature");
  assert(! Txn({&origin},{TxnOtp(p1,10)},{Sig(p2)}).getValid(e,v),"Txn with wrong signature");
  assert(! Txn({&origin},{TxnOtp(p1,10)},{Sig(p1),Sig(p1)}).getValid(e,v),"Txn with too many signatures");
  assert(! Txn({&origin},{TxnOtp(p1,5)},{Sig(p1)}).getValid(e,v),"Valid with less otp than inp");
  assert(! Txn({&origin,&origin},{TxnOtp(p1,20)},{Sig(p1)}).getValid(e,v),"Duplicate inputs");
  assert(  Txn({&origin},{TxnOtp(p1,10)},{Sig(p1)}).getValid(e,v),"Valid 1-to-1 transaction");
  Txn t1({&origin},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p1)});
  assert(t1.getValid(e,v),"Valid 1-to-2 transaction");
  assert(t1.getValid(e,v),"Re-checking validity(true)");
  Block b1 = Block({t1},{});
  assert(b1.getValid(e,v),"Empty block for now (TODO: do things with this)");
  b1.apply(e);
  assert(! Txn({&origin},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p1)}).getValid(e,v),"Txn that uses spent output");
  assert(! Txn({(const TxnOtp*)&origin,&(t1.getOtps()[0])},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p1)}).getValid(e,v),"Txn that uses partly spent output");
  assert(! Txn({&b1.getTxns()[0].getOtps()[0],&b1.getTxns()[0].getOtps()[1]},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p2)}).getValid(e,v),"2 input, 1 sig");
  assert(! Txn({&b1.getTxns()[0].getOtps()[0],&b1.getTxns()[0].getOtps()[1]},{TxnOtp(p1,5)},{Sig(p1),Sig(p2)}).getValid(e,v),"Multi input, smaller coinamt output");
  Txn t2({&b1.getTxns()[0].getOtps()[0],&b1.getTxns()[0].getOtps()[1]},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p1),Sig(p2)});
  assert(  t2.getValid(e,v),"2-input 2-output txn")
  std::cout << "Tests done" << std::endl;
  return 0;
}
