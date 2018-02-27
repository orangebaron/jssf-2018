#include "types.hpp"
#include "types.cpp"
#include <iostream>
using namespace blockchain;

#define assert(a,b) if(!a) { std::cout<<"Test failed: "<<b<<std::endl; return 0; }

int main() {
  Pubkey p, q(4);
  TxnOtp origin = TxnOtp(p,5,true);
  assert(origin.getValid(),"Origin");
  assert(! Txn({&origin},{},{}).getValid(),"Txn with no output or signature");
  assert(! Txn({&origin},{},{Sig(p)}).getValid(),"Txn with no output");
  assert(! Txn({&origin},{TxnOtp(p,5)},{}).getValid(),"Txn with no signature");
  assert(! Txn({&origin},{TxnOtp(p,5)},{Sig(q)}).getValid(),"Txn with wrong signature");
  assert(  Txn({&origin},{TxnOtp(p,5)},{Sig(p)}).getValid(),"Valid 1-to-1 transaction");
  // TODO: test multiple i/o txns
  Block a = Block({
    Txn({},{},{})
  },{

  });
  assert(a.getValid(),"Empty block for now (TODO: do things with this)");
  std::cout << "Tests succeeded" << std::endl;
  return 0;
}
