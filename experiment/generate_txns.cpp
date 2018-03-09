#ifndef GENERATE_TXNS_CPP
#define GENERATE_TXNS_CPP

#include "generate_txns.hpp"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
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

#endif
