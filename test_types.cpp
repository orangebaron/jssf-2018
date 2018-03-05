#include <string>
#include "test_txns.hpp"
#include "test_code.hpp"

// call with no arguments to run all tests or one argument (test no.) to do 1 test
int main(int argc, char* argv[]) {
  int testNum = 0;
  int testWanted = argc==1 ? -1 : std::stoi(argv[1]);
  testTxns(testNum,testWanted);
  testCode(testNum,testWanted);
  std::cout << "Tests done" << std::endl;
  return 0;
}
