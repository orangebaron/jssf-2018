#ifndef TEST_TXNS_HPP
#define TEST_TXNS_HPP

#include "unit_tests.hpp"
#include "../types.cpp"
#include <iostream>
using namespace blockchain;

void testTxns(int& testNum,int testWanted) {
  ExtraChainData e;
  ValidsChecked v;
  Pubkey p1, p2(1);
  TxnOtp origin = TxnOtp(p1,10,&v);
  test(origin.getValid(e,v),"Origin");
  test(! Txn({&origin},{},{}).getValid(e,v),"Txn with no output or signature");
  test(! Txn({&origin},{},{}).getValid(e,v),"Re-checking validity (false)");
  test(! Txn({&origin},{},{Sig(p1)}).getValid(e,v),"Txn with no output");
  test(! Txn({&origin},{TxnOtp(p1,10)},{}).getValid(e,v),"Txn with no signature");
  test(! Txn({&origin},{TxnOtp(p1,10)},{Sig(p2)}).getValid(e,v),"Txn with wrong signature");
  test(! Txn({&origin},{TxnOtp(p1,10)},{Sig(p1),Sig(p1)}).getValid(e,v),"Txn with too many signatures");
  test(! Txn({&origin},{TxnOtp(p1,5)},{Sig(p1)}).getValid(e,v),"Valid with less otp than inp");
  test(! Txn({&origin,&origin},{TxnOtp(p1,20)},{Sig(p1)}).getValid(e,v),"Duplicate inputs");
  test(  Txn({&origin},{TxnOtp(p1,10)},{Sig(p1)}).getValid(e,v),"Valid 1-to-1 transaction");
  Txn t1({&origin},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p1)});
  test(t1.getValid(e,v),"Valid 1-to-2 transaction");
  test(t1.getValid(e,v),"Re-checking validity(true)");
  Block b1 = Block({t1},{});
  test(b1.getValid(e,v),"Empty block for now (TODO: do things with this)");
  b1.apply(e);
  test(! Txn({&origin},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p1)}).getValid(e,v),"Txn that uses spent output");
  test(! Txn({(const TxnOtp*)&origin,&(t1.getOtps()[0])},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p1)}).getValid(e,v),"Txn that uses partly spent output");
  test(! Txn({&b1.getTxns()[0].getOtps()[0],&b1.getTxns()[0].getOtps()[1]},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p2)}).getValid(e,v),"2 input, 1 sig");
  test(! Txn({&b1.getTxns()[0].getOtps()[0],&b1.getTxns()[0].getOtps()[1]},{TxnOtp(p1,5)},{Sig(p1),Sig(p2)}).getValid(e,v),"Multi input, smaller coinamt output");
  Txn t2({&b1.getTxns()[0].getOtps()[0],&b1.getTxns()[0].getOtps()[1]},{TxnOtp(p1,5),TxnOtp(p2,5)},{Sig(p1),Sig(p2)});
  test(  t2.getValid(e,v),"2-input 2-output txn");
}

#endif
