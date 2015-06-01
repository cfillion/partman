#ifndef TOKENS_MOCK_H
#define TOKENS_MOCK_H

class EmptyToken : public Token
{
public:
  bool empty() const { return true; }
  std::string code() const { return ""; }
};

class TestToken : public Token
{
public:
  std::string code() const { return "test"; }
};

#endif
