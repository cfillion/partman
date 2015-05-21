#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include <boost/bimap.hpp>
#include <map>
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
  TokenPtr<> make_variable(const std::string &, const YAML::Node &) const;
  TokenPtr<> make_value(const YAML::Node &) const;
  std::string id(const std::string &name) const;

  static boost::bimap<std::string, std::string> s_identifiers;

  TokenPtr<> m_token;
};

class Header : public Generator
{
public:
  Header();
  void read_yaml(const YAML::Node &) override;

private:
  TokenPtr<Block> m_block;
  TokenPtr<Variable> m_tagline;
};

class Paper : public Generator
{
public:
  Paper();
  void read_yaml(const YAML::Node &) override;

private:
  TokenPtr<Block> m_block;
  TokenPtr<String> m_paper_size;
};

class Setup : public Generator
{
public:
  Setup();
  void read_yaml(const YAML::Node &) override;

private:
  TokenPtr<Block> m_block;
};

class Part : public Generator
{
public:
  Part(const std::string &name);
  void read_yaml(const YAML::Node &) override;

  const std::string &identifier() const { return m_id; }

private:
  TokenPtr<Block> m_block;

  std::string m_id;
};

class Document : public Generator
{
public:
  Document();
  void read_yaml(const YAML::Node &) override;

private:
  void add_parts_from_yaml(const YAML::Node &);

  TokenPtr<String> m_version;
  Header m_header;
  Paper m_paper;
  Setup m_setup;
  std::map<std::string, Part> m_parts;
};

#endif
