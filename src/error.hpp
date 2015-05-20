#ifndef ERROR_HPP
#define ERROR_HPP

#include <exception>
#include <boost/format.hpp>

class Error : public std::exception
{
public:
  Error(const boost::format &format) : m_what(format.str()) {}
  Error(const std::string &message) : m_what(message) {}

  virtual const char *what() const noexcept override { return m_what.c_str(); }

private:
  std::string m_what;
};

#endif
