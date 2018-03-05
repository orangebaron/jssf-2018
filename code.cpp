#ifndef CODE_CPP
#define CODE_CPP

#include "code.hpp"
#include <iostream>
using namespace blockchain;

#define command(cmd,cost,aSize,code) \
case (unsigned int)Opcodes::cmd: \
if (returnVal.gasUsed + cost > gasLimit) return returnVal; \
returnVal.gasUsed += cost; \
argsSize = aSize; \
code; \
break;

#define memat(x) memory[codeLoc+x]
#define ptrat(x) memory[memat(x)]
#define mathcmd(cmd,cost,x) command(cmd,cost,2,ptrat(1) x##= memat(2))
#define boolcmd(cmd,cost,x) command(cmd,cost,2,ptrat(1) = ptrat(1) x ptrat(2))
#define invboolcmd(cmd,cost,x) command(cmd,cost,2,ptrat(1) = !(ptrat(1) x ptrat(2)))
#define cndjmp(cmd,cost,x) command(cmd,cost,2,if((int)ptrat(1) x 0) codeLoc = memat(2))

RunOtp CodeMemory::run(GasAmt gasLimit) {
  RunOtp returnVal;
  for (size_t codeLoc = 0, argsSize = 0; codeLoc < memory.size(); codeLoc += 1 + argsSize, argsSize = 0) {
    switch(memory[codeLoc]) {
      command(QUIT,0,0,codeLoc = memory.size());
      mathcmd(ADD,1,+);
      mathcmd(SUB,1,-);
      mathcmd(MULT,3,*);
      mathcmd(DIV,5,/);
      mathcmd(SHIFTR,1,>>);
      mathcmd(SHIFTL,1,<<);
      command(INV,1,1,ptrat(1) = -ptrat(1));
      boolcmd(AND,1,&&);
      boolcmd(OR,1,||);
      boolcmd(XOR,1,^);
      invboolcmd(NAND,1,&&);
      invboolcmd(NOR,1,||);
      invboolcmd(XNOR,1,^);
      command(JMP,1,1,codeLoc = memat(1));
      cndjmp(JE,1,==);
      cndjmp(JNE,1,!=);
      cndjmp(JL,1,<);
      cndjmp(JG,1,>);
      cndjmp(JLE,1,<=);
      cndjmp(JGE,1,>=);
      command(SPEND,2,1,returnVal.moneySpent.push_back(TxnOtp(Pubkey(),ptrat(1))));
    }
  }
  return returnVal;
}

#endif
