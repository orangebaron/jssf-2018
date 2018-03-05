#ifndef CODE_CPP
#define CODE_CPP

#include "code.hpp"
#include <iostream>
using namespace blockchain;

#define command(cmd,cost,aSize,code); \
case (unsigned int)Opcodes::cmd: \
if (returnVal.gasUsed + cost > gasLimit) return returnVal; \
returnVal.gasUsed += cost; \
argsSize = aSize; \
code; \
break;

#define memat(x) memory[codeLoc+x]
#define ptrat(x) memory[memat(x)]

RunOtp CodeMemory::run(GasAmt gasLimit) {
  RunOtp returnVal;
  for (size_t codeLoc = 0, argsSize = 0; codeLoc < memory.size(); codeLoc += 1 + argsSize, argsSize = 0) {
    switch(memory[codeLoc]) {
      command(QUIT,0,0, //QUIT
        codeLoc = memory.size();
      ); command(ADD,1,2, //ADD *A B
        ptrat(1) += memat(2);
      ); command(SUB,1,2, //SUB *A B
        ptrat(1) -= memat(2);
      ); command(MULT,3,2, //MULT *A B
        ptrat(1) *= memat(2);
      ); command(DIV,5,2, //DIV *A B
        ptrat(1) /= memat(2);
      ); command(SHIFTR,1,2, //BITSHIFT-R *A B
        ptrat(1) >>=memat(2);
      ); command(SHIFTL,1,2, //BITSHIFT-L *A B
        ptrat(1) <<= memat(2);
      ); command(INV,1,1, //INVERT *A
        ptrat(1) = -ptrat(1);
      ); command(SPEND,2,1, //SPEND AMT
        returnVal.moneySpent.push_back(TxnOtp(Pubkey(),ptrat(1)));
      );
    }
  }
  return returnVal;
}

#endif
