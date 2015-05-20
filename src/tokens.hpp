#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <ostream>
#include <string>
#include <vector>

class Token;
typedef std::shared_ptr<Token> TokenPtr;

class Token
{
  // friend std::ostream &operator<<(std::ostream &, const TokenPtr);
  friend Token &operator<<(Token &, TokenPtr);

public:
  virtual std::string code() const;

protected:
  const std::vector<TokenPtr> &children() const { return m_children; }

private:
  std::vector<TokenPtr> m_children;
};

std::ostream &operator<<(std::ostream &, const TokenPtr);
Token &operator<<(Token &, TokenPtr);

class Command : public Token
{
public:
  Command(const std::string &name) : m_name(name) {}

  virtual std::string code() const override;

private:
  std::string m_name;
};

class Block : public Token
{
public:
  enum BraceType { BRACE, BRACKET };
  Block(const BraceType type) : m_type(type) {}

  virtual std::string code() const override;

private:
  BraceType m_type;
};

class Variable : public Token
{
public:
  Variable(const std::string &name, const TokenPtr value)
    : m_name(name), m_value(value) {}

  virtual std::string code() const override;

private:
  std::string m_name;
  TokenPtr m_value;
};

class Boolean : public Token
{
public:
  Boolean(const bool value) : m_value(value) {}

  virtual std::string code() const override;

private:
  bool m_value;
};

class String : public Token
{
public:
  String(const std::string value) : m_value(value) {}

  virtual std::string code() const override;

private:
  std::string m_value;
};

class Literal : public Token
{
public:
  Literal(const std::string value) : m_value(value) {}

  virtual std::string code() const override;

private:
  std::string m_value;
};

#endif
