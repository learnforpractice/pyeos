#include <softfloat.hpp>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../double.hpp"

/*
 * Find types and Operations in source/include/softfloat.h
 *
 * To compile:
 * gcc example.c -I ../source/include/ ../build/Linux-386-GCC/softfloat.a -o example
 *
 */

int main() {

   Double d(1.1);
   Double d2(2.2);
   Double d3;
   d3 = d+d2;
   printf("d+d2: %f\n", *(double*)&d3._v.d);

   d += 3.5;
   printf("d += 3.5: %f\n", *(double*)&d._v.d);
   d -= 3.5;
   printf("d -= 3.5: %f\n", *(double*)&d._v.d);

   d *= 4;
   printf("d *= 4: %f\n", *(double*)&d._v.d);

   printf("int64_t(d): %d\n", int64_t(d));
   d += 4;
   printf("d += 4: %f\n", *(double*)&d._v.d);
   d /= 4;
   printf("d /= 4: %f\n", *(double*)&d._v.d);

   return 0;
}
