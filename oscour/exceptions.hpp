#pragma once
#include <stdexcept>

namespace oscour
{
struct malformed_packet : public std::runtime_error
{
  malformed_packet(const char* w = "malformed packet") : std::runtime_error(w)
  {
  }
};

struct malformed_message : public std::runtime_error
{
  malformed_message(const char* w = "malformed message")
      : std::runtime_error(w)
  {
  }
};

struct malformed_bundle : public std::runtime_error
{
  malformed_bundle(const char* w = "malformed bundle") : std::runtime_error(w)
  {
  }
};

struct wrong_argument_type : public std::runtime_error
{
  wrong_argument_type(const char* w = "wrong argument type")
      : std::runtime_error(w)
  {
  }
};

struct missing_argument : public std::runtime_error
{
  missing_argument(const char* w = "missing argument") : std::runtime_error(w)
  {
  }
};

struct excess_argument : public std::runtime_error
{
  excess_argument(const char* w = "too many arguments") : std::runtime_error(w)
  {
  }
};

struct out_of_memory : public std::runtime_error
{
  out_of_memory(const char* w = "out of buffer memory") : std::runtime_error(w)
  {
  }
};

struct bundle_not_in_progress : public std::runtime_error
{
  bundle_not_in_progress(
      const char* w = "call to EndBundle when bundle is not in progress")
      : std::runtime_error(w)
  {
  }
};

struct message_in_progress : public std::runtime_error
{
  message_in_progress(
      const char* w
      = "opening or closing bundle or message while message is in progress")
      : std::runtime_error(w)
  {
  }
};

struct message_not_in_progress : public std::runtime_error
{
  message_not_in_progress(
      const char* w = "call to EndMessage when message is not in progress")
      : std::runtime_error(w)
  {
  }
};
}
