#include "generators.hpp"

#include <boost/format.hpp>
#include <random>
#include <yaml-cpp/yaml.h>

#include "error.hpp"

using namespace std;
using format = boost::format;

const string LILY_VERSION = "2.18.2";

const string TRUE_STR = "true";
const string FALSE_STR = "false";

const int ID_LENGTH = 8;

IdentifierMap Generator::s_identifiers;

TokenPtr<> Generator::make_variable(const std::string &key,
  const YAML::Node &node) const
{
  // TODO: support setting hashes values
  return make_shared<Variable>(key, make_value(node));
}

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

string Generator::id(const std::string &name) const
{
  if(s_identifiers.left.count(name))
    return s_identifiers.left.at(name);

  std::random_device rd;
  std::default_random_engine engine(rd());
  std::uniform_int_distribution<char> rand('a', 'z');

  string random_str;
  while(random_str.size() < ID_LENGTH || s_identifiers.right.count(random_str))
    random_str.push_back(rand(engine));


  const string identifier = "pm_" + random_str + "_" + name;

  s_identifiers.insert(Identifier(name, identifier));

  return identifier;
}

Header::Header()
{
  m_tagline = make_shared<Variable>("tagline", make_shared<Boolean>(false));

  m_block = make_shared<Block>(Block::BRACE);
  *m_block << m_tagline;

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

    if(key == "tagline")
      *m_tagline = make_value(node);
    else
      *m_block << make_variable(key, node);
  }

  if(!root["tagline"])
    *m_block << make_shared<Variable>("tagline", make_shared<Boolean>(false));
}

const string PAPER_SIZE = "paper-size";

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
      *m_block << make_variable(key, node);
  }
}

enum SetupKey { KEY, TIME, TEMPO };

const map<string, SetupKey> SETUP_KEYS = {
  {"key", KEY},
  {"time", TIME},
  {"tempo", TEMPO},
};

Setup::Setup()
{
  m_block = make_shared<Block>(Block::BRACE);
  *m_block << make_shared<Command>("compressFullBarRests");

  m_token = make_shared<Variable>(id("setup"), m_block);
}

void Setup::read_yaml(const YAML::Node &root)
{
  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    if(!SETUP_KEYS.count(key))
      throw Error(format("invalid key '%s'") % key);

    auto command = make_shared<Command>(key);
    *command << make_shared<Literal>(node.as<string>());

    *m_block << command;
  }
}

Part::Part(const std::string &name)
{
  m_id = id(name);

  m_block = make_shared<Block>(Block::BRACE);

  auto command = make_shared<Command>("relative");
  *command << m_block;

  m_token = make_shared<Variable>(m_id, command);
}

void Part::read_yaml(const YAML::Node &root)
{
}

enum DocumentKey { VERSION, HEADER, PAPER, SETUP, PARTS, BOOK, SCORE };

const map<string, DocumentKey> DOCUMENT_KEYS = {
  {"version", VERSION},
  {"header", HEADER},
  {"paper", PAPER},
  {"setup", SETUP},
  {"parts", PARTS},
  {"book", BOOK},
  {"score", SCORE},
};

Document::Document()
{
  m_token = make_shared<Token>();
  m_version = make_shared<String>(LILY_VERSION);

  auto warning = make_shared<Comment>(
    "WARNING: THIS FILE WAS GENERATED BY PARTMAN\n"
    "========== DO NOT EDIT MANUALLY! ==========\n"
    "ALL CHANGES MADE IN THIS FILE WILL BE LOST!"
  , true);

  auto version = make_shared<Command>("version");
  *version << m_version;

  auto point_click = make_shared<Command>("pointAndClickOff");

  *m_token
    << warning
    << version
    << point_click
    << m_header.token()
    << m_paper.token()
    << m_setup.token()
  ;
}

void Document::read_yaml(const YAML::Node &root)
{
  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;
    
    if(!DOCUMENT_KEYS.count(key))
      throw Error(format("invalid key '%s'") % key);

    const DocumentKey type = DOCUMENT_KEYS.at(key);

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
      m_setup.read_yaml(node);
      break;
    case PARTS:
      add_parts_from_yaml(node);
      break;
    case BOOK:
      break;
    case SCORE:
      break;
    }
  }
}

void Document::add_parts_from_yaml(const YAML::Node &root)
{
  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    Part part(key);
    *m_token << part.token();

    m_parts.insert({key, part});
  }
}
