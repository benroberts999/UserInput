#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct Option {
  std::string key;
  std::string value_str;
};

class InputBlock {
private:
  std::string m_name{};
  std::vector<Option> m_options{};
  std::vector<InputBlock> m_blocks{};

public:
  InputBlock(std::string_view name, std::vector<Option> options) {
    m_name = name;
    m_options = options;
  }
  void add(InputBlock block) { m_blocks.push_back(block); }
  void add(Option option) { m_options.push_back(option); }

  std::string_view name() const { return m_name; }
  const std::vector<Option> &options() const { return m_options; }
  const std::vector<InputBlock> &blocks() const { return m_blocks; }

  template <typename T> T get(std::string_view key, T default_value) const;
  // convert string to type T

  template <typename T = std::string>
  std::optional<T> get(std::string_view key) const;

  std::optional<InputBlock> getBlock(std::string_view name) const;

  void print(std::ostream &os = std::cout, int depth = 0) const;
};

//******************************************************************************
void InputBlock::print(std::ostream &os, int depth) const {

  std::string indent = "";
  for (int i = 0; i < depth; ++i)
    indent += "  ";

  os << indent << m_name << " {";

  const auto multi_entry = (!m_blocks.empty() || (m_options.size() > 1));

  if (multi_entry)
    os << "\n";

  for (const auto &[key, value] : m_options) {
    os << (multi_entry ? indent + "  " : " ");
    os << key << " = " << value << ';';
    os << (multi_entry ? '\n' : ' ');
  }

  for (const auto &block : m_blocks)
    block.print(os, depth + 1);

  if (multi_entry)
    os << indent;

  os << "}\n";
}
