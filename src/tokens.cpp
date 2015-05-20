#include "tokens.hpp"

std::ostream &operator<<(std::ostream &stream, const Token &token)
{
  stream << token.code();
  return stream;
}
