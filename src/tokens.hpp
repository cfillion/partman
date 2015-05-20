#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <ostream>
#include <string>
#include <vector>

class Token;
typedef std::shared_ptr<Token> TokenPtr;

class Token
{
  friend std::ostream &operator<<(std::ostream &, const TokenPtr);
  friend Token &operator<<(Token &, TokenPtr);

public:

protected:
  const std::vector<TokenPtr> &children() const { return m_children; }
  virtual std::string code() const;

private:
  std::vector<TokenPtr> m_children;
};

std::ostream &operator<<(std::ostream &, const TokenPtr);
Token &operator<<(Token &, TokenPtr);

class Command : public Token
{
public:
  Command(std::string name) : m_name(name) {}

protected:
  virtual std::string code() const override;

private:
  std::string m_name;
};

#endif
