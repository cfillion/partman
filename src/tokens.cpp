#include "tokens.hpp"

#include <sstream>

using namespace std;

typedef stringstream sstream;

const string SPACE = "\x20";

std::ostream &operator<<(ostream &stream, const TokenPtr token)
{
  stream << token->code();
  return stream;
}

Token &operator<<(Token &parent, TokenPtr child)
{
  parent.m_children.push_back(child);
  return parent;
}

string Token::code() const
{
  sstream ss;

  for(auto it = children().begin(); it != children().end(); it++) {
    ss << *it << endl;

    if(it + 1 != children().end())
      ss << endl;
  }

  return ss.str();
}

string Command::code() const
{
  sstream ss;
  ss << "\\" << m_name;

  for(const TokenPtr child : children())
    ss << SPACE << child;

  return ss.str();
}

string Block::code() const
{
  sstream ss;

  switch(m_type) {
  case BRACE:
    ss << "{";
    break;
  case BRACKET:
    ss << "<<";
    break;
  }

  ss << endl;

  switch(m_type) {
  case BRACE:
    ss << "}";
    break;
  case BRACKET:
    ss << ">>";
    break;
  }

  return ss.str();
}
