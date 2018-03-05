#include "types.cpp"
#include "code.cpp"
#include <iostream>
#include <string>
using namespace blockchain;

#define testNum 0
#define assert(a,b) \
if (argc==1 || std::stoi(argv[1])==__LINE__) \
if (!(a)) { std::cout<<"Test failed: "<<b<<", line "<<__LINE__<<std::endl; /*return 0;*/ }

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
  assert(  t2.getValid(e,v),"2-input 2-output txn");

  #define cmd(x) (unsigned int)Opcodes::x
  //basic code, will be changed later
  CodeMemory c0({cmd(ADD),6,1, cmd(SPEND),6, cmd(QUIT), 5});
  auto x = c0.run(10);
  assert(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 6 && x.gasUsed == 3,"Add and spend code, more than enough gas");
  c0 = CodeMemory({cmd(ADD),6,1, cmd(SPEND),6, cmd(QUIT), 5});
  x = c0.run(2);
  assert(x.moneySpent.size() == 0 && x.gasUsed == 1,"Add and spend code, not enough gas");
  c0 = CodeMemory({cmd(SUB),6,1, cmd(SPEND),6, cmd(QUIT), 5});
  x = c0.run(3);
  assert(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 4 && x.gasUsed == 3,"Sub and spend code, just enough gas");
  c0 = CodeMemory({cmd(MULT),6,2, cmd(SPEND),6, cmd(QUIT), 5});
  x = c0.run(10);
  assert(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 10 && x.gasUsed == 5,"Mult and spend code");
  c0 = CodeMemory({cmd(DIV),6,5, cmd(SPEND),6, cmd(QUIT), 5});
  x = c0.run(10);
  assert(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 1 && x.gasUsed == 7,"Div and spend code");
  c0 = CodeMemory({cmd(SHIFTR),11,8, cmd(SPEND),11, cmd(SHIFTL),11,16, cmd(SPEND),11, cmd(QUIT), 0x100});
  x = c0.run(10);
  assert(x.moneySpent.size() == 2 && x.moneySpent[0].getAmt() == 1 && x.moneySpent[1].getAmt() == 0x10000 && x.gasUsed == 6,"Bitshift-R, Bitshift-L and spend code");
  c0 = CodeMemory({cmd(INV),5, cmd(SPEND),5, cmd(QUIT), (unsigned int)-1});
  x = c0.run(10);
  assert(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 1 && x.gasUsed == 3,"Inv and spend code");

  std::cout << "Tests done" << std::endl;
  return 0;
}
