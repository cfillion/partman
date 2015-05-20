#include "generators.hpp"

#include <boost/format.hpp>
#include <yaml-cpp/yaml.h>

#include "error.hpp"

using namespace std;
using namespace boost;

using namespace Generators;

enum KeyType { HEADER, PAPER, SETUP, PARTS, BOOK, SCORE };

const string LILY_VERSION = "2.18.2";

const map<string, KeyType> KEY_TYPES = {
  {"header", HEADER},
  {"paper", PAPER},
  {"setup", SETUP},
  {"parts", PARTS},
  {"book", BOOK},
  {"score", SCORE},
};

const string TRUE_STR = "true";
const string FALSE_STR = "false";

TokenPtr Generators::from_yaml(const YAML::Node &root)
{
  auto version = make_shared<Command>("version");
  *version << make_shared<String>(LILY_VERSION);

  auto out = make_shared<Token>();
  *out << version;

  if(!root["header"])
    *out << default_header();

  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;
    
    if(!KEY_TYPES.count(key)) {
      throw Error(format("invalid key '%s'") % key);
    }

    const KeyType type = KEY_TYPES.at(key);

    switch(type) {
    case HEADER:
      *out << make_header(node);
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

  return out;
}

TokenPtr Generators::make_header(const YAML::Node &root)
{
  if(!root.IsMap())
    throw Error("header must be a map");

  auto block = make_shared<Block>(Block::BRACE);

  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    *block << make_shared<Variable>(key, make_value(node));
  }

  // disable Lilypond's tagline by default
  if(!root["tagline"])
    *block << make_shared<Variable>("tagline", make_shared<Boolean>(false));

  auto cmd = make_shared<Command>("header");
  *cmd << block;

  return cmd;
}

TokenPtr Generators::default_header()
{
  return make_header(YAML::Node(YAML::NodeType::Map));
}

TokenPtr Generators::make_value(const YAML::Node &node)
{
  assert(node.IsScalar());

  const string value = node.Scalar();

  try {
    return make_shared<Boolean>(node.as<bool>());
  }
  catch(YAML::BadConversion) {}

  try {
    node.as<double>();
    return make_shared<Literal>(value);
  }
  catch(YAML::BadConversion) {}

  return make_shared<String>(value);
}
