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

RunOtp CodeMemory::run(GasAmt gasLimit) {
  RunOtp returnVal;
  auto m = memory.data();
  for (size_t codeLoc = 0, argsSize = 0; codeLoc < memory.size(); codeLoc += 1 + argsSize, argsSize = 0) {
    switch(memory[codeLoc]) {
      command(0,1,2, //ADD *A B
        m[m[codeLoc+1]] += m[codeLoc+2];
      ); command(1,1,2, //SUB *A B
        m[m[codeLoc+1]] -= m[codeLoc+2];
      ); command(2,2,1, //SPEND AMT
        returnVal.moneySpent.push_back(TxnOtp(Pubkey(),m[m[codeLoc+1]]));
      );
    }
  }
  return returnVal;
}

#endif
