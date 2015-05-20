#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <ostream>
#include <string>
#include <vector>

class Token
{
  friend std::ostream &operator<<(std::ostream &, const Token &);

protected:
  virtual std::string code() const = 0;
};

class Command : Token
{
private:
  std::vector<Token> m_arguments;
};

class Expression : Token
{
private:
  std::vector<Token> m_tokens;
};

std::ostream &operator<<(std::ostream &stream, const Token &token);

#endif
