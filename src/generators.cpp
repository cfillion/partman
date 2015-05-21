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

enum SetupKey { S_KEY, S_TIME, S_TEMPO };

const map<string, SetupKey> SETUP_KEYS = {
  {"key", S_KEY},
  {"time", S_TIME},
  {"tempo", S_TEMPO},
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

enum PartKey { P_NAME, P_TYPE, P_RELATIVE, P_INSTRUMENT, P_PARTS };

const map<string, PartKey> PART_KEYS = {
  {"name", P_NAME},
  {"type", P_TYPE},
  {"relative", P_RELATIVE},
  {"instrument", P_INSTRUMENT},
  {"parts", P_PARTS},
};

Part Part::from_yaml(const std::string &name, const YAML::Node &node)
{
  Part part(name);
  part.read_yaml(node);
  return part;
}

Part::Part(const std::string &name)
  : m_name(name)
{
  m_id = id(name);

  m_type = make_shared<Literal>("Staff");

  m_long_name = make_shared<String>("");
  m_short_name = make_shared<String>();
  m_instrument = make_shared<String>();

  m_with_block = make_shared<Block>(Block::BRACE);
  *m_with_block << make_shared<Variable>("instrumentName", m_long_name);
  *m_with_block << make_shared<Variable>("shortInstrumentName", m_short_name);
  *m_with_block << make_shared<Variable>("midiInstrument", m_instrument);

  auto with = make_shared<Command>("with");
  *with << m_with_block;

  m_staff_block = make_shared<Block>(Block::BRACKET);

  m_staff = make_shared<Command>("new");
  *m_staff << m_type;
  *m_staff << with;
  *m_staff << m_staff_block;

  auto include = make_shared<Command>("include");
  *include << make_shared<String>("parts/" + name + ".ily");

  m_music_block = make_shared<Block>(Block::BRACE);
  *m_music_block << make_shared<Command>(id("setup"));
  *m_music_block << include;

  m_token = make_shared<Variable>(m_id, m_staff);
}

void Part::read_yaml(const YAML::Node &root)
{
  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    if(!PART_KEYS.count(key))
      throw Error(format("invalid key '%s'") % key);

    const PartKey type = PART_KEYS.at(key);

    switch(type) {
      case P_NAME:
        set_names_from_yaml(node);
        break;
      case P_TYPE:
        *m_type = node.as<string>();
        break;
      case P_RELATIVE: {
        auto relative = make_shared<Command>("relative");
        *relative << make_shared<Literal>(node.as<string>());
        *relative << m_music_block;
        *m_staff_block << relative;
        break;
      }
      case P_INSTRUMENT:
        *m_instrument = node.as<string>();
        break;
      case P_PARTS:
        add_sub_parts_from_yaml(node);
        break;
    }
  }
}

void Part::set_names_from_yaml(const YAML::Node &node)
{
  if(node.IsSequence()) {
    vector<string> names = node.as<vector<string> >();

    if(names.size() > 2) {
      throw Error(format("expected at most two names, but got %d instead")
        % names.size());
    }

    names.resize(2);

    *m_long_name = names[0];
    *m_short_name = names[1];
  }
  else
    *m_long_name = node.as<string>();
}

void Part::add_sub_parts_from_yaml(const YAML::Node &root)
{
  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    *m_staff_block << Part::from_yaml(key, node).staff();
  }
}

enum DocumentKey { D_VERSION, D_HEADER, D_PAPER, D_SETUP,
  D_PARTS, D_BOOK, D_SCORE };

const map<string, DocumentKey> DOCUMENT_KEYS = {
  {"version", D_VERSION},
  {"header", D_HEADER},
  {"paper", D_PAPER},
  {"setup", D_SETUP},
  {"parts", D_PARTS},
  {"book", D_BOOK},
  {"score", D_SCORE},
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
    case D_VERSION:
      *m_version = node.as<string>();
      break;
    case D_HEADER:
      m_header.read_yaml(node);
      break;
    case D_PAPER:
      m_paper.read_yaml(node);
      break;
    case D_SETUP:
      m_setup.read_yaml(node);
      break;
    case D_PARTS:
      add_parts_from_yaml(node);
      break;
    case D_BOOK:
      break;
    case D_SCORE:
      break;
    }
  }
}

void Document::add_parts_from_yaml(const YAML::Node &root)
{
  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    *m_token << Part::from_yaml(key, node).token();
  }
}
