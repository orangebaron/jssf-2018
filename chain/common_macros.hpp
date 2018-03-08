#ifndef COMMON_MACROS_HPP
#define COMMON_MACROS_HPP

#define validCheckBegin(); \
  try { if (v.at(this)) return true; else return false; } catch (...) {} \
  v[this] = false;
#define validCheckEnd(); \
  v[this] = true; \
  return true;
#define getWorkBegin(); \
  try { return w.at(this); } catch (...) {} \
  WorkType& work = w.emplace(this,0).first->second;

#endif
