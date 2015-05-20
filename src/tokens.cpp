#include "tokens.hpp"

#include <sstream>

using namespace std;

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
  stringstream stream;

  for(const TokenPtr child : children())
    stream << child << endl;

  return stream.str();
}

string Command::code() const
{
  stringstream stream;
  stream << "\\" << m_name << SPACE;

  return stream.str();
}
