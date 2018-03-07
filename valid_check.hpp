#ifndef VALID_CHECK_HPP
#define VALID_CHECK_HPP

#define validCheckBegin(); \
  try { if (v.at(this)) return true; else return false; } catch (...) {} \
  v[this] = false;
#define validCheckEnd(); \
  v[this] = true; \
  return true;

#endif
