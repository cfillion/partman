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

  std::string code() const override;

  static Generator from_yaml(YAML::Node &node);
};
#endif
