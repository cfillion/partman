#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <memory>
#include <ostream>
#include <string>
#include <vector>

class Token;

template <class T = Token>
using TokenPtr = std::shared_ptr<T>;

class Token
{
  friend Token &operator<<(Token &, TokenPtr<>);

public:
  virtual ~Token(){}
  virtual std::string code() const;
  virtual bool empty() const { return false; }

protected:
  const std::vector<TokenPtr<> > &children() const { return m_children; }

private:
  std::vector<TokenPtr<> > m_children;
};

std::ostream &operator<<(std::ostream &, const TokenPtr<>);
Token &operator<<(Token &, TokenPtr<>);

class Command : public Token
{
public:
  Command(const std::string &name) : m_name(name) {}

  virtual std::string code() const override;
  virtual bool empty() const override { return m_name.empty(); }

  const std::string &get() const { return m_name; }
  void set(const std::string &val) { m_name = val; }

  Command &operator=(const std::string &val) { set(val); return *this; }

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
  Variable(const std::string &name, const TokenPtr<> value)
    : m_name(name), m_value(value) {}

  virtual std::string code() const override;
  virtual bool empty() const override { return m_value->empty(); }

  const TokenPtr<> &get() const { return m_value; }
  void set(const TokenPtr<> val) { m_value = val; }

  Variable &operator=(const TokenPtr<> &val) { set(val); return *this; }

private:
  std::string m_name;
  TokenPtr<> m_value;
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
  String(const std::string value = "") : m_value(value) {}

  virtual std::string code() const override;
  virtual bool empty() const override { return m_value.empty(); }

  const std::string &get() const { return m_value; }
  void set(const std::string &val) { m_value = val; }

  String &operator=(const std::string &val) { set(val); return *this; }

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

class Function : public Token
{
public:
  Function(const std::string name) : m_name(name) {}
  virtual std::string code() const override;

private:
  std::string m_name;
};

class Comment : public Token
{
public:
  Comment(const std::string text, const bool decorate = false)
    : m_text(text), m_decorate(decorate) {}

  virtual std::string code() const override;

private:
  std::string m_text;
  bool m_decorate;
};

#endif
