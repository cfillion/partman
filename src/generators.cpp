#include "generators.hpp"

#include <boost/format.hpp>
#include <random>
#include <unordered_set>
#include <yaml-cpp/yaml.h>

#include "error.hpp"

using namespace std;
using format = boost::format;

const string LILY_VERSION = "2.18.2";

const string TRUE_STR = "true";
const string FALSE_STR = "false";

const int ID_LENGTH = 8;

const unordered_set<string> KEY_COMMANDS = { "key", "time", "tempo" };

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

KeyValue::KeyValue()
{
  m_token = make_shared<Block>(Block::BRACE);
}

void KeyValue::read_yaml(const YAML::Node &root)
{
  if(!root.IsMap())
    throw Error("keyvalue block must be a map");

  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    bool append = true;

    for(const SpecialHandler &h : m_handlers)
      if(!(append = append && h(key, node))) break;

    if(!append) continue;

    if(KEY_COMMANDS.count(key)) {
      auto command = make_shared<Command>(key);
      *command << make_shared<Literal>(node.as<string>());
      *m_token << command;
    }
    else
      *m_token << make_variable(key, node);
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
  else {
    const string name = node.as<string>();
    *m_long_name = name;
    *m_short_name = name;
  }
}

void Part::add_sub_parts_from_yaml(const YAML::Node &root)
{
  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    *m_staff_block << Part::from_yaml(m_name + "_" + key, node).staff();
  }
}

enum DocumentKey { D_VERSION, D_HEADER, D_PAPER, D_SETUP,
  D_PARTS, D_BOOK, D_SCORE , D_GSTAFF_SIZE };

const map<string, DocumentKey> DOCUMENT_KEYS = {
  {"version", D_VERSION},
  {"header", D_HEADER},
  {"paper", D_PAPER},
  {"setup", D_SETUP},
  {"parts", D_PARTS},
  {"book", D_BOOK},
  {"score", D_SCORE},
  {"global-staff-size", D_GSTAFF_SIZE},
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
  ;

  prepare_header();
  prepare_paper();
  prepare_setup();
}

void Document::prepare_header()
{
  auto tagline = make_shared<Variable>("tagline", make_shared<Boolean>(false));
  *m_header.token() << tagline;

  m_header.add_handler([=](const string &key, const YAML::Node &node) {
    if(key == "tagline") {
      *tagline = make_value(node);
      return false;
    }

    return true;
  });

  auto header_cmd = make_shared<Command>("header");
  *header_cmd << m_header.token();
  *m_token << header_cmd;
}

void Document::prepare_paper()
{
  auto paper_size = make_shared<String>("letter");
  auto ps_func = make_shared<Function>("set-paper-size");
  *ps_func << paper_size;
  *m_paper.token() << ps_func;

  m_paper.add_handler([=](const string &key, const YAML::Node &node) {
    if(key == "paper-size") {
      *paper_size = node.as<string>();
      return false;
    }

    return true;
  });

  auto paper_cmd = make_shared<Command>("paper");
  *paper_cmd << m_paper.token();
  *m_token << paper_cmd;
}

void Document::prepare_setup()
{
  *m_setup.token() << make_shared<Command>("compressFullBarRests");

  auto setup = make_shared<Variable>(id("setup"), m_setup.token());
  *m_token << setup;
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
      add_parts(node);
      break;
    case D_BOOK:
      *m_token << make_book(node);
      break;
    case D_SCORE:
      *m_token << make_score(node);
      break;
    case D_GSTAFF_SIZE:
      auto func = make_shared<Function>("set-global-staff-size");
      *func << make_value(node);
      *m_token << func;
      break;
    }
  }
}

void Document::add_parts(const YAML::Node &root)
{
  for(auto it = root.begin(); it != root.end(); it++) {
    const string key = it->first.as<string>();
    const YAML::Node node = it->second;

    *m_token << Part::from_yaml(key, node).token();
  }
}

TokenPtr<> Document::make_score(const YAML::Node &root) const
{
  if(!root["parts"].IsSequence())
    throw Error("score.parts must be an array");

  auto music_block = make_shared<Block>(Block::BRACKET);

  for(auto it = root["parts"].begin(); it != root["parts"].end(); it++)
    *music_block << make_part_ref(*it);

  auto score_block = make_shared<Block>(Block::BRACE);
  *score_block << music_block;

  if(root["layout"]) {
    KeyValue block;

    if(root["layout"].IsMap())
      block.read_yaml(root["layout"]);

    auto layout = make_shared<Command>("layout");
    *layout << block.token();
    *score_block << layout;
  }

  if(root["midi"]) {
    KeyValue block;

    if(root["midi"].IsMap())
      block.read_yaml(root["midi"]);

    auto midi = make_shared<Command>("midi");
    *midi << block.token();
    *score_block << midi;
  }

  auto score = make_shared<Command>("score");
  *score << score_block;

  return score;
}

TokenPtr<> Document::make_part_ref(const YAML::Node &node) const
{
  if(node.IsSequence()) {
    auto block = make_shared<Block>(Block::BRACKET);

    for(auto it = node.begin(); it != node.end(); it++)
      *block << make_part_ref(*it);

    auto group = make_shared<Command>("new");
    *group << make_shared<Literal>("StaffGroup");
    *group << block;

    return group;
  }
  else
    return make_shared<Command>(id(node.as<string>()));
}

TokenPtr<> Document::make_book(const YAML::Node &node) const
{
  if(!node.IsSequence())
    throw Error("a book must be an array of scores");

  auto block = make_shared<Block>(Block::BRACE);

  for(auto it = node.begin(); it != node.end(); it++)
    *block << make_score(*it);

  auto book = make_shared<Command>("book");
  *book << block;

  return book;
}
