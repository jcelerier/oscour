#pragma once
#include <stdexcept>
namespace oscour
{
class malformed_packet : public std::runtime_error
{
public:
  malformed_packet(const char* w = "malformed packet") : std::runtime_error(w)
  {
  }
};

class malformed_message : public std::runtime_error
{
public:
  malformed_message(const char* w = "malformed message")
      : std::runtime_error(w)
  {
  }
};

class malformed_bundle : public std::runtime_error
{
public:
  malformed_bundle(const char* w = "malformed bundle") : std::runtime_error(w)
  {
  }
};

class wrong_argument_type : public std::runtime_error
{
public:
  wrong_argument_type(const char* w = "wrong argument type")
      : std::runtime_error(w)
  {
  }
};

class missing_argument : public std::runtime_error
{
public:
  missing_argument(const char* w = "missing argument") : std::runtime_error(w)
  {
  }
};

class excess_argument : public std::runtime_error
{
public:
  excess_argument(const char* w = "too many arguments") : std::runtime_error(w)
  {
  }
};
}
