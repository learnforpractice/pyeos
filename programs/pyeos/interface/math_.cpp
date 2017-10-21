#include "math_.hpp"

void mul64_(void* n1, void* n2, void* out) {
   ((uint64_t*)out)[0] = ((uint64_t*)n1)[0] * ((uint64_t*)n2)[0];
}

void div64_(void* n1, void* n2, void* out) {
   ((uint64_t*)out)[0] = ((uint64_t*)n1)[0] / ((uint64_t*)n2)[0];
}

void add64_(void* n1, void* n2, void* out) {
   ((uint64_t*)out)[0] = ((uint64_t*)n1)[0] + ((uint64_t*)n2)[0];
}

void sub64_(void* n1, void* n2, void* out) {
   ((uint64_t*)out)[0] = ((uint64_t*)n1)[0] - ((uint64_t*)n2)[0];
}

int gt64_(void* n1, void* n2) {
   return ((uint128_t*)n1)[0] > ((uint128_t*)n2)[0];
}

int ge64_(void* n1, void* n2) {
   return ((uint64_t*)n1)[0] >= ((uint64_t*)n2)[0];
}

int eq64_(void* n1, void* n2) {
   return ((uint64_t*)n1)[0] == ((uint64_t*)n2)[0];
}

int lt64_(void* n1, void* n2) {
   return ((uint64_t*)n1)[0] < ((uint64_t*)n2)[0];
}

int le64_(void* n1, void* n2) {
   return ((uint64_t*)n1)[0] <= ((uint64_t*)n2)[0];
}

void mul128_(void* n1, void* n2, void* out) {
   ((uint128_t*)out)[0] = ((uint128_t*)n1)[0] * ((uint128_t*)n2)[0];
}

void div128_(void* n1, void* n2, void* out) {
   ((uint128_t*)out)[0] = ((uint128_t*)n1)[0] / ((uint128_t*)n2)[0];
}

void add128_(void* n1, void* n2, void* out) {
   ((uint128_t*)out)[0] = ((uint128_t*)n1)[0] + ((uint128_t*)n2)[0];
}

void sub128_(void* n1, void* n2, void* out) {
   ((uint128_t*)out)[0] = ((uint128_t*)n1)[0] - ((uint128_t*)n2)[0];
}

int gt128_(void* n1, void* n2) {
   return ((uint128_t*)n1)[0] > ((uint128_t*)n2)[0];
}

int ge128_(void* n1, void* n2) {
   return ((uint128_t*)n1)[0] >= ((uint128_t*)n2)[0];
}

int eq128_(void* n1, void* n2) {
   return ((uint128_t*)n1)[0] == ((uint128_t*)n2)[0];
}

int lt128_(void* n1, void* n2) {
   return ((uint128_t*)n1)[0] < ((uint128_t*)n2)[0];
}

int le128_(void* n1, void* n2) {
   return ((uint128_t*)n1)[0] <= ((uint128_t*)n2)[0];
}
