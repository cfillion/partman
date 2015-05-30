#include "tokens.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <sstream>

using namespace boost;
using namespace std;

typedef stringstream sstream;

const string NL = "\n";
const string SP = "\x20";

const string LEVEL = SP+SP;
const string COMMENT = "% ";

std::ostream &operator<<(ostream &stream, const TokenPtr<> token)
{
  stream << token->code();
  return stream;
}

Token &operator<<(Token &parent, TokenPtr<> child)
{
  parent.m_children.push_back(child);
  return parent;
}

string Token::code() const
{
  sstream ss;

  for(auto it = children().begin(); it != children().end(); it++) {
    if((*it)->empty())
      continue;

    ss << *it << NL;

    if(it + 1 != children().end())
      ss << NL;
  }

  return ss.str();
}

string Command::code() const
{
  sstream ss;
  ss << "\\" << m_name;

  for(const TokenPtr<> child : children()) {
    if(!child->empty())
      ss << SP << child;
  }

  return ss.str();
}

string Block::code() const
{
  sstream ss;

  switch(m_type) {
  case BraceStyle:
    ss << "{";
    break;
  case BracketStyle:
    ss << "<<";
    break;
  }

  bool first_nl = false;

  for(const TokenPtr<> child : children()) {
    if(child->empty())
      continue;

    if(!first_nl) {
      ss << NL;
      first_nl = true;
    }

    ss << LEVEL << replace_all_copy(child->code(), NL, NL + LEVEL) << NL;
  }

  switch(m_type) {
  case BraceStyle:
    ss << "}";
    break;
  case BracketStyle:
    ss << ">>";
    break;
  }

  return ss.str();
}

string Variable::code() const
{
  return m_name + " = " + m_value->code();
}

string Boolean::code() const
{
  return m_value ? "##t" : "##f";
}

string String::code() const
{
  return "\"" + replace_all_copy(m_value, "\"", "\\\"") + "\"";
}

string Literal::code() const
{
  return m_value;
}

string Function::code() const
{
  sstream ss;
  ss << "#(" << m_name;

  for(const TokenPtr<> child : children())
    ss << SP << child;

  ss << ")";
  return ss.str();
}

string Comment::code() const
{
  const string decoration =
    m_decorate ? string(COMMENT.rbegin(), COMMENT.rend()) : "";

  return COMMENT + replace_all_copy(m_text, NL, decoration + NL + COMMENT)
    + decoration;
}
