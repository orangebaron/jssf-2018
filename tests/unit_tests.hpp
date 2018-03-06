#ifndef UNIT_TESTS_HPP
#define UNIT_TESTS_HPP

#define test(cond,name) \
if (testWanted==-1 || testWanted==testNum) \
if (!(cond)) std::cout<<"Test failed: "<<name<< \
  ", line "<<__LINE__<< \
  ", file "<<__FILE__<< \
  ", test no. "<<testNum<< \
  std::endl; \
testNum++

#endif
