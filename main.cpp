#include "InputBlock.hpp"
#include "test.InputBlock.hpp"
#include <iostream>

int main() {

  // Basic example, we will read in from "example.txt"
  std::ifstream in_file("example.in");
  UserIO::InputBlock input("examle1", in_file);

  std::cout << "Print input in user-fiendly form:\n";
  input.print();

  std::cout << "\nSome simple examples:\n";
  // here, 0.0 is default. if 'g' not found, will return 0.0
  std::cout << input.get("g", 0.0) << "\n";

  // If no default given, will return std::optional
  // Need to give 'type' this time (otherwise, deduced from default)
  auto optional_g = input.get<double>("g");
  if (optional_g) {
    std::cout << "Found g\n";
    std::cout << optional_g.value() << "\n";
  } else {
    std::cout << "g not found\n";
  }
  auto optional_h = input.get<double>("h");
  if (optional_h) {
    std::cout << "Found h\n";
    std::cout << optional_h.value() << "\n";
  } else {
    std::cout << "h not found\n";
  }

  // get a block (also as a std::optional).
  auto cat_block = input.getBlock("Cat");
  if (cat_block != std::nullopt) {
    std::cout << "Have Cat block:";
    std::cout << cat_block->get("mass", 0.0) << "\n";
  }

  std::cout << input.get({"Dog"}, "mass", 0.0) << "\n";
  std::cout << input.get({"Dog"}, "speed", 0.0) << "\n";
  std::cout << input.get({"Dog", "Puppy"}, "mass", 0.0) << "\n";
  std::cout << input.get({"Cat"}, "mass", 0.0) << "\n";
  std::cout << input.get({"Cat"}, "speed", 0.0) << "\n";
  // Not in list, so get default:
  std::cout << input.get({"Elephant"}, "speed", 0.0) << "\n";

  std::cout << "List input:\n";
  // the {0,0,0} is the default list
  const auto list1 = input.get<std::vector<int>>("list", {0, 0, 0});
  for (auto &x : list1)
    std::cout << x << ", ";
  std::cout << "\n";
  const auto list2 = input.get<std::vector<double>>({"Cat"}, "list", {0.0});
  for (auto &x : list2)
    std::cout << x << ", ";
  std::cout << "\n";

  std::cout << "\nFor full examples: see test code:\n";
  test_InputBlock();

  return 0;
}
