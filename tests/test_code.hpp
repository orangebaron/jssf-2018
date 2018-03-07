#ifndef TEST_CODE_HPP
#define TEST_TXNS_HPP

#include "unit_tests.hpp"
#include "../code.cpp"
#include <iostream>
using namespace blockchain;

#define cmd(x) (unsigned int)Opcodes::x
#define arr(...) { __VA_ARGS__ }
#define singleSpendCodeTest(arr,gasLim,spent,gas,name) \
test(CodeMemory(arr).run(gasLim,e,Pubkey()).moneySpent.size() == 1 && \
CodeMemory(arr).run(gasLim,e,Pubkey()).moneySpent[0].getAmt() == spent && \
CodeMemory(arr).run(gasLim,e,Pubkey()).gasUsed == gas,name);
#define multipleSpendCodeTest(arr,gasLim,amtSpent,gas,name) \
test(CodeMemory(arr).run(gasLim,e,Pubkey()).moneySpent.size() == amtSpent && \
CodeMemory(arr).run(gasLim,e,Pubkey()).gasUsed == gas,name);

void testCode(int& testNum,int testWanted) {
  ExtraChainData e;
  #define x CodeMemory({cmd(SPEND),4,5, cmd(QUIT), 2,3}).run(2,e,Pubkey())
  test(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 3 &&
    x.moneySpent[0].getPerson() == 2 && x.gasUsed == 2,
    "Spend but pubkey is specified");
  #undef x
  #define x CodeMemory({cmd(SHIFTR),13,8, cmd(SPEND),0,13, cmd(SHIFTL),13,16, cmd(SPEND),0,13, cmd(QUIT), 0x100}).run(10,e,Pubkey())
  test(x.moneySpent.size() == 2 &&
  x.moneySpent[0].getAmt() == 1 &&
  x.moneySpent[1].getAmt() == 0x10000 &&
  x.gasUsed == 6,"Bitshift-R, Bitshift-L code");
  #undef x
  #define x CodeMemory({cmd(ADD),7,1, cmd(SPEND),0,7, cmd(QUIT), 5}).run(2,e,Pubkey())
  test(x.moneySpent.size() == 0 && x.gasUsed == 1,"Add code, not enough gas");
  #undef x

  singleSpendCodeTest(arr(cmd(ADD),7,1, cmd(SPEND),0,7, cmd(QUIT), 5),10,6,3,"Add code, more than enough gas");
  singleSpendCodeTest(arr(cmd(SUB),7,1, cmd(SPEND),0,7, cmd(QUIT), 5),3,4,3,"Sub code, just enough gas");
  singleSpendCodeTest(arr(cmd(MULT),7,2, cmd(SPEND),0,7, cmd(QUIT), 5),10,10,5,"Mult code");
  singleSpendCodeTest(arr(cmd(DIV),7,5, cmd(SPEND),0,7, cmd(QUIT), 5),10,1,7,"Div code");
  singleSpendCodeTest(arr(cmd(INV),6, cmd(SPEND),0,6, cmd(QUIT), (unsigned int)-1),10,1,3,"Inv code");
  singleSpendCodeTest(arr(cmd(JMP),6, cmd(SPEND),0,6, cmd(QUIT), cmd(SPEND),0,10, cmd(QUIT), 10),10,10,3,"Jmp code");
  singleSpendCodeTest(arr(cmd(MEMSET),10,11, cmd(MEMGET),11,10, cmd(SPEND),0,11, cmd(QUIT), 0,5),20,5,6,"Storage edit code");
  e.storage[Pubkey()][2] = 5;
  singleSpendCodeTest(arr(cmd(MEMGET),8,7, cmd(SPEND),0,8, cmd(QUIT), 2,0),20,5,4,"Storage get code");
  singleSpendCodeTest(arr(cmd(SET),11,7, cmd(MOV),10,11, cmd(SPEND),0,10, cmd(QUIT), 0,0),20,7,4,"Move and set code");

  multipleSpendCodeTest(arr(cmd(SUB),10,1, cmd(SPEND),0,0, cmd(JNE),10,0, cmd(QUIT), 5),20,5,20,"JNE loop code");
  multipleSpendCodeTest(arr(cmd(SUB),10,1, cmd(SPEND),0,0, cmd(JE),10,0, cmd(QUIT), 1),20,2,8,"JE single loop code");
  multipleSpendCodeTest(arr(cmd(SUB),10,1, cmd(SPEND),0,0, cmd(JG),10,0, cmd(QUIT), 6),20,5,20,"JG loop code");
  multipleSpendCodeTest(arr(cmd(ADD),10,1, cmd(SPEND),0,0, cmd(JL),10,0, cmd(QUIT), (unsigned int)-2),20,2,8,"JL loop code");
  multipleSpendCodeTest(arr(cmd(SUB),10,1, cmd(SPEND),0,0, cmd(JGE),10,0, cmd(QUIT), 6),24,6,24,"JGE loop code");
  multipleSpendCodeTest(arr(cmd(ADD),10,1, cmd(SPEND),0,0, cmd(JLE),10,0, cmd(QUIT), (unsigned int)-2),20,3,12,"JLE loop code");
}

#endif
