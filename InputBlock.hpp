#pragma once
#include <algorithm>
#include <fstream>
#include <iostream>
#include <istream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace UserIO {

//******************************************************************************
//! Removes all white space (space, tab, newline), AND quote marks!
inline std::string removeSpaces(std::string lines);

//! Removes all c++ style block comments from a string
inline void removeBlockComments(std::string &input);

//! Removes all c++ style comments from a string (block and line)
inline std::string removeComments(const std::string &input);

//! Parses a string to type T by stringstream
template <typename T> T parse_str_to_T(const std::string &value_as_str);

//! Parses entire file into string. Note: v. inefficient
inline std::string file_to_string(const std::istream &file);

//******************************************************************************
//! Simple struct; holds key-value pair, both strings. == compares key
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
//! Holds list of Options, and a list of other InputBlocks. Can be initialised
//! with a list of options, with a string, or from a file (ifstream).
//! Format for input is, e.g.,:
/*!
 BlockName1{
   option1=value1;
   option2=value2;
   InnerBlock{
     option1=v3;
   }
 }
*/
class InputBlock {
private:
  std::string m_name{};
  std::vector<Option> m_options{};
  std::vector<InputBlock> m_blocks{};

public:
  InputBlock(){};

  InputBlock(std::string_view name, std::initializer_list<Option> options = {})
      : m_name(name), m_options(options) {}

  InputBlock(std::string_view name, const std::string &string_input)
      : m_name(name) {
    add(string_input);
  }

  InputBlock(std::string_view name, const std::istream &file) : m_name(name) {
    add(file_to_string(file));
  }

  //! Add a new InputBlock (will be merged with existing if names match)
  void add(InputBlock block);
  //! Adds a new option to end of list
  void add(Option option);
  //! Adds options/inputBlocks by parsing a string
  void add(const std::string &string);

  std::string_view name() const { return m_name; }
  const std::vector<Option> &options() const { return m_options; }
  const std::vector<InputBlock> &blocks() const { return m_blocks; }

  //! Comparison of blocks compares the 'name'
  friend bool operator==(InputBlock block, std::string_view name);
  friend bool operator==(std::string_view name, InputBlock block);
  friend bool operator!=(InputBlock block, std::string_view name);
  friend bool operator!=(std::string_view name, InputBlock block);

  //! If 'key' exists in the options, returns value. Else, returns
  //! default_value. Note: If two keys with same name, will use the later
  template <typename T> T get(std::string_view key, T default_value) const;

  //! Returns optional value. Contains value if key exists; empty otherwise.
  //! Note: If two keys with same name, will use the later
  template <typename T = std::string>
  std::optional<T> get(std::string_view key) const;

  //! Get value from set of nested blocks. .get({block1,block2},option)
  template <typename T>
  T get(std::initializer_list<std::string> blocks, std::string_view key,
        T default_value) const;
  //! As above, but without default value
  template <typename T>
  std::optional<T> get(std::initializer_list<std::string> blocks,
                       std::string_view key) const;

  //! Returns optional InputBlock. Contains InputBlock if block of given name
  //! exists; empty otherwise.
  std::optional<InputBlock> getBlock(std::string_view name) const;

  //! Get an 'Option' (kay, value) - rarely needed
  std::optional<Option> getOption(std::string_view key) const;

  //! Prints options to screen in user-friendly form. Same form as input string.
  //! By default prints to cout, but can be given any ostream
  void print(std::ostream &os = std::cout, int indent_depth = 0) const;

private:
  InputBlock *getBlock_ptr(std::string_view name);
  const InputBlock *getBlock_cptr(std::string_view name) const;

  void add_option(std::string_view in_string);
  void add_blocks_from_string(std::string_view string);
  void consolidate();
};

//******************************************************************************
//******************************************************************************
void InputBlock::add(InputBlock block) {
  auto existing_block = getBlock_ptr(block.m_name);
  if (existing_block) {
    existing_block->m_options.insert(existing_block->m_options.end(),
                                     block.m_options.cbegin(),
                                     block.m_options.cend());
  } else {
    m_blocks.push_back(block);
  }
}

//******************************************************************************
void InputBlock::add(Option option) { m_options.push_back(option); }

//******************************************************************************
void InputBlock::add(const std::string &string) {
  add_blocks_from_string(removeSpaces(removeComments(string)));
}

//******************************************************************************
bool operator==(InputBlock block, std::string_view name) {
  return block.m_name == name;
}
bool operator==(std::string_view name, InputBlock block) {
  return block == name;
}
bool operator!=(InputBlock block, std::string_view name) {
  return !(block == name);
}
bool operator!=(std::string_view name, InputBlock block) {
  return !(block == name);
}

//******************************************************************************
template <typename T>
T InputBlock::get(std::string_view key, T default_value) const {
  return get<T>(key).value_or(default_value);
}

template <typename T = std::string>
std::optional<T> InputBlock::get(std::string_view key) const {
  // Use reverse iterators so that we find _last_ option that matches key
  // i.e., assume later options override earlier ones.
  const auto option = std::find(m_options.crbegin(), m_options.crend(), key);
  if (option == m_options.crend())
    return {};
  return parse_str_to_T<T>(option->value_str);
}

