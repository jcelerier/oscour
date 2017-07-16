#pragma once
#include <oscour/types.hpp>
namespace oscour
{

std::ostream& operator<<(std::ostream& s, nil)
{
  s << "nil";
  return s;
}
std::ostream& operator<<(std::ostream& s, infinitum)
{
  s << "infinitum";
  return s;
}
std::ostream& operator<<(std::ostream& s, string v)
{
  s << v.value;
  return s;
}
std::ostream& operator<<(std::ostream& s, symbol v)
{
  s << v.value;
  return s;
}
std::ostream& operator<<(std::ostream& s, rgba v)
{
  s << v.value;
  return s;
}
std::ostream& operator<<(std::ostream& s, midi v)
{
  s << v.value;
  return s;
}
std::ostream& operator<<(std::ostream& s, time_tag v)
{
  s << v.value;
  return s;
}
std::ostream& operator<<(std::ostream& s, blob v)
{
  s << "blob";
  return s;
}
std::ostream& operator<<(std::ostream& s, begin_array v)
{
  s << "[";
  return s;
}
std::ostream& operator<<(std::ostream& s, end_array v)
{
  s << "]";
  return s;
}
}
