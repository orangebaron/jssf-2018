#include "types.hpp"
#include "types.cpp"
#include <cassert>
using namespace blockchain;

int main() {
  Block *b = new Block({},{});
  assert(b->getValid());
}
