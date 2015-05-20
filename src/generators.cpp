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
const string PAPER_SIZE = "paper-size";

TokenPtr<> Generator::make_value(const YAML::Node &node) const
{
  assert(node.IsScalar());

  const string value = node.Scalar();

  try {
    return make_shared<Boolean>(node.as<bool>());
  }
  catch(YAML::BadConversion) {}

  try {
    if(value.find('\\') != string::npos ||
        value.find('#') == 0 || node.as<double>())
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

Paper::Paper()
{
  m_paper_size = make_shared<String>("letter");

  auto ps_func = make_shared<Function>("set-paper-size");
  *ps_func << m_paper_size;

  m_block = make_shared<Block>(Block::BRACE);
  *m_block << ps_func;

  m_token = make_shared<Command>("paper");
  *m_token << m_block;
}

void Paper::read_yaml(const YAML::Node &root)
{
  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    if(key == PAPER_SIZE)
      *m_paper_size = node.as<string>();
    else
      *m_block << make_shared<Variable>(key, make_value(node));
  }
}

Document::Document()
{
  m_token = make_shared<Token>();
  m_version = make_shared<String>(LILY_VERSION);

  auto version = make_shared<Command>("version");
  *version << m_version;

  *m_token
    << version
    << m_header.token()
    << m_paper.token()
  ;
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
      m_paper.read_yaml(node);
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
