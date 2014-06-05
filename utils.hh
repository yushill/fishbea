#ifndef __UTILS_HH__
#define __UTILS_HH__

#include <cstdarg>
#include <cstdio>

std::string
cfmt( char const* _fmt, ... )
{
  va_list ap;

  for (intptr_t capacity = 128, size; true; capacity = (size > -1) ? size + 1 : capacity * 2) {
    /* allocation */
    char storage[capacity];
    /* Try to print in the allocated space. */
    va_start( ap, _fmt );
    size = vsnprintf( storage, capacity, _fmt, ap );
    va_end( ap );
    /* If it worked, return */
    if (size >= 0 and size < capacity)
      return std::string( storage );
  }
  throw "should not be here!";
}

#endif /* __UTILS_HH__ */