template <typename T>
T InputBlock::get(std::initializer_list<std::string> blocks,
                  std::string_view key, T default_value) const {
  return get<T>(blocks, key).value_or(default_value);
}

template <typename T = std::string>
std::optional<T> InputBlock::get(std::initializer_list<std::string> blocks,
                                 std::string_view key) const {
  // Find key in nested blocks
  const InputBlock *pB = this;
  for (const auto &block : blocks) {
    pB = pB->getBlock_cptr(block);
    if (pB == nullptr)
      return std::nullopt;
  }
  return pB->get<T>(key);
}

//******************************************************************************
std::optional<InputBlock> InputBlock::getBlock(std::string_view name) const {
  // note: by copy!
  const auto block = std::find(m_blocks.crbegin(), m_blocks.crend(), name);
  if (block == m_blocks.crend())
    return {};
  return *block;
}

//******************************************************************************
std::optional<Option> InputBlock::getOption(std::string_view key) const {
  // Use reverse iterators so that we find _last_ option that matches key
  // i.e., assume later options override earlier ones.
  const auto option = std::find(m_options.crbegin(), m_options.crend(), key);
  if (option != m_options.crend())
    return *option;
  return {};
}

//******************************************************************************
void InputBlock::print(std::ostream &os, int depth) const {

  std::string indent = "";
  for (int i = 1; i < depth; ++i)
    indent += "  ";

  // Don't print outer-most name
  if (depth != 0)
    os << indent << m_name << " { ";

  const auto multi_entry = (!m_blocks.empty() || (m_options.size() > 1));

  if (depth != 0 && multi_entry)
    os << "\n";

  for (const auto &[key, value] : m_options) {
    os << (depth != 0 && multi_entry ? indent + "  " : "");
    os << key << " = " << value << ';';
    os << (multi_entry ? '\n' : ' ');
  }

  for (const auto &block : m_blocks)
    block.print(os, depth + 1);

  if (depth != 0 && multi_entry)
    os << indent;

  if (depth != 0)
    os << "}\n";
}

//******************************************************************************
void InputBlock::add_blocks_from_string(std::string_view string) {

  // Expects that string has comments and spaces removed already

  auto start = 0ul;
  while (start < string.length()) {

    // Find the first of either next ';' or open '{'
    // This is the end of the next input option, or start of block
    auto end = std::min(string.find(';', start), string.find('{', start));
    if (end > string.length() || start >= end)
      break;

    if (string.at(end) == ';') {
      // end of option:

      this->add_option(string.substr(start, end - start));

    } else {
      // start of block

      // 'name' directly preceeds "{"
      const auto block_name = string.substr(start, end - start);
      start = end + 1;

      // Now, find *matching* close '}' - ensure balanced
      int depth_count = 1;
      auto next_start = start + 1;
      while (depth_count != 0) {
        if (next_start > string.length())
          break;
        const auto next_end = std::min(string.find('{', next_start),
                                       string.find('}', next_start));
        if (next_end > string.length())
          break;

        // count depth of bracket nesting:
        if (string.at(next_end) == '{')
          ++depth_count;
        else
          --depth_count;

        if (depth_count == 0) {
          end = next_end;
          break;
        }
        if (depth_count > 100) {
          std::cerr << "FAIL 271 in InputBlock::add_blocks_from_string: Depth "
                       "error. Check balanced {} in input\n";
          end = next_end;
          break;
        }
        next_start = next_end + 1;
      }

      // Add a new block, populate it with string. Recursive, since blocks may
      // contain blocks
      auto &block = m_blocks.emplace_back(block_name);
      block.add_blocks_from_string(string.substr(start, end - start));
    }

    start = end + 1;
  }
  // Merge duplicated blocks.
  consolidate();
}

//******************************************************************************
void InputBlock::add_option(std::string_view in_string) {
  const auto pos = in_string.find('=');
  const auto option = in_string.substr(0, pos);
  const auto value = pos < in_string.length() ? in_string.substr(pos + 1) : "";
  m_options.push_back({std::string(option), std::string(value)});
}

//******************************************************************************
InputBlock *InputBlock::getBlock_ptr(std::string_view name) {
  auto block = std::find(m_blocks.rbegin(), m_blocks.rend(), name);
  if (block == m_blocks.rend())
    return nullptr;
  return &(*block);
}

const InputBlock *InputBlock::getBlock_cptr(std::string_view name) const {
  auto block = std::find(m_blocks.crbegin(), m_blocks.crend(), name);
  if (block == m_blocks.rend())
    return nullptr;
  return &(*block);
}

//******************************************************************************
void InputBlock::consolidate() {
  for (auto bl = m_blocks.end() - 1; bl != m_blocks.begin() - 1; --bl) {
    bl->consolidate();
    auto bl2 = std::find(m_blocks.begin(), bl, bl->name());
    if (bl2 != bl) {
      bl2->m_options.insert(bl2->m_options.end(), bl->m_options.cbegin(),
                            bl->m_options.cend());
      m_blocks.erase(bl);
    }
  }
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

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
template <typename T> T parse_str_to_T(const std::string &value_as_str) {
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

//******************************************************************************
inline std::string file_to_string(const std::istream &file) {
  std::string out;
  if (!file)
    return "";
  // Horribly inneficient...
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

} // namespace UserIO
