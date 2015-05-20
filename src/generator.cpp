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

string Generator::code() const
{
  return "";
}

Generator Generator::from_yaml(YAML::Node &node)
{
  Generator self;

  for(auto it = node.begin(); it != node.end(); it++) {
    const string key = it->first.as<string>();
    
    if(!KEY_TYPES.count(key)) {
      throw Error(format("invalid key '%s'") % key);
    }

    const KeyType type = KEY_TYPES.at(key);

    switch(type) {
    case HEADER:
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
