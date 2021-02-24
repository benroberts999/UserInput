#include "InputBlock.hpp"
#include <iostream>

int main() {

  std::cout << "hello world\n";

  InputBlock ib("name1", {{"key1", "val1"}, {"key2", "val2"}});

  // ib.add({{"key1", "val1"}, {"key2", "val2"}});
  ib.add(InputBlock("blockA", {{"keyA", "valA"}, {"keyA2", "valA2"}}));
  ib.add(InputBlock("blockB", {{"keyB", "valB"}, {"keyB2", "valB2"}}));
  ib.add(InputBlock("blockC", {{"keyC", "valC"}}));
  ib.add(ib);

  ib.print();

  return 0;
}
