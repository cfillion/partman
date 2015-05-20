#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include <map>
#include <string>

#include "tokens.hpp"

namespace YAML
{
  class Node;
};

class Generator
{
public:
  virtual void read_yaml(const YAML::Node &node) = 0;
  TokenPtr<> token() const { return m_token; }

protected:
  TokenPtr<> make_value(const YAML::Node &node) const;
  TokenPtr<> m_token;
};

class Header : public Generator
{
public:
  Header();
  void read_yaml(const YAML::Node &node) override;

private:
  TokenPtr<Block> m_block;
};

class Document : public Generator
{
public:
  Document();
  void read_yaml(const YAML::Node &node) override;

private:
  TokenPtr<String> m_version;
  Header m_header;
};

#endif
