#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <string>

#include "tokens.hpp"

namespace YAML
{
  class Node;
};

class Generator : public Token
{
public:
  enum KeyType { HEADER, PAPER, SETUP, PARTS, BOOK, SCORE };

  static TokenPtr from_yaml(const YAML::Node &node);

private:
  static TokenPtr make_header(const YAML::Node &node);
};
#endif
