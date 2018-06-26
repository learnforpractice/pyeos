#include <stdint.h>

#include "../double.hpp"

Double fabs(Double x)
{
   union {double f; uint64_t i;} u;
   u.i = x._v.v;
   u.i &= -1ULL/2;
   return u.f;
}
