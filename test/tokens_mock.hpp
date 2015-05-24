#ifndef TOKENS_MOCK_H
#define TOKENS_MOCK_H

class EmptyToken : public Token
{
public:
  bool empty() const { return true; }
  std::string code() const { return ""; }
};

#endif
