#ifndef CODE_CPP
#define CODE_CPP

#include "code.hpp"
#include <iostream>
#include "common_macros.hpp"
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

CodeMemory::CodeMemory(vector<unsigned int> memory): memory(memory) {}
RunOtp CodeMemory::run(const ExtraChainData& e, const ContractCall& caller) const {
  vector<unsigned int> memory = this->memory;
  RunOtp returnVal;
  GasAmt gasLimit = caller.getMaxGas();
  Pubkey p = caller.getCalled();
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
      command(STRGET,2,2,
        {ptrat(1) = 0;
        try { ptrat(1)=e.storage.at(p).at(ptrat(2)); } catch(...) {}
        for (auto i: returnVal.storageChanged) if (i.getLocation() == ptrat(2)) ptrat(1) = i.getValue();
        });
      command(STRSET,2,2,returnVal.storageChanged.push_back(StorageChange(p,ptrat(1),ptrat(2),e)));
      command(MOV,1,2,ptrat(1)=ptrat(2));
      command(SET,1,2,ptrat(1)=memat(2));
      command(DEBUGPRINT,0,0,std::cout<<"DEBUG PRINT "<<codeLoc<<std::endl);
    }
  }
  } catch (...) {}
  return returnVal;
}
size_t CodeMemory::getMemSize() const { return memory.size(); }

ContractCreation::ContractCreation(CodeMemory mem,Pubkey key): mem(mem), key(key) {}
Pubkey ContractCreation::getKey() { return key; }
Hash ContractCreation::getHash() const {
  return Hash();
}
bool ContractCreation::getValid(const ExtraChainData& e,ValidsChecked& v) const {
  validCheckBegin();
  if (e.contractCodes.find(key)!=e.contractCodes.end()) return false;
  validCheckEnd();
}
void ContractCreation::apply(ExtraChainData& e) const {
  e.contractCodes.emplace(key,mem);
  e.contractMaxIds[key] = 0;
  e.contractMoney[key] = 0;
}
void ContractCreation::unapply(ExtraChainData& e) const {
  e.contractCodes.erase(key);
}
WorkType ContractCreation::getWork(WorkCalculated&) const { return 2*mem.getMemSize(); }

ContractCall::ContractCall(Pubkey caller,Pubkey called,vector<unsigned int> args,TxnAmt amt,GasAmt maxGas,unsigned long id):
  caller(caller),called(called),args(args),amt(amt),maxGas(maxGas),id(id) {}
Hash ContractCall::getHash() const { return Hash(); }
bool ContractCall::getValid(const ExtraChainData& e,ValidsChecked& v) const {
  validCheckBegin();
  if (e.contractCodes.find(called) == e.contractCodes.end()) return false;
  if (e.contractMaxIds.at(called)+1!=id) return false;
  validCheckEnd();
}
Pubkey ContractCall::getCaller() const { return caller; }
Pubkey ContractCall::getCalled() const { return called; }
vector<unsigned int> ContractCall::getArgs() const { return args; }
TxnAmt ContractCall::getAmt() const { return amt; }
GasAmt ContractCall::getMaxGas() const { return maxGas; }
RunOtp ContractCall::getOtp(const ExtraChainData& e) {
  return e.contractCodes.find(called)->second.run(e,*this);
}
unsigned long ContractCall::getId() const { return id; }
WorkType ContractCall::getWork(WorkCalculated&) const { return 5+(args.size()*2); }

#endif
