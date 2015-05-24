#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include <boost/bimap.hpp>
#include <boost/function.hpp>
#include <string>

#include "tokens.hpp"

namespace YAML
{
  class Node;
};

typedef boost::bimap<std::string, std::string> IdentifierMap;
typedef IdentifierMap::value_type Identifier;

class Generator
{
public:
  virtual void read_yaml(const YAML::Node &node) = 0;
  TokenPtr<> token() const { return m_token; }

protected:
  static boost::bimap<std::string, std::string> s_identifiers;

  TokenPtr<> make_variable(const std::string &, const YAML::Node &) const;
  TokenPtr<> make_value(const YAML::Node &) const;

  std::string id(const std::string &name) const;

  TokenPtr<> m_token;
};

class KeyValue : public Generator
{
public:
  KeyValue();
  void read_yaml(const YAML::Node &) override;

  typedef boost::function<bool(const std::string &,
    const YAML::Node &)> SpecialHandler;

  void add_handler(SpecialHandler h) { m_handlers.push_back(h); }

private:
  std::vector<SpecialHandler> m_handlers;
};

class Part : public Generator
{
public:
  static Part from_yaml(const std::string &, const YAML::Node &);

  Part(const std::string &name);
  void read_yaml(const YAML::Node &) override;

  const std::string &name() const { return m_name; }
  const std::string &identifier() const { return m_id; }
  const TokenPtr<Command> staff() const { return m_staff; }

private:
  void set_names(const YAML::Node &);
  void add_sub_parts(const YAML::Node &);

  TokenPtr<Command> m_staff;
  TokenPtr<Block> m_staff_block;
  TokenPtr<Block> m_with_block;
  TokenPtr<Block> m_music_block;
  TokenPtr<Literal> m_type;
  TokenPtr<String> m_long_name;
  TokenPtr<String> m_short_name;
  TokenPtr<String> m_instrument;

  std::string m_name;
  std::string m_id;
};

class Document : public Generator
{
public:
  Document();
  void read_yaml(const YAML::Node &) override;

private:
  void prepare_header();
  void prepare_paper();
  void prepare_setup();

  void add_parts(const YAML::Node &);

  TokenPtr<> make_score(const YAML::Node &) const;
  TokenPtr<> make_part_ref(const YAML::Node &) const;
  TokenPtr<> make_book(const YAML::Node &) const;

  TokenPtr<String> m_version;

  KeyValue m_header;
  KeyValue m_paper;
  KeyValue m_setup;
};

#endif
