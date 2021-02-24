#include "InputBlock.hpp"
#include <iostream>

int main() {

  std::cout << "hello world\n";

  InputBlock ib("name1", {{"key1", "1"}, {"key2", "3.7"}, {"key3", "hello"}});

  // ib.add({{"key1", "val1"}, {"key2", "val2"}});
  ib.add(InputBlock("blockA", {{"keyA", "valA"}, {"keyA2", "valA2"}}));
  ib.add(InputBlock("blockB", {{"keyB1", "valB"}, {"keyB2", "valB2"}}));
  ib.add(InputBlock("blockB", {{"keyB2", "17"}, {"keyB3", "16.3"}}));
  ib.add(InputBlock("blockC", {{"keyC", "valC"}}));
  // ib.add(ib);

  ib.print();

  std::cout << "\n";

  std::cout << ib.get("key1", 0) << " / " << 1 << "\n";
  std::cout << ib.get("key12", 0) << " / " << 0 << "\n";
  std::cout << ib.get("key2", 15.4) << " / " << 3.7 << "\n";
  std::cout << ib.get<std::string>("key3").value() << "\n";

  std::cout << ib.getBlock("blockB")->get("keyB2", 15) << " / " << 17.3 << "\n";

  /*
  To-do:
    - Option to read in vector
    - Parse string into Block
      - separate into {}-encloded strings, and ;-separated list
      - each ;-separated: into 'options'
      - each {}-enclosed: Parse {}-enclosed string into block
    - Clean-up interface, and remove performance issues.
  */

  // std::string block_string =
  //     "name2 {      key1 = 1;  key2 = 3.7;  key3 = hello;  blockA {    keyA =
  //     " "valA;     keyA2 = valA2;  }   blockB {     keyB1 = valB;    keyB2 =
  //     " "valB2;    keyB2 = 17;    keyB3 = 16.3;  }  blockC { keyC = valC;

  std::string block_string = "block{key1=v1; key2 = val2; //key3=val3;\n "
                             "name{k1=v1;}k4=v4;}block{key4=v4;}";

  std::cout << block_string << "\n";

  InputBlock ib2("name", block_string);

  ib2.print();

  return 0;
}
