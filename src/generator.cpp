#include "generator.hpp"

#include <boost/format.hpp>
#include <yaml-cpp/yaml.h>

#include "error.hpp"

using namespace std;
using namespace boost;

const map<string, Generator::KeyType> KEY_TYPES = {
  {"header", Generator::HEADER},
  {"paper", Generator::PAPER},
  {"setup", Generator::SETUP},
  {"parts", Generator::PARTS},
  {"book", Generator::BOOK},
  {"score", Generator::SCORE},
};

TokenPtr Generator::from_yaml(const YAML::Node &root)
{
  auto self = make_shared<Generator>();

  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;
    
    if(!KEY_TYPES.count(key)) {
      throw Error(format("invalid key '%s'") % key);
    }

    const KeyType type = KEY_TYPES.at(key);

    switch(type) {
    case HEADER:
      *self << make_header(node);
      break;
    case PAPER:
      break;
    case SETUP:
      break;
    case PARTS:
      break;
    case BOOK:
      break;
    case SCORE:
      break;
    }
  }

  return self;
}

TokenPtr Generator::make_header(const YAML::Node &node)
{
  auto block = make_shared<Block>(Block::BRACE);

  auto cmd = make_shared<Command>("header");
  *cmd << block;

  return cmd;
}
