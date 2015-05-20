#include "generators.hpp"

#include <boost/format.hpp>
#include <yaml-cpp/yaml.h>

#include "error.hpp"

using namespace std;
using namespace boost;

enum KeyType { VERSION, HEADER, PAPER, SETUP, PARTS, BOOK, SCORE };

const string LILY_VERSION = "2.18.2";

const map<string, KeyType> KEY_TYPES = {
  {"version", VERSION},
  {"header", HEADER},
  {"paper", PAPER},
  {"setup", SETUP},
  {"parts", PARTS},
  {"book", BOOK},
  {"score", SCORE},
};

const string TRUE_STR = "true";
const string FALSE_STR = "false";

TokenPtr<> Generator::make_value(const YAML::Node &node) const
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

Header::Header()
{
  m_block = make_shared<Block>(Block::BRACE);

  m_token = make_shared<Command>("header");
  *m_token << m_block;
}

void Header::read_yaml(const YAML::Node &root)
{
  if(!root.IsMap())
    throw Error("header must be a map");

  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    *m_block << make_shared<Variable>(key, make_value(node));
  }

  if(!root["tagline"])
    *m_block << make_shared<Variable>("tagline", make_shared<Boolean>(false));
}

Document::Document()
{
  m_token = make_shared<Token>();
  m_version = make_shared<String>(LILY_VERSION);

  auto version = make_shared<Command>("version");
  *version << m_version;
  *m_token << version;

  *m_token << m_header.token();
}

void Document::read_yaml(const YAML::Node &root)
{
  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;
    
    if(!KEY_TYPES.count(key)) {
      throw Error(format("invalid key '%s'") % key);
    }

    const KeyType type = KEY_TYPES.at(key);

    switch(type) {
    case VERSION:
      *m_version = node.as<string>();
      break;
    case HEADER:
      m_header.read_yaml(node);
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
}
