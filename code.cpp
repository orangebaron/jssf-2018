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

#define memat(x) memory.at(codeLoc+x)
#define ptrat(x) memory.at(memat(x))
#define mathcmd(cmd,cost,x) command(cmd,cost,2,ptrat(1) x##= memat(2))
#define boolcmd(cmd,cost,x) command(cmd,cost,2,ptrat(1) = ptrat(1) x ptrat(2))
#define invboolcmd(cmd,cost,x) command(cmd,cost,2,ptrat(1) = !(ptrat(1) x ptrat(2)))
#define cndjmp(cmd,cost,x) command(cmd,cost,1,if((int)ptrat(1) x 0) {codeLoc = memat(2); justJumped = true;})

RunOtp CodeMemory::run(GasAmt gasLimit, const ExtraChainData& e, Pubkey p) {
  RunOtp returnVal;
  try {
  for (size_t codeLoc = 0, argsSize = 0, justJumped = false;
    codeLoc < memory.size();
    codeLoc += justJumped ? 0 : (1 + argsSize), argsSize = 0, justJumped = false) {

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
      command(JMP,1,1,codeLoc = memat(1);justJumped = true);
      cndjmp(JE,1,==);
      cndjmp(JNE,1,!=);
      cndjmp(JL,1,<);
      cndjmp(JG,1,>);
      cndjmp(JLE,1,<=);
      cndjmp(JGE,1,>=);
      command(SPEND,2,2,returnVal.moneySpent.push_back(TxnOtp(Pubkey(ptrat(1)),ptrat(2))));
      command(MEMGET,2,2,
        {ptrat(1) = 0;
        try { ptrat(1)=e.storage.at(p).at(ptrat(2)); } catch(...) {}
        for (auto i: returnVal.storageChanged) if (i.getLocation() == ptrat(2)) ptrat(1) = i.getValue();
        });
      command(MEMSET,2,2,returnVal.storageChanged.push_back(StorageChange(p,ptrat(1),ptrat(2),e)));
      command(DEBUGPRINT,0,0,std::cout<<"DEBUG PRINT "<<codeLoc<<std::endl);
    }
  }
  } catch (...) {}
  return returnVal;
}

#endif
