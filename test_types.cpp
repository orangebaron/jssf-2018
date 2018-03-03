#include "types.hpp"
#include "types.cpp"
#include <iostream>
#include <string>
using namespace blockchain;

#define testNum 0
#define assert(a,b) \
if (argc==1 || std::stoi(argv[1])==__LINE__) \
if (!a) { std::cout<<"Test failed: "<<b<<", line "<<__LINE__<<std::endl; return 0; }

// call with no arguments to run all tests or one argument (line no.) to do 1 test
int main(int argc, char* argv[]) {
  ExtraChainData e;
  ValidsChecked v;
  Pubkey p1, p2(1);
  TxnOtp origin = TxnOtp(p1,10,v,true);
  assert(origin.getValid(e,v),"Origin");
  assert(! Txn({&origin},{},{}).getValid(e,v),"Txn with no output or signature");
  assert(! Txn({&origin},{},{Sig(p1)}).getValid(e,v),"Txn with no output");
  assert(! Txn({&origin},{TxnOtp(p1,10)},{}).getValid(e,v),"Txn with no signature");
  assert(! Txn({&origin},{TxnOtp(p1,10)},{Sig(p2)}).getValid(e,v),"Txn with wrong signature");
  assert(! Txn({&origin},{TxnOtp(p1,10)},{Sig(p1),Sig(p1)}).getValid(e,v),"Txn with too many signatures");
  assert(! Txn({&origin},{TxnOtp(p1,5)},{Sig(p1)}).getValid(e,v),"Valid with less otp than inp");
  assert(! Txn({&origin,&origin},{TxnOtp(p1,20)},{Sig(p1)}).getValid(e,v),"Duplicate inputs");
  assert(  Txn({&origin},{TxnOtp(p1,10)},{Sig(p1)}).getValid(e,v),"Valid 1-to-1 transaction");
  Txn t1({&origin},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p1)});
  assert(t1.getValid(e,v),"Valid 1-to-2 transaction");
  // TODO: test multiple i/o txns
  Block b1 = Block({t1},{});
  assert(b1.getValid(e),"Empty block for now (TODO: do things with this)");
  b1.apply(e);
  std::cout << "Tests succeeded" << std::endl;
  return 0;
}
