#include <sys/mman.h>
#include <fcntl.h>
#include <algorithm>

int main() {
  *(char*)mmap(NULL,1/*size*/,PROT_READ|PROT_WRITE,MAP_SHARED,open("mm.txt",O_RDWR),0) = 'a';
  system("./read.out");
}
