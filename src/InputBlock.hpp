#pragma once
#include <algorithm>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

//******************************************************************************
inline std::string removeSpaces(std::string lines) {

  // remove spaces
  lines.erase(std::remove_if(lines.begin(), lines.end(),
                             [](unsigned char x) { return x == ' '; }),
              lines.end());
  // remove tabs
  lines.erase(std::remove_if(lines.begin(), lines.end(),
                             [](unsigned char x) { return x == '\t'; }),
              lines.end());
  // remove newlines
  lines.erase(std::remove_if(lines.begin(), lines.end(),
                             [](unsigned char x) { return x == '\n'; }),
              lines.end());

  // remove ' and "
  lines.erase(std::remove_if(lines.begin(), lines.end(),
                             [](unsigned char x) { return x == '\''; }),
              lines.end());
  lines.erase(std::remove_if(lines.begin(), lines.end(),
                             [](unsigned char x) { return x == '\"'; }),
              lines.end());

  return lines;
}

//******************************************************************************
inline void removeBlockComments(std::string &input) {
  for (auto posi = input.find("/*"); posi != std::string::npos;
       posi = input.find("/*")) {
    auto posf = input.find("*/");
    if (posf != std::string::npos) {
      input = input.substr(0, posi) + input.substr(posf + 2);
    } else {
      input = input.substr(0, posi);
    }
  }
}

//******************************************************************************
inline std::string removeComments(const std::string &input) {
  std::string lines = "";
  {
    std::string line;
    std::stringstream stream1(input);
    while (std::getline(stream1, line, '\n')) {
      auto comm1 = line.find('!'); // nb: char, NOT string literal!
      auto comm2 = line.find('#');
      auto comm3 = line.find("//"); // str literal here
      auto comm = std::min(comm1, std::min(comm2, comm3));
      lines += line.substr(0, comm);
      lines += '\n';
    }
  }
  removeBlockComments(lines);

  return lines;
}

//******************************************************************************
struct Option {
  std::string key;
  std::string value_str;

  friend bool operator==(Option option, std::string_view key) {
    return option.key == key;
  }
  friend bool operator==(std::string_view key, Option option) {
    return option == key;
  }
  friend bool operator!=(Option option, std::string_view key) {
    return !(option == key);
  }
  friend bool operator!=(std::string_view key, Option option) {
    return !(option == key);
  }
};

//******************************************************************************
class InputBlock {
private:
  std::string m_name{};
  std::vector<Option> m_options{};
  std::vector<InputBlock> m_blocks{};

public:
  InputBlock(std::string_view name, std::vector<Option> options = {})
      : m_name(name), m_options(options) {}

  InputBlock(std::string_view name, const std::string &string_input)
      : m_name(name) {
    auto fixed_string = removeSpaces(removeComments(string_input));
    add_block(fixed_string);
  }

  void add(InputBlock block) {
    auto existing_block = getBlock_ptr(block.m_name);
    if (existing_block) {
      std::cout << block.m_name << " yes\n";
      existing_block->m_options.insert(existing_block->m_options.end(),
                                       block.m_options.cbegin(),
                                       block.m_options.cend());
    } else {
      std::cout << block.m_name << " no\n";
      m_blocks.push_back(block);
    }
  }

  void add(Option option) { m_options.push_back(option); }

  std::string_view name() const { return m_name; }
  const std::vector<Option> &options() const { return m_options; }
  const std::vector<InputBlock> &blocks() const { return m_blocks; }

  friend bool operator==(InputBlock block, std::string_view name) {
    return block.m_name == name;
  }
  friend bool operator==(std::string_view name, InputBlock block) {
    return block == name;
  }

  template <typename T>
  static T convert_str_to(const std::string &value_as_str) {
    if constexpr (std::is_same_v<T, std::string>) {
      // already a string, just return value
      return value_as_str;
    } else {
      // T is not a string: convert using stringstream
      T value_T;
      std::stringstream ss(value_as_str);
      ss >> value_T;
      return value_T;
    }
  }

  template <typename T> T get(std::string_view key, T default_value) const {
    return get<T>(key).value_or(default_value);
  }

  template <typename T = std::string>
  std::optional<T> get(std::string_view key) const {
    // Use reverse iterators so that we find _last_ option that matches key
    // i.e., assume later options override earlier ones.
    const auto option = std::find(m_options.crbegin(), m_options.crend(), key);
    if (option == m_options.crend())
      return {};
    return convert_str_to<T>(option->value_str);
  }

  InputBlock *getBlock_ptr(std::string_view name) {
    auto block = std::find(m_blocks.rbegin(), m_blocks.rend(), name);
    if (block == m_blocks.rend())
      return nullptr;
    return &(*block);
  }

  std::optional<InputBlock> getBlock(std::string_view name) const {
    // std::cout << "getBlock: " << name << "\n";
    const auto block = std::find(m_blocks.crbegin(), m_blocks.crend(), name);
    if (block == m_blocks.crend())
      return {};
    return *block;
  }

  std::optional<Option> getOption(std::string_view key) const {
    // Use reverse iterators so that we find _last_ option that matches key
    // i.e., assume later options override earlier ones.
    const auto option = std::find(m_options.crbegin(), m_options.crend(), key);
    if (option != m_options.crend())
      return *option;
    return {};
  }

  void print(std::ostream &os = std::cout, int depth = 0) const;

  void add_option(std::string_view in_string) {
    const auto pos = in_string.find('=');
    const auto option = in_string.substr(0, pos);
    const auto value =
        pos < in_string.length() ? in_string.substr(pos + 1) : "";
    m_options.push_back({std::string(option), std::string(value)});
  }

  void add_block(std::string_view string);
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

//******************************************************************************
void InputBlock::add_block(std::string_view string) {

  std::cout << "\n";

  auto start = 0ul;

  while (true) {

    if (start > string.length())
      break;

    auto end = std::min(string.find(';', start), string.find('{', start));

    if (end > string.length() || start >= end)
      break;

    if (string.at(end) == ';') {

      this->add_option(string.substr(start, end - start));

    } else {

      const auto block_name = string.substr(start, end - start);
      start = end + 1;

      int count = 1;
      auto next_start = start + 1;
      while (count != 0) {
        if (next_start > string.length())
          break;
        const auto next_end = std::min(string.find('{', next_start),
                                       string.find('}', next_start));
        if (next_end > string.length())
          break;

        if (string.at(next_end) == '{')
          ++count;
        else
          --count;

        if (count == 0) {
          end = next_end;
          break;
        }
        if (count == 100) {
          std::cout << "Depth error\n";
          end = next_end;
          break;
        }
        next_start = next_end + 1;
      }

      auto &block = m_blocks.emplace_back(block_name);
      block.add_block(string.substr(start, end - start));
    }

    start = end + 1;
  }
}
