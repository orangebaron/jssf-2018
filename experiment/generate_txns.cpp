#ifndef GENERATE_TXNS_CPP
#define GENERATE_TXNS_CPP

#include "generate_txns.hpp"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
using namespace blockchain;

FileWrapper::FileWrapper(string filename) {
  file = open(filename.c_str(),O_RDWR);
  endPtr = (char*)mmap(NULL,1/*size*/,PROT_READ|PROT_WRITE,MAP_SHARED,file,0);
}
FileWrapper::~FileWrapper() {
  close(file);
}
size_t FileWrapper::write(FileData data) {
  FileData d { endPtr,data.len };
  endPtr += data.len;
  std::copy(data.data,data.data+data.len,d.data);
  idDataMap.push_back(d);
  return idDataMap.size()-1;
}
FileData FileWrapper::read(size_t id) {
  return idDataMap.at(id);
}

void User::genTxn(bool fake) {

}
User::User(FileWrapper& f, int txnsPerSecond, int fakesPerSecond): f(f) {
  t = thread([](User* user,int txnsPerSecond,int fakesPerSecond) {
    std::chrono::milliseconds ms(1000/(txnsPerSecond+fakesPerSecond));
    for (int counter=0; !user->stop; counter=(counter+1)%(txnsPerSecond+fakesPerSecond)) {
      user->genTxn(counter<txnsPerSecond);
      std::this_thread::sleep_for(ms);
    }
  },this,txnsPerSecond,fakesPerSecond);
}
User::~User() {
  stop = true;
  t.join();
}

Miner::Miner(FileWrapper& f, bool fake): f(f) {
  t = thread([](Miner* miner,bool fake) {
    while (!miner->stop) {}
  },this,fake);
}
Miner::~Miner() {
  stop = true;
  t.join();
}

#endif
