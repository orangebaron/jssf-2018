#ifndef CODE_CPP
#define CODE_CPP

#include "code.hpp"
#include <iostream>
using namespace blockchain;

#define command(n,cost,aSize,code); \
case n: \
if (returnVal.gasUsed + cost > gasLimit) return returnVal; \
returnVal.gasUsed += cost; \
argsSize = aSize; \
code; \
break;

#define memat(x) m[codeLoc+x]
#define ptrat(x) m[memat(x)]

RunOtp CodeMemory::run(GasAmt gasLimit) {
  RunOtp returnVal;
  auto m = memory.data();
  for (size_t codeLoc = 0, argsSize = 0; codeLoc < memory.size(); codeLoc += 1 + argsSize, argsSize = 0) {
    switch(memory[codeLoc]) {
        command(0,1,2, //ADD *A B
        ptrat(1) += memat(2);
      ); command(1,1,2, //SUB *A B
        ptrat(1) -= memat(2);
      ); command(2,3,2, //MULT *A B
        ptrat(1) *= memat(2);
      ); command(3,5,2, //DIV *A B
        ptrat(1) /= memat(2);
      ); command(4,1,2, //BISHIFT-R *A B
        ptrat(1) >>=memat(2);
      ); command(5,1,2, //BITSHIFT-L *A B
        ptrat(1) <<= memat(2);
      ); command(6,1,1, //INVERT *A
        ptrat(1) = -ptrat(1);
      ); command(10,2,1, //SPEND AMT
        returnVal.moneySpent.push_back(TxnOtp(Pubkey(),ptrat(1)));
      );
    }
  }
  return returnVal;
}

#endif
