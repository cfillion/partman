#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include <string>

#include "tokens.hpp"

namespace YAML
{
  class Node;
};

namespace Generators {
  TokenPtr from_yaml(const YAML::Node &node);

  TokenPtr make_header(const YAML::Node &node);
  TokenPtr default_header();

  TokenPtr make_value(const YAML::Node &node);
}


#endif
