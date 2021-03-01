# InputBlock

A data structure for storing user-input.

Each InputBlock contains:
  * A name
  * A list of options (stored as {key,value} pairs)
  * A list of InputBlocks (recursive)

Everything is stored as strings. Converted to required data-type on retrieval.

Main way to interface:
  * .get<Type>("key", default_value);
    * Ff key "key" exists, returns its value. Otherwise returns default_value
    * Type may be deduced from default_value. Type is std::string by default
  * .get<Type>("key")
    * Returns a std::optional of type Type
    * if key "key" exists, will contain it's value. Otherwise empty
  * .get<Type>({}, "key")
    * Equivalent to above
  * .get<Type>({Block1, Block2, Block3}, "key")
    * For nested blocks:
    * Returns value/optional for "key" that lives in Block3, which lives in Block2, which lives in Block1
  * As well as basic types, can be used for a list of comma-separated input values (returned as std::vector)

You can construct an InputBlock from a string or from a file (or from another InputBlock).
The string uses c++-style braces to separate blocks, and semi-colon to separate options. c++-style comments are ignored.
Example:

Block1{
  option1=value1;
  option2=value2;
  Block2{
    option21=value21;
    option22=value22;
  }
}

See main.cpp for simple example, and test.InputBlock.hpp for full examples.
